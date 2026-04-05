#include "CS3113/Entity.h"

// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr char    BG_COLOUR[] = "#000000";
constexpr Vector2 ORIGIN      = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

constexpr int   NUMBER_OF_TILES         = 100,
                NUMBER_OF_POINTS        = 101;

constexpr float TILE_DIMENSION          = 10.0f,
                // in m/ms², since delta time is in ms
                ACCELERATION_OF_GRAVITY = 0.1625f,
                FIXED_TIMESTEP          = 1.0f / 60.0f,
                END_GAME_THRESHOLD      = 800.0f;

constexpr float MAX_FUEL = 100.0f;
constexpr float FUEL_BURN_RATE = 0.5f;

constexpr float MAX_SAFE_VERTICAL_SPEED = 2.0f;
constexpr float MAX_SAFE_HORIZONTAL_SPEED = 1.5f;

float gFuel = MAX_FUEL; //lol might add some gamersupps while at it

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

float gTerrainPoints[NUMBER_OF_POINTS];

constexpr float HORIZONTAL_THRUST = 0.25f;
constexpr float VERTICAL_THRUST = 0.35f;

Entity *gLander = nullptr;
Entity *gTiles   = nullptr;
Entity* gAsteroid = nullptr;

float gAsteroidVelocityX = 0.0f;
float gAsteroidVelocityY = 0.0f;

bool gLandingTile[NUMBER_OF_TILES];

bool PLAYING = true;
bool LAND = false;
bool CRASH = false;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Physics");

    /*
        ----------- PROTAGONIST -----------
    */
    std::map<Direction, std::vector<int>> landerAnimation;
    landerAnimation[RIGHT] = { 0,1,2,3,4,5,6,7 };

    gLander = new Entity(
        {ORIGIN.x, 50.0f}, // position
        {10.0f, 10.0f},  // scale
        "assets/paddle.png",        // texture file address (I couldnt get the appollo to animate corrctly_)
        ATLAS,
        {1,8},
        landerAnimation,
        PLAYER                         // entity type
    );

    gLander->setColliderDimensions({
        gLander->getScale().x / 3.0f,
        gLander->getScale().y / 3.0f
    });
    gLander->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    /*
        ----------- TERRAIN -----------
    */

    float baseHeight = ORIGIN.y + 150.0f;
    gTerrainPoints[0] = baseHeight;
    gTerrainPoints[1] = baseHeight;
    gTerrainPoints[2] = baseHeight;
    for (int i = 3; i < NUMBER_OF_POINTS; i++) {
        float variation = GetRandomValue(-20,20);
        gTerrainPoints[i] = gTerrainPoints[i-1] + variation;
        if (gTerrainPoints[i] < 250) gTerrainPoints[i] = 250;
        if (gTerrainPoints[i] > 550) gTerrainPoints[i] = 550;
    }

    int pad3 = GetRandomValue(10,30);
    int pad2 = GetRandomValue(40, 60);
    int pad1 = GetRandomValue(70, 90);

    float h = gTerrainPoints[pad3];
    gTerrainPoints[pad3 + 1] = h;
    gTerrainPoints[pad3 + 2] = h;
    gTerrainPoints[pad3 + 3] = h;

    h = gTerrainPoints[pad2];
    gTerrainPoints[pad2 + 1] = h;
    gTerrainPoints[pad2 + 2] = h;

    h = gTerrainPoints[pad1];
    gTerrainPoints[pad1 + 1] = h;

    /*
        ----------- TILES -----------
    */
    gTiles = new Entity[NUMBER_OF_TILES];

    // Compute the left‑most x coordinate so that the entire row is centred
    float leftMostX = ORIGIN.x - (NUMBER_OF_TILES * TILE_DIMENSION) / 2.0f;

    for (int i = 0; i < NUMBER_OF_TILES; i++) 
    {
        float heightA = gTerrainPoints[i];
        float heightB = gTerrainPoints[i+1];

        gLandingTile[i] = (heightA == heightB);

        float tileHeight = std::max(heightA,heightB);

        gTiles[i].setTexture("assets/empty.png");
        if (gLandingTile[i]) gTiles[i].setEntityType(WIN);
        else gTiles[i].setEntityType(PLATFORM);
        gTiles[i].setScale({TILE_DIMENSION,200.0f}); //make taller
        gTiles[i].setColliderDimensions({TILE_DIMENSION,200.0f});
        gTiles[i].setPosition({
            leftMostX + i * TILE_DIMENSION,
            tileHeight + 100.0f
        });
    }

    //asteroid

    int asteroidSize = GetRandomValue(3,5);
    float asteroidPixelSize = asteroidSize * TILE_DIMENSION;
    gAsteroid = new Entity(
        { GetRandomValue(200,800), GetRandomValue(50,200)}, //try and land on it
        { asteroidPixelSize, asteroidPixelSize },
        "assets/paddle.png",
        PLATFORM
    );

    gAsteroidVelocityX = GetRandomValue(-10, 10) / 10000.0f;
    gAsteroidVelocityY = GetRandomValue(-10, 10) / 10000.0f; //fix to be slower
}

