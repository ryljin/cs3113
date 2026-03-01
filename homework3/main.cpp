/**
* Author: Ryan Jin
* Assignment: Pong Clone
* Date due: 02/28/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/Entity.h"
#include <vector>
#include <ctime>

// I borrowed some logic off my DVD player game
// 
// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr float FIXED_TIMESTEP          = 1.0f / 60.0f;

float PADDLE_SPEED = 400.0f;
float BALL_SPEED = 350.0f;
constexpr float PADDLE_WIDTH = 20.0f;
constexpr float PADDLE_HEIGHT = 120.0f;
constexpr float BALL_SIZE = 20.0f;

constexpr float AI_ERROR = 75.0f; //when both are AI, higher is less accurate, lower is more
constexpr float AI_ERROR_DUO = 100.0f;

constexpr float MAX_BOUNCE_ANGLE = 75.0f;

constexpr Color BG_COLOR = BLACK;



// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

int gScore1 = 0;
int gScore2 = 0;
constexpr int WIN_SCORE = 3;

bool gAI1 = true; //start by default
bool gAI2 = false;
bool PLAYING = true;
// fix jitter by period
float gAITimer1 = 0.0f;
float gAITimer2 = 0.0f;
float gAIUpdateInterval = 0.01f;

//increase speeds

// Entities for pong
Entity* gPaddle1 = nullptr;
Entity* gPaddle2 = nullptr;
Entity* gBall = nullptr;

//this will keep track of points at a period
struct TrailPoint {
    Vector2 position;
    float timeCreated;
};

std::vector<TrailPoint> gTrail;
constexpr float TRAIL_DURATION = 30.0f;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Physics");
    SetTargetFPS(FPS);
    SetRandomSeed(time(NULL));

    gPaddle1 = new Entity(
        {
                50.0f, SCREEN_HEIGHT / 2.0f   // position
        },
        { PADDLE_WIDTH, PADDLE_HEIGHT },     // scale
        "assets/paddle.png",                  // texture file addresses
        PLAYER
    );
    gPaddle2 = new Entity(
        {
                SCREEN_WIDTH - 50.0f, SCREEN_HEIGHT / 2.0f   // position
        },
    { PADDLE_WIDTH, PADDLE_HEIGHT },     // scale
        "assets/paddle.png",                  // texture file addresses
        PLAYER
    );
    gBall = new Entity(
        {
                SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f   // position
        },
    { BALL_SIZE, BALL_SIZE },     // scale
        "assets/ball.png",                  // texture file addresses
        BALL
    );
    gBall -> setVelocity({BALL_SPEED, BALL_SPEED});
}

//serve ball after score
void serveBall() {
    Vector2 startPos = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    gBall->setPosition(startPos);
    float direction = 1.0f;

    if (gAI1 && !gAI2) {
        direction = 1.0f; //player should receive because reaction time
    }
    else if (!gAI1 && gAI2) {
        direction = -1.0f;
    }
    else {
        if (GetRandomValue(0, 1) == 0) direction = -1.0f;
        else direction = 1.0f;
    }

    float angle = GetRandomValue(-30, 30) * (PI / 180.0f);
    Vector2 velocity;
    velocity.x = direction * BALL_SPEED   * cos(angle);
    velocity.y = BALL_SPEED   * sin(angle);

    gBall->setVelocity(velocity);
}

void processInput() 
{
    // reset velocity for paddles
    gPaddle1->setVelocity({ 0, 0 });
    gPaddle2->setVelocity({ 0, 0 });

    if (IsKeyDown(KEY_W)) gPaddle1->setVelocity({0, -PADDLE_SPEED});
    else if (IsKeyDown(KEY_S)) gPaddle1->setVelocity({ 0, PADDLE_SPEED });
    if (!gAI1){
        if (IsKeyDown(KEY_UP)) gPaddle2->setVelocity({ 0, -PADDLE_SPEED });
        else if (IsKeyDown(KEY_DOWN)) gPaddle2->setVelocity({ 0, PADDLE_SPEED });
    }
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;

    if (IsKeyPressed(KEY_R)) {
        if (!PLAYING){
            gScore1 = 0;
            gScore2 = 0;
            PLAYING = true;
            serveBall();
        }
    }

    if (IsKeyPressed(KEY_T)) {
        gAI1 = !gAI1;
    }
    if (IsKeyPressed(KEY_Y)) {
        gAI2 = !gAI2;
    }

    if (IsKeyPressed(KEY_EQUAL)) { // why is it named like this
        BALL_SPEED += 25.0f;
        if (BALL_SPEED > 5000.0f)
            BALL_SPEED = 5000.0f;
        PADDLE_SPEED += 25.0f;
        if (PADDLE_SPEED > 5000.0f)
            PADDLE_SPEED = 5000.0f;
    }

    if (IsKeyPressed(KEY_MINUS)) {
        BALL_SPEED -= 25.0f;
        if (BALL_SPEED < 100.0f)
            BALL_SPEED = 100.0f;
        PADDLE_SPEED -= 25.0f;
        if (PADDLE_SPEED < 100.0f)
            PADDLE_SPEED = 100.0f;
    }
}

void update() 
{
    // Delta time
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    // Fixed timestep
    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        gAITimer1 += FIXED_TIMESTEP;
        gAITimer2 += FIXED_TIMESTEP;
        if (gAI1 && gAITimer1 >= gAIUpdateInterval) {
            gAITimer1 = 0.0f;
            Vector2 ballPos = gBall->getPosition();
            Vector2 paddlePos = gPaddle2->getPosition();
            float targetY = ballPos.y;
            if (gAI1 && gAI2) targetY += GetRandomValue(-AI_ERROR_DUO, AI_ERROR_DUO);
            else targetY += GetRandomValue(-AI_ERROR, AI_ERROR);
            float range = 10.0f;
            if (targetY > paddlePos.y + range) gPaddle2->setVelocity({0,PADDLE_SPEED});
            else if (targetY < paddlePos.y - range) gPaddle2->setVelocity({ 0,-PADDLE_SPEED });
            else gPaddle2->setVelocity({0,0});
        }
        if (gAI2 && gAITimer2 >= gAIUpdateInterval) {
            gAITimer2 = 0.0f;
            Vector2 ballPos = gBall->getPosition();
            Vector2 paddlePos = gPaddle1->getPosition();
            float targetY = ballPos.y;
            if (gAI1 && gAI2) targetY += GetRandomValue(-AI_ERROR_DUO, AI_ERROR_DUO);
            else targetY += GetRandomValue(-AI_ERROR, AI_ERROR);
            float range = 10.0f;
            if (targetY > paddlePos.y + range) gPaddle1->setVelocity({ 0,PADDLE_SPEED });
            else if (targetY < paddlePos.y - range) gPaddle1->setVelocity({ 0,-PADDLE_SPEED });
            else gPaddle1->setVelocity({ 0,0 });
        }
        gPaddle1->update(FIXED_TIMESTEP);
        gPaddle2->update(FIXED_TIMESTEP);
        gBall->update(FIXED_TIMESTEP);

        float currentTime = GetTime();
        gTrail.push_back({ gBall->getPosition(), currentTime});
        float now = GetTime();

        while (!gTrail.empty() && now - gTrail.front().timeCreated > TRAIL_DURATION) {
            gTrail.erase(gTrail.begin());
        }

        Vector2 p2Pos = gPaddle2->getPosition();
        if (p2Pos.y < PADDLE_HEIGHT / 2) {
            gPaddle2->setPosition({ p2Pos.x, PADDLE_HEIGHT / 2 });
        }
        if (p2Pos.y > SCREEN_HEIGHT - PADDLE_HEIGHT / 2) {
            gPaddle2->setPosition({ p2Pos.x, SCREEN_HEIGHT - PADDLE_HEIGHT / 2 });
        }

        Vector2 p1Pos = gPaddle1->getPosition();
        if (p1Pos.y < PADDLE_HEIGHT / 2) {
            gPaddle1->setPosition({ p1Pos.x, PADDLE_HEIGHT / 2 });
        }
        if (p1Pos.y > SCREEN_HEIGHT - PADDLE_HEIGHT / 2) {
            gPaddle1->setPosition({ p1Pos.x, SCREEN_HEIGHT - PADDLE_HEIGHT / 2 });
        }

        Vector2 ballPos = gBall->getPosition();
        Vector2 ballVel = gBall->getVelocity();

        /*
        if (ballPos.y <= BALL_SIZE / 2 || ballPos.y >= SCREEN_HEIGHT - BALL_SIZE / 2) {
            ballVel.y *= -1; //flip velocty/bounce
            gBall->setVelocity(ballVel);
        }
        */
        if (ballPos.y <= BALL_SIZE / 2) {
            ballPos.y = BALL_SIZE / 2;

            //save direction so don't bounce backwards
            float originalDir;
            if (ballVel.x >= 0) originalDir = 1.0f;
            else originalDir = -1.0f;

            ballVel.y = fabs(ballVel.y);

            ballVel.x += GetRandomValue(-80,80); //less deterministic
            float speed = sqrt(ballVel.x * ballVel.x + ballVel.y * ballVel.y);
            ballVel.x = (ballVel.x / speed) * BALL_SPEED;
            ballVel.y = (ballVel.y / speed) * BALL_SPEED  ;

            ballVel.x = fabs(ballVel.x) * originalDir;

            gBall->setPosition(ballPos);
            gBall->setVelocity(ballVel);
        }
        if (ballPos.y >= SCREEN_HEIGHT - BALL_SIZE / 2) {
            ballPos.y = SCREEN_HEIGHT - BALL_SIZE / 2;

            float originalDir;
            if (ballVel.x >= 0) originalDir = 1.0f;
            else originalDir = -1.0f;

            ballVel.y = -fabs(ballVel.y);

            ballVel.x += GetRandomValue(-80, 80); //less deterministic
            float speed = sqrt(ballVel.x * ballVel.x + ballVel.y * ballVel.y);
            ballVel.x = (ballVel.x / speed) * BALL_SPEED;
            ballVel.y = (ballVel.y / speed) * BALL_SPEED;

            ballVel.x = fabs(ballVel.x) * originalDir;

            gBall->setPosition(ballPos);
            gBall->setVelocity(ballVel);
        }

        if (gBall->isColliding(gPaddle1)) {
            Vector2 ballPos = gBall->getPosition();
            Vector2 paddlePos = gPaddle1->getPosition();
            // i should do this in entity...
            float intersectY = paddlePos.y - ballPos.y;
            float normalized = intersectY / (PADDLE_HEIGHT / 2.0f);
            if (normalized > 1.0f) normalized = 1.0f;
            if (normalized < -1.0f) normalized = -1.0f;
            float bounceAngle = normalized * MAX_BOUNCE_ANGLE * (PI / 180.0f);
            ballVel.x = BALL_SPEED   * cos(bounceAngle);
            ballVel.y = - BALL_SPEED   * sin(bounceAngle);
            gBall->setVelocity(ballVel);


            /*
            ballVel.x = fabs(ballVel.x); //flip velocty/bounce
            gBall->setVelocity(ballVel);
            */
        }

        if (gBall->isColliding(gPaddle2)) {
            Vector2 ballPos = gBall->getPosition();
            Vector2 paddlePos = gPaddle2->getPosition();
            float intersectY = paddlePos.y - ballPos.y;
            float normalized = intersectY / (PADDLE_HEIGHT / 2.0f);
            if (normalized > 1.0f) normalized = 1.0f;
            if (normalized < -1.0f) normalized = -1.0f;
            float bounceAngle = normalized * MAX_BOUNCE_ANGLE * (PI / 180.0f);
            ballVel.x = -BALL_SPEED   * cos(bounceAngle);
            ballVel.y = -BALL_SPEED   * sin(bounceAngle);
            gBall->setVelocity(ballVel);

            //ballVel.x = -fabs(ballVel.x); //flip velocty/bounce
            //gBall->setVelocity(ballVel);
        }

        /*
        if (ballPos.x < 0 || ballPos.x > SCREEN_WIDTH) {
            gAppStatus = TERMINATED; //QUIT WHEN SCORE
        }
        */
        if (ballPos.x < 0) {
            gScore2++;
            serveBall();
        }
        if (ballPos.x > SCREEN_WIDTH) {
            gScore1++;
            serveBall();
        }
        if (gScore1 >= WIN_SCORE || gScore2 >= WIN_SCORE) {
            PLAYING = false;
            //gAppStatus = TERMINATED;
        }

        deltaTime -= FIXED_TIMESTEP;
    }
    gTimeAccumulator = deltaTime;
}

