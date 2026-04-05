/**
* Author: Ryan Jin
* Assignment: Rise of the AI
* Date due: 2025-04-04, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/Entity.h"
#include "CS3113/Map.h"
#include <vector>
#include <time.h> //just in case

struct GameState
{
    Entity *xochitl;

    Entity* ghost;

    Map *map;

    Entity* button;

    Music bgm;
    Sound jumpSound;

    Camera2D camera;

    int hearts;
    float hitCooldown;

    int currentWidth;
    int currentHeight;

};

// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr char    BG_COLOUR[]      = "#011627";
constexpr Vector2 ORIGIN           = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 },
                  ATLAS_DIMENSIONS = { 6, 8 };

constexpr Vector2 GHOST_ATLAS_DIMENSIONS = { 7, 8 };

constexpr int   NUMBER_OF_TILES         = 20,
                NUMBER_OF_BLOCKS        = 3;
constexpr float TILE_DIMENSION          = 75.0f,
                // in m/ms², since delta time is in ms
                ACCELERATION_OF_GRAVITY = 981.0f,
                FIXED_TIMESTEP          = 1.0f / 60.0f,
                END_GAME_THRESHOLD      = 800.0f;

constexpr int LEVEL_WIDTH  = 30,
              LEVEL_HEIGHT = 13;

// i hate parkour
constexpr float DEBUG_FLY_SPEED = 12.0f;

constexpr int MAX_HEARTS = 3;
constexpr float HIT_COOLDOWN_TIME = 5.0f; //avoid multi hit kill

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;
/*
unsigned int gLevelData[] = {
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
    4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4,
    4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
};
*/

/*
unsigned int gLevel1Data[] = {
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    4,0,0,0,0,0,0,4,0,0,0,0,0,4,
    4,0,0,0,0,0,0,0,4,0,0,4,0,4,
    4,0,0,0,0,0,0,0,0,0,0,0,0,4,
    4,0,0,0,0,0,4,0,0,0,0,4,0,4,
    4,0,0,0,4,0,0,0,0,0,4,0,0,4,
    4,4,0,0,0,0,0,0,4,4,0,0,0,4,
    4,0,0,0,4,4,0,0,0,0,0,0,0,4,
    4,0,0,0,0,0,0,4,0,4,4,2,2,4,
    4,0,0,0,0,0,0,0,0,4,4,4,4,4,
    4,2,2,2,2,2,2,2,4,4,4,4,4,4,
    4,3,3,3,3,3,3,3,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4
};

unsigned int gLevel2Data[] = {
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    4,0,0,0,0,0,0,0,0,0,0,0,0,4,
    4,0,0,0,4,0,0,0,0,4,0,0,0,4,
    4,0,0,0,0,0,0,4,0,0,0,0,0,4,
    4,0,4,0,0,0,0,0,0,0,4,0,0,4,
    4,0,0,0,0,4,0,0,4,0,0,0,0,4,
    4,0,0,4,0,0,0,0,0,0,0,4,0,4,
    4,0,0,0,0,0,4,0,0,4,0,0,0,4,
    4,0,4,0,0,0,0,0,0,0,0,0,0,4,
    4,0,0,0,0,4,0,0,0,0,4,0,0,4,
    4,2,2,2,0,0,2,2,2,0,0,2,2,4,
    4,3,3,3,0,0,3,3,3,0,0,3,3,4,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4
};

unsigned int gLevel3Data[] = {
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    4,0,0,0,0,0,0,0,0,0,0,0,0,4,
    4,0,4,4,0,0,0,0,0,4,4,0,0,4,
    4,0,0,0,0,4,0,0,4,0,0,0,0,4,
    4,0,0,0,0,0,0,0,0,0,0,4,0,4,
    4,0,0,4,0,0,4,0,0,4,0,0,0,4,
    4,0,0,0,0,0,0,0,0,0,0,0,0,4,
    4,4,0,0,4,0,0,0,0,4,0,0,4,4,
    4,0,0,0,0,0,0,4,0,0,0,0,0,4,
    4,0,0,4,0,0,0,0,0,0,4,0,0,4,
    4,2,2,2,2,0,0,2,2,2,2,0,0,4,
    4,3,3,3,3,0,0,3,3,3,3,0,0,4,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4
};
*/

unsigned int gLevel1Data[LEVEL_WIDTH * LEVEL_HEIGHT];
unsigned int gLevel2Data[LEVEL_WIDTH * LEVEL_HEIGHT];
unsigned int gLevel3Data[LEVEL_WIDTH * LEVEL_HEIGHT];

int gWidth1, gWidth2, gWidth3;

int gCurrentLevel = 1;

GameState gState;

bool PLAYING = false;
bool WIN = false;
bool LOSE = false;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void fillLevelData(unsigned int* data, int& width, int height);

void loadLevel(int levelNumber);

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Maps");
    InitAudioDevice();

    gState.bgm = LoadMusicStream("assets/game/05 - NOT Silent Forest.wav");
    SetMusicVolume(gState.bgm, 0.33f);
    PlayMusicStream(gState.bgm);

    gState.jumpSound = LoadSound("assets/game/Dirt Jump.wav");

    /*
        ----------- MAP -----------
    */


    gState.map = nullptr;

    /*
    gState.map = new Map(
        LEVEL_WIDTH, LEVEL_HEIGHT,   // map grid cols & rows
        (unsigned int *) gLevelData, // grid data
        "assets/game/tileset.png",   // texture filepath
        TILE_DIMENSION,              // tile size
        4, 1,                        // texture cols & rows
        ORIGIN                       // in-game origin
    );
    */

    /*
        ----------- PROTAGONIST -----------
    */
    std::map<Direction, std::vector<int>> xochitlAnimationAtlas = {
        {DOWN,  {  0,  1,  2,  3,  4,  5,  6,  7 }},
        {LEFT,  {  8,  9, 10, 11, 12, 13, 14, 15 }},
        {UP,    { 24, 25, 26, 27, 28, 29, 30, 31 }},
        {RIGHT, { 40, 41, 42, 43, 44, 45, 46, 47 }},
    };

    float sizeRatio  = 48.0f / 64.0f;

    // Assets from @see https://sscary.itch.io/the-adventurer-female
    gState.xochitl = new Entity(
        {ORIGIN.x - 300.0f, ORIGIN.y - 200.0f}, // position
        {200.0f * sizeRatio, 200.0f},           // scale
        "assets/game/walk.png",                 // texture file address
        ATLAS,                                  // single image or atlas?
        ATLAS_DIMENSIONS,                       // atlas dimensions
        xochitlAnimationAtlas,                  // actual atlas
        PLAYER                                  // entity type
    );

    gState.button = new Entity(
        { 87.5f, 220.0f }, // position
        { 75.0f, 75.0f },           // scale
        "assets/game/tile_0061.png",                 // texture file address
        BLOCK                                 // entity type
    );

    gState.xochitl->setJumpingPower(550.0f);
    gState.xochitl->setColliderDimensions({
        gState.xochitl->getScale().x / 3.5f,
        gState.xochitl->getScale().y / 3.0f
    });
    gState.xochitl->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    /*
       ----------- GHOST -----------
   */

   // skip manual indice
    std::vector<int> fireballIndices;
    for (int i = 6; i < 42; i++) { // skip empty/not lit
        fireballIndices.push_back(i);
    }

    std::map<Direction, std::vector<int>> ghostAnimationAtlas = {
        { DOWN, fireballIndices },
        { LEFT, fireballIndices },
        { UP, fireballIndices },
        { RIGHT, fireballIndices }
    };

   /*
    std::map<Direction, std::vector<int>> ghostAnimationAtlas = {
        {DOWN,  {  0,  1,  2,  3,  4,  5,  6,  7 }},
        {LEFT,  {  8,  9, 10, 11, 12, 13, 14, 15 }},
        {UP,    { 24, 25, 26, 27, 28, 29, 30, 31 }},
        {RIGHT, { 40, 41, 42, 43, 44, 45, 46, 47 }},
    };
    */

    // @see dyru.itch.io/pixel-ghost-template
    gState.ghost = new Entity(
        { ORIGIN.x + 300.0f, ORIGIN.y - 200.0f }, // position
        { 100.0f, 100.0f },                        // scale
        "assets/game/gosth.png",                 // texture file address
        ATLAS,                                   // single image or atlas?
        GHOST_ATLAS_DIMENSIONS,
        ghostAnimationAtlas,                    // actual atlas
        NPC                                      // entity type
    );

    gState.ghost->setAIState(IDLE); //switch when changing enemies across levels
    gState.ghost->setColliderDimensions({
        gState.ghost->getScale().x / 2.0f,
        gState.ghost->getScale().y
        });
    gState.ghost->setDirection(LEFT);
    gState.ghost->setSpeed(50.0f);

    /*
        ----------- CAMERA -----------
    */
    gState.camera = { 0 };                                // zero initialize
    gState.camera.target = gState.xochitl->getPosition(); // camera follows player
    gState.camera.offset = ORIGIN;                        // camera offset to center of screen
    gState.camera.rotation = 0.0f;                        // no rotation
    gState.camera.zoom = 1.0f;                            // default zoom

    gState.hearts = MAX_HEARTS;
    gState.hitCooldown = 0.0f;

    //generate the levels
    fillLevelData(gLevel1Data, gWidth1, LEVEL_HEIGHT);
    fillLevelData(gLevel2Data, gWidth2, LEVEL_HEIGHT);
    fillLevelData(gLevel3Data, gWidth3, LEVEL_HEIGHT);

    loadLevel(1);

    SetTargetFPS(FPS);
}

// control physics apply
bool isDebugFlying()
{
    return IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_LEFT) ||
        IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN);
}

void advanceLevelOrQuit() {
    // word
    if (gCurrentLevel < 3)
    {
        loadLevel(gCurrentLevel + 1);
        return;
    }
    else
    {
        //gAppStatus = TERMINATED;
        PLAYING = false;
        WIN = true;
        return;
    }
}

void loadLevel(int levelNumber) {
    gCurrentLevel = levelNumber;

    if (gState.map != nullptr) {
        delete gState.map; //clean up
        gState.map = nullptr;
    }

    unsigned int* selectedLevelData = nullptr;
    int selectedWidth = 0;

    //trying out switch case
    switch (levelNumber) {
        case 1:
            selectedLevelData = gLevel1Data;
            selectedWidth = gWidth1;
            gState.ghost->setPosition({ ORIGIN.x + 250.0f, ORIGIN.y - 200.0f });
            gState.ghost->setAIType(WANDERER);
            gState.ghost->setAIState(WALKING);
            break;
        case 2:
            selectedLevelData = gLevel2Data;
            selectedWidth = gWidth2;
            gState.ghost->setPosition({ ORIGIN.x + 250.0f, ORIGIN.y - 200.0f });
            gState.ghost->setAIType(FOLLOWER);
            gState.ghost->setAIState(IDLE);
            break;
        case 3:
            selectedLevelData = gLevel3Data;
            selectedWidth = gWidth3;
            gState.ghost->setPosition({ ORIGIN.x + 250.0f, ORIGIN.y - 200.0f });
            gState.ghost->setAIType(LERPER);
            gState.ghost->setAIState(IDLE);
            gState.ghost->setLerpFactor(0.2f);
            break;
        default:
            selectedLevelData = gLevel1Data;
            gCurrentLevel = 1;
            selectedWidth = gWidth1;
            gState.ghost->setPosition({ ORIGIN.x + 250.0f, ORIGIN.y - 200.0f });
            gState.ghost->setAIType(WANDERER);
            gState.ghost->setAIState(WALKING);
            break;
    }

    gState.map = new Map(
        selectedWidth, LEVEL_HEIGHT,
        selectedLevelData,
        "assets/game/tileset.png",
        TILE_DIMENSION,
        4, 1,
        ORIGIN
    );

    gState.currentWidth = selectedWidth;

    float halfWidth = (selectedWidth * TILE_DIMENSION) / 2.0f;
    gState.xochitl->setPosition({ ORIGIN.x - halfWidth + TILE_DIMENSION, ORIGIN.y - 100.0f });
    gState.ghost->setPosition({ ORIGIN.x + halfWidth - TILE_DIMENSION, ORIGIN.y - 100.0f });

    gState.button->activate();
    gState.camera.target = gState.xochitl->getPosition();

}

//respawn if killed
void resetGameToLevel1()
{
    gState.hearts = MAX_HEARTS;
    gState.hitCooldown = 0.0f;
    loadLevel(1);
}

void fillLevelData(unsigned int* data, int& width, int height) {
    width = GetRandomValue(14, LEVEL_WIDTH);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * width + x;

            if (y == 0 || y == height - 1 || x == 0 || x == width - 1) data[index] = 4; // wall of sterlings
            else if (y == height - 3) data[index] = (GetRandomValue(0, 10) > 2) ? 3 : 0; //prof blocks
            else if (y == height - 2) data[index] = (GetRandomValue(0, 10) > 2) ? 2 : 0;  //Eric blocks
            else if (y < height - 3 && GetRandomValue(0, 100) < 15) data[index] = 4;
            else data[index] = 0;
        }
    }
}


void processInput() 
{
    // start menu and end menu
    if (!PLAYING && !WIN && !LOSE) {
        if (IsKeyPressed(KEY_ENTER)) {
            PLAYING = true;
            loadLevel(1);
        }
    }
    else if (WIN || LOSE) {
        if (IsKeyPressed(KEY_ENTER)) {
            WIN = false;
            LOSE = false;
            PLAYING = false;
            gState.hearts = MAX_HEARTS; // Reset lives for next run
        }
    }

    else{

        gState.xochitl->resetMovement();

        if      (IsKeyDown(KEY_A)) gState.xochitl->moveLeft();
        else if (IsKeyDown(KEY_D)) gState.xochitl->moveRight();

        if (IsKeyPressed(KEY_W) && gState.xochitl->isCollidingBottom())
        {
            gState.xochitl->jump();
            PlaySound(gState.jumpSound);
        }

        if (GetLength(gState.xochitl->getMovement()) > 1.0f) 
            gState.xochitl->normaliseMovement();

        //if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;

        if (IsKeyPressed(KEY_ONE))   loadLevel(1);
        if (IsKeyPressed(KEY_TWO))  loadLevel(2);
        if (IsKeyPressed(KEY_THREE))   loadLevel(3);

        // fly to test
        Vector2 debugPosition = gState.xochitl->getPosition(); // help debug test fly

        if (IsKeyDown(KEY_RIGHT)) debugPosition.x += DEBUG_FLY_SPEED;
        if (IsKeyDown(KEY_LEFT))  debugPosition.x -= DEBUG_FLY_SPEED;
        if (IsKeyDown(KEY_UP))    debugPosition.y -= DEBUG_FLY_SPEED;
        if (IsKeyDown(KEY_DOWN))  debugPosition.y += DEBUG_FLY_SPEED;

        gState.xochitl->setPosition(debugPosition);
    }

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() 
{
    if (!PLAYING) return; //in menu
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
        UpdateMusicStream(gState.bgm);


        if (gState.hitCooldown > 0) {
            gState.hitCooldown -= FIXED_TIMESTEP;
        }

        // only physic if not debug
        if (!isDebugFlying()){
            gState.xochitl->update(
                FIXED_TIMESTEP, // delta time / fixed timestep
                nullptr,        // player
                gState.map,     // map
                gState.button,
                1,               // col. entity count
                nullptr,
                0
            );
        }

        gState.ghost->update(
            FIXED_TIMESTEP,
            gState.xochitl,   // target player
            gState.map,       // collide with map
            nullptr,          // no collidable entity array for now
            0,
            nullptr,          // no extra blocks
            0
        );

        if (gState.xochitl -> isColliding(gState.ghost)) {
            //not enough coffee, I'll just handle it here
            if (gState.hitCooldown <= 0) {
                gState.hearts -= 1;
                gState.hitCooldown = HIT_COOLDOWN_TIME;
            }
        }

        if (gState.hearts <= 0) {
            PLAYING = false;
            LOSE = true;
            return;
        }

        deltaTime -= FIXED_TIMESTEP;

        Vector2 currentPlayerPosition = gState.xochitl->getPosition();

        panCamera(&gState.camera, &currentPlayerPosition);

        // if we want to button end
        if (!gState.button->isActive()) {
            advanceLevelOrQuit();
            return;
        }

        if (gState.xochitl->getPosition().y > 800.0f){
            advanceLevelOrQuit(); 
            return;
        }
        if (gState.xochitl->getPosition().y < -220.0f){
            advanceLevelOrQuit(); 
            return;
        }
    }
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    if (PLAYING){
        BeginMode2D(gState.camera);

        gState.xochitl->render();
        gState.ghost->render();
        gState.button->render();
        gState.map->render();

        EndMode2D();

        // DEBUG
        DrawText(TextFormat("Level: %i", gCurrentLevel), 20, 20, 30, WHITE);
        DrawText("Press 1, 2, 3 to switch levels", 20, 55, 20, WHITE);

        DrawText(TextFormat("Hearts: %i", gState.hearts), 20, 90, 30, RED);
    }
    else {
        //yeeted from lunar lander
        int titleSize = 60;
        const char* result;


        if (WIN) result = "YOU WIN!";
        else if (LOSE) result = "YOU LOSE";
        else result = "RISE OF THE AI:";
        // "A BRAINROT PLATFORMER"

        DrawText(result, SCREEN_WIDTH / 2 - MeasureText(result, 60) / 2, SCREEN_HEIGHT / 2 - 50, 60, WHITE);
        DrawText("Press ENTER to Continue", SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 + 30, 20, GRAY);
    }
    EndDrawing();
}

void shutdown() 
{
    delete gState.xochitl;
    delete gState.ghost;
    delete gState.map;

    UnloadMusicStream(gState.bgm);
    UnloadSound(gState.jumpSound);

    CloseAudioDevice();
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