void processInput() 
{   bool thrust = false;
    float ax = 0.0f;
    float ay = ACCELERATION_OF_GRAVITY;
    if (gFuel > 0){
        if (IsKeyDown(KEY_A)){
            ax -= HORIZONTAL_THRUST;
            thrust = true;
        }
        if (IsKeyDown(KEY_D)){
            ax += HORIZONTAL_THRUST;
            thrust = true;
        }
        if (IsKeyDown(KEY_W)){
            ay -= VERTICAL_THRUST;
            thrust = true;
        }
    }
    gLander->setThrusting(thrust);
    gLander->setAcceleration({ax,ay});
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;

    //reuse from pong
    if (IsKeyPressed(KEY_R))
    {
        if (!PLAYING)
        {
            PLAYING = true;
            LAND = false;
            CRASH = false;

            gFuel = MAX_FUEL;

            delete gLander;
            delete[] gTiles;

            initialise(); //i should have done this last time
        }
    }
}

void update() 
{
    if (!PLAYING) return;
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
        //gLander->update(FIXED_TIMESTEP, gTiles, NUMBER_OF_TILES, gBlocks, NUMBER_OF_BLOCKS);
        gLander->update(FIXED_TIMESTEP, gTiles, NUMBER_OF_TILES, gAsteroid, 1);

        for (int i = 0; i < NUMBER_OF_TILES; i++) 
        {
            gTiles[i].update(FIXED_TIMESTEP, nullptr, 0);
        }

        if (gFuel > 0 && (IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_D))) {
            gFuel -= FUEL_BURN_RATE * FIXED_TIMESTEP;
            if (gFuel < 0) gFuel = 0;
        }

        if (!gLander->isThrusting()) gLander->setFrame(0);
        //overwrite if not thrusting
        deltaTime -= FIXED_TIMESTEP;

        gAsteroid->update(FIXED_TIMESTEP, gTiles, NUMBER_OF_TILES);
    }

    Vector2 pos = gLander->getPosition();
    if (pos.x < 0) pos.x = 0;
    if (pos.x > SCREEN_WIDTH) pos.x = SCREEN_WIDTH;
    gLander->setPosition(pos);

    /*
    Vector2 pos2 = gAsteroid->getPosition(); //excuse my poor naming
    gAsteroidVelocityX += GetRandomValue(-1, 1) * 0.00005f;
    gAsteroidVelocityY += GetRandomValue(-1, 1) * 0.00005f; //slow it way down
    float maxDrift = 0.02f;

    if (gAsteroidVelocityX > maxDrift) gAsteroidVelocityX = maxDrift;
    if (gAsteroidVelocityX < -maxDrift) gAsteroidVelocityX = -maxDrift;

    if (gAsteroidVelocityY > maxDrift) gAsteroidVelocityY = maxDrift;
    if (gAsteroidVelocityY < -maxDrift) gAsteroidVelocityY = -maxDrift;

    pos2.x += gAsteroidVelocityX;
    pos2.y += gAsteroidVelocityY;
    */
    //alternate drifting
    Vector2 vel = gAsteroid->getVelocity();

    vel.x += GetRandomValue(-1, 1) * 0.01f;
    vel.y += GetRandomValue(-1, 1) * 0.01f;

    float maxSpeed = 0.5f;

    if (vel.x > maxSpeed) vel.x = maxSpeed;
    if (vel.x < -maxSpeed) vel.x = -maxSpeed;
    if (vel.y > maxSpeed) vel.y = maxSpeed;
    if (vel.y < -maxSpeed) vel.y = -maxSpeed;

    gAsteroid->setVelocity(vel);



    //if (pos2.x < 0 || pos2.x > SCREEN_WIDTH) gAsteroidVelocityX *= -1;
    //if (pos2.y < 0 || pos2.y > SCREEN_HEIGHT) gAsteroidVelocityY *= -1;
    /*
    if (pos2.x < -50) pos2.x = SCREEN_WIDTH + 50;
    if (pos2.x > SCREEN_WIDTH + 50) pos2.x = -50;

    if (pos2.y < -50) pos2.y = SCREEN_HEIGHT + 50;
    if (pos2.y > SCREEN_HEIGHT + 50) pos2.y = -50;

    gAsteroid->setPosition(pos2);
    */ 
    //get crash
    if (gLander->getLandingState() == CRASHED) {
        PLAYING = false;
        CRASH = true;
    }
    if (gLander->getLandingState() == LANDED) {
        PLAYING = false;
        LAND = true;
    }

    if (gLander->getPosition().y > END_GAME_THRESHOLD) 
        gAppStatus = TERMINATED;
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    gLander->render();

    gAsteroid->render();

    float leftMostX = ORIGIN.x - (NUMBER_OF_TILES * TILE_DIMENSION) / 2.0f;
    if (PLAYING){
        for (int i = 0; i < NUMBER_OF_TILES; i++) {
            float x1 = leftMostX + i * TILE_DIMENSION;
            float x2 = leftMostX + (i+1) * TILE_DIMENSION;

            float y1 = gTerrainPoints[i];
            float y2 = gTerrainPoints[i+1];
            bool flashOn = ((int)(GetTime() * 4) % 2) == 0; //flash to see easier
            Color lineColor; //mark landing zones
            if (gLandingTile[i]) {
                lineColor = flashOn ? GREEN : DARKGREEN;
            }
            else {
                lineColor = WHITE;
            }
            DrawLine(x1, y1, x2, y2, lineColor);
        }

        for (int i = 0; i < NUMBER_OF_TILES;  i++) gTiles[i].render();

        //fuel bar so it looks nicer
        float fuelPercent = gFuel / MAX_FUEL;
        DrawRectangle(20,50,200,20,DARKGRAY);
        DrawRectangle(20,50,200 * fuelPercent, 20, GREEN);
        DrawRectangleLines(20,50,200,20,WHITE);

        //get speed for display:
        Vector2 vel = gLander->getVelocity();

        float vSpeed = fabs(vel.y);
        float hSpeed = fabs(vel.x);

        Color VColor = (vSpeed > MAX_SAFE_VERTICAL_SPEED) ? RED : GREEN;
        Color hColor = (hSpeed > MAX_SAFE_HORIZONTAL_SPEED) ? RED : GREEN;

        DrawText(TextFormat("Fuel: %d", (int)gFuel), 20, 20, 24, WHITE);
        //use color to show dangeorus warning (todo: horizontal collision check)
        DrawText(TextFormat("V Speed: %.2f / %.2f", vSpeed, MAX_SAFE_VERTICAL_SPEED), 20, 80, 20, VColor);

        DrawText(TextFormat("H Speed: %.2f / %.2f", hSpeed, MAX_SAFE_HORIZONTAL_SPEED), 20, 105, 20, hColor);
    }
    //also reuse
    else {
        int titleSize = 60;
        const char* result;


        if (LAND) result = "SUCCESSFUL LANDING!";
        else result = "CRASHED";

        DrawText(result, SCREEN_WIDTH / 2 - MeasureText(result, titleSize) / 2, SCREEN_HEIGHT / 2 - 60, titleSize, WHITE);
        DrawText("Press R to Restart", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2 + 10, 30, WHITE);
        DrawText("Press Q to Quit", SCREEN_WIDTH / 2 - 110, SCREEN_HEIGHT / 2 + 50, 20, GRAY);

    }
    EndDrawing();
}

void shutdown() 
{ 
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