void render()
{
    BeginDrawing();
    ClearBackground(BG_COLOR);
    if (PLAYING){
        gPaddle1->render();
        gPaddle2->render();

        float now = GetTime();
        for (size_t i = 1; i < gTrail.size(); i++) {
            float age = now - gTrail[i].timeCreated;
            float alphaR = 1.0f - (age / TRAIL_DURATION);
            if (alphaR<0.0f) alphaR = 0.0f;
            Color trailColor = {255, 255, 255, (unsigned char)(alphaR * 255)};

            DrawLineEx(
                gTrail[i-1].position,
                gTrail[i].position,
                3.0f,
                trailColor
            );
        }

        DrawText(TextFormat("%d", gScore1), SCREEN_WIDTH / 4, 30, 40, WHITE);
        DrawText(TextFormat("%d", gScore2), SCREEN_WIDTH * 3 / 4, 30, 40, WHITE);

        DrawText("Press Y to Toggle P1", 20, SCREEN_HEIGHT-60, 20, WHITE);
        Color aiColor1;
        if (gAI2) aiColor1 = GREEN;
        else aiColor1 = RED;
        DrawText(TextFormat("AI: %s", gAI2 ? "ON" : "OFF"), 20, SCREEN_HEIGHT-35, 20, aiColor1);

        //fix to fit better here
        const char* toggleText = "Press T to Toggle P2";
        DrawText(toggleText, SCREEN_WIDTH - MeasureText(toggleText,20) - 20, SCREEN_HEIGHT - 60, 20, WHITE);
        Color aiColor2;
        if (gAI1) aiColor2 = GREEN;
        else aiColor2 = RED;
        //fix to fit better here
        const char* aiText2 = TextFormat("AI: %s", gAI1 ? "ON" : "OFF");
        int aiWidth2 = MeasureText(aiText2, 20);
        DrawText(aiText2, SCREEN_WIDTH - aiWidth2 - 20, SCREEN_HEIGHT - 35, 20, aiColor2);


        gBall->render();
    }
    else {
        int titleSize = 60;
        const char* winner;

        if (gScore1 >= WIN_SCORE) winner = "PLAYER 1 WINS!";
        else winner = "PLAYER 2 WINS";

        DrawText(winner, SCREEN_WIDTH/2 - MeasureText(winner, titleSize)/2, SCREEN_HEIGHT/2-60,titleSize, WHITE);
        DrawText("Press R to Restart", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2 + 10, 30, WHITE);
        DrawText("Press Q to Quit", SCREEN_WIDTH / 2 - 110, SCREEN_HEIGHT / 2 + 50, 20, GRAY);
    }

    EndDrawing();
}

void shutdown() 
{ 
    delete gPaddle1;
    delete gPaddle2;
    delete gBall;
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}
