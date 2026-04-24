#include "CS3113/Entity.h"
#include "CS3113/Map.h"
#include "CS3113/ShaderProgram.h"
#include <vector>
#include <time.h> //just in case
#include <fstream>
#include <sstream>
#include <string>
#include <dirent.h>
#include <algorithm>
#include <map> //dict for global map
#include <cctype>

struct GameState
{
    Entity *xochitl;

    std::vector<Entity*> ghosts; //multiple zombies

    Map *map;

    Texture2D mapBackground;

    std::vector<Entity*> buttons; //ext

    std::vector<Entity*> lootButtons; // lot

    std::vector<Entity*> lanterns; //map lighting

    Music bgm;
    Sound jumpSound; //this isnt used.. for now
    Sound playerHitSound;
    Sound zombieHitSound;

    Camera2D camera;

    int hearts;
    float hitCooldown;

    int currentWidth;
    int currentHeight;

    // adding loot
    int day;
    int hour;
    int minute;

    // rate limit AI
    int lastAIGenerationDay;

    int healthStat;
    int foodStat;
    int energyStat;

    int distanceTravelled;

    // for US travel
    int worldGridX;
    int worldGridY;
    int localKmX;
    int localKmY;
    int travelDirection;
    int currentBiome;
    std::map<std::string, int> globalBiomeMap;
    std::vector<std::string> biomeTravelTexts[10];

    // coordinates and starting city (TODO: ADD A COORDINATE TO GRID SYSTEM)
    int startCity;
    float startLatitude;
    float startLongitude;
    float latitude;
    float longitude;

    std::string currentLocation;
    std::string travelLog;
    std::string travelDebugLog;

    std::string armorName;
    int armorProtection;
    int armorMobility;

    std::string flashlightName;
    int flashlightRadius;
    int flashlightRange;

    std::string firearmName;
    int firearmDamage;
    int firearmSpeed;

    std::string meleeName;
    int meleeDamage;
    int meleeSpeed;

    int ammo;
    //nt supplies;
    std::string actionMessage;
    float actionMessageTimer;

    float exitLockTimer;

    //compare supplies
    std::string pendingGearType;
    std::string pendingGearName;
    int pendingGearFirst;
    int pendingGearSecond;

    //cheats
    bool godMode;

    //toggle AI
    bool aiEnabled;

    Entity* currentEnemy;

    Vector2 combatHeartPosition;
    Vector2 combatHeartVelocity;

    Rectangle combatBox;
    Rectangle combatEnemyAttack;

    float combatTimer;
    float combatAttackTimer;
    float combatAttackDuration;

    int combatEnemyDamage;
    float combatEnemySpeed;

    float combatTelegraphDuration;
    bool combatAttackLive;

    Vector2 combatLerperPosition;
    Vector2 combatLerperStart;
    Vector2 combatLerperTarget;
    float combatLerperRadius;

    // these really ballooned up...

    int combatEnemyHealth;
    int combatEnemyMaxHealth;

    int combatAttackType;
    float combatAttackInterval;

    Vector2 combatBallPosition;
    Vector2 combatBallStart;
    Vector2 combatBallTarget;
    float combatBallRadius;

    Vector2 combatStoredPlayerPosition;
    Vector2 combatStoredEnemyPosition;

    // combat cooldowns
    float combatMeleeCooldown;
    float combatFirearmCooldown;

    // storing loot i wish c++ had pickle
    std::vector<std::string> armorLootNames;
    std::vector<int> armorLootProtection;
    std::vector<int> armorLootMobility;

    std::vector<std::string> meleeLootNames;
    std::vector<int> meleeLootDamage;
    std::vector<int> meleeLootSpeed;

    std::vector<std::string> firearmLootNames;
    std::vector<int> firearmLootDamage;
    std::vector<int> firearmLootSpeed;

    std::vector<std::string> flashlightLootNames;
    std::vector<int> flashlightLootRadius;
    std::vector<int> flashlightLootRange;
};

// map loading
struct LoadedMap
{
    std::vector<unsigned int> data;
    int width;
    int height;
};

enum TravelDirection
{
    TRAVEL_NORTH = 0,
    TRAVEL_EAST = 1,
    TRAVEL_SOUTH = 2,
    TRAVEL_WEST = 3
};

//we can add more later but this covers coasts, midwest
enum StartCity
{
    START_NEW_YORK = 0,
    START_CHICAGO = 1,
    START_LOS_ANGELES = 2
};

// implement via external dict
// TODO: AI query
enum BiomeType
{
    BIOME_UNKNOWN = 0,
    BIOME_WATER = 1,
    BIOME_RIVER = 2,
    BIOME_URBAN = 3,
    BIOME_SUBURBAN = 4,
    BIOME_INDUSTRIAL = 5,
    BIOME_FARMLAND = 6,
    BIOME_WILDERNESS = 7,
    BIOME_ROAD = 8, // represent major highways
    BIOME_SPECIAL = 9
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

std::vector<std::string> gMapLibrary;
LoadedMap gLoadedMap;
std::string gCurrentMapPath = "";
int gCurrentLevel = 0;

// grid to coordinate
constexpr float GLOBAL_ORIGIN_LATITUDE = 24.0f;
constexpr float GLOBAL_ORIGIN_LONGITUDE = -125.0f;
constexpr float GLOBAL_CELL_KM = 5.0f;
constexpr float KM_PER_LATITUDE = 111.0f;

GameState gState;
ShaderProgram gLightShader;
Vector2 gLightPosition = { 0.0f, 0.0f };
Vector2 gLightDirection = { 1.0f, 0.0f };

constexpr int MAX_MAP_LANTERNS = 8;

// inspired by another project
ShaderProgram gDamageShader;
float gDamageTimer = 0.0f;

enum GameMode
{
    MODE_MENU,
    MODE_MAP,
    MODE_INVENTORY,
    MODE_COMBAT,
    MODE_GEAR_COMPARE,
    MODE_WIN,
    MODE_LOSE
};

GameMode gGameMode = MODE_MENU;
// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void fillLevelData(std::vector<std::string>& mapLibrary);

void loadLevel(int levelNumber);

bool loadLevelDataFromTxt(const std::string& filePath, LoadedMap& level);

// map generation and biome map selection

std::string getBiomeFilePrefix(int biome);
void fillBiomeLevelData(std::vector<std::string>& mapLibrary, int biome);
void fillGenericLevelData(std::vector<std::string>& mapLibrary);
void loadLootLevelForCurrentBiome();

void loadLootData();

void loadBiomeTextData();
void loadGlobalBiomeData();
void saveGlobalBiomeData();
std::string getBiomeKey(int x, int y);
std::string getBiomeName(int biome);
std::string getMapLocationName();
int getBiomeAt(int x, int y);
int generateBiomeAt(int x, int y);
bool isBiomePassable(int biome); // i dont want to do water travel, plus its more interesting
std::string getTravelTextForBiome(int biome);
bool moveTravelPosition();

// call AI gemini bridge
std::string callAI(const std::string& promptType, const std::string& parameters);

//load save from text, im too lazy to add a new game button so just delete the file
void saveGame();
void loadGame();

std::string getTravelDirectionName();

void setRandomStartCity();
void setWorldGridFromCoordinate(float latitude, float longitude);
void updateCoordinateFromWorldGrid();
// im not very original at these namings... these are going to get confusing im sorry

void fillLevelData(std::vector<std::string>& mapLibrary);

//better text filling
void drawWrappedText(const std::string& text, int x, int y, int maxWidth, int fontSize, int lineSpacing, Color color);

//debug map
void drawLocalBiomeDebugGrid(int x, int y);

// AI can be slow so show loading instead of thinking freezing
void drawAIStatus(const std::string& promptType);

// generate biome data using real world data with AI
// I originally wanted to use census data but these raster images were too large
// now we fill as we play
void generateGlobalBiomesAroundPlayer();

void generateStartupAIData();

void generateEncounterData();

void generateLootDataWithAI();

void generateLootMapsForBiome(int biome);


void generateTimedAIDataIfNeeded();

int getRandomPassableBiome();

void generateGlobalBiomesAroundPlayerDirection();

//comment support for feeding into AI
std::string stripComment(const std::string& line);

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Maps");
    InitAudioDevice();
    gLightShader.load("shaders/vertex.glsl", "shaders/fragment.glsl");
    gDamageShader.load("shaders/vertex.glsl", "shaders/damage.glsl");

    gState.bgm = LoadMusicStream("assets/game/05 - NOT Silent Forest.wav");
    SetMusicVolume(gState.bgm, 0.33f);
    PlayMusicStream(gState.bgm);

    gState.jumpSound = LoadSound("assets/game/Dirt Jump.wav");
    gState.playerHitSound = LoadSound("assets/game/steve.wav");
    gState.zombieHitSound = LoadSound("assets/game/zombie.wav");

    /*
        ----------- STATS -----------
    */

    gState.day = 1;
    gState.hour = 8;
    gState.minute = 0;

    gState.lastAIGenerationDay = 1;

    gState.healthStat = 100;
    gState.foodStat = 100;
    gState.energyStat = 100;

    gState.distanceTravelled = 0;

    gState.travelDirection = TRAVEL_EAST;
    gState.currentBiome = BIOME_URBAN;

    setRandomStartCity();

    gState.armorName = "Jacket";
    gState.armorProtection = 1;
    gState.armorMobility = 3;

    gState.flashlightName = "Flashlight";
    gState.flashlightRadius = 45;
    gState.flashlightRange = 4;

    gState.firearmName = "Pistol";
    gState.firearmDamage = 2;
    gState.firearmSpeed = 2;

    gState.meleeName = "Crowbar";
    gState.meleeDamage = 2;
    gState.meleeSpeed = 1;

    gState.ammo = 18;
    //gState.supplies = 3;
    gState.actionMessage = "";
    gState.actionMessageTimer = 0.0f;
    gState.exitLockTimer = 0.0f;
    gState.pendingGearType = "";
    gState.pendingGearName = "";
    gState.pendingGearFirst = 0;
    gState.pendingGearSecond = 0;
    gState.godMode = false;
    gState.aiEnabled = true;
    loadLootData();
    loadBiomeTextData();
    loadGlobalBiomeData();
    loadGame();
    gState.currentBiome = getBiomeAt(gState.worldGridX, gState.worldGridY);
    generateStartupAIData();

    /*
        ----------- COMBAT -----------
    */

    gState.currentEnemy = nullptr;

    gState.combatHeartPosition = { 0.0f, 0.0f };
    gState.combatHeartVelocity = { 0.0f, 0.0f };

    gState.combatBox = { 300.0f, 180.0f, 400.0f, 240.0f };
    gState.combatEnemyAttack = { 0.0f, 0.0f, 0.0f, 0.0f };

    gState.combatTimer = 0.0f;
    gState.combatAttackTimer = 0.0f;
    gState.combatAttackDuration = 0.0f;

    gState.combatEnemyDamage = 0;
    gState.combatEnemySpeed = 0.0f;

    gState.combatTelegraphDuration = 0.0f;
    gState.combatAttackLive = false;

    gState.combatLerperPosition = { 0.0f, 0.0f };
    gState.combatLerperStart = { 0.0f, 0.0f };
    gState.combatLerperTarget = { 0.0f, 0.0f };
    gState.combatLerperRadius = 14.0f;

    gState.combatEnemyHealth = 0;
    gState.combatEnemyMaxHealth = 0;

    gState.combatAttackType = 0;
    gState.combatAttackInterval = 1.4f;

    gState.combatBallPosition = { 0.0f, 0.0f };
    gState.combatBallStart = { 0.0f, 0.0f };
    gState.combatBallTarget = { 0.0f, 0.0f };
    gState.combatBallRadius = 12.0f;

    gState.combatStoredPlayerPosition = { 0.0f, 0.0f };
    gState.combatStoredEnemyPosition = { 0.0f, 0.0f };

    gState.combatMeleeCooldown = 0.0f;
    gState.combatFirearmCooldown = 0.0f;

    /*
        ----------- MAP -----------
    */


    gState.map = nullptr;
    gState.mapBackground = LoadTexture("assets/game/map_background.png");

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

    /*
    gState.button = new Entity(
        { 87.5f, 220.0f }, // position
        { 75.0f, 75.0f },           // scale
        "assets/game/tile_0061.png",                 // texture file address
        BLOCK                                 // entity type
    );
    */

    gState.xochitl->setColliderDimensions({
        gState.xochitl->getScale().x / 3.5f,
        gState.xochitl->getScale().y / 3.0f
    });
    gState.xochitl->setAcceleration({ 0.0f, 0.0f });

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
    //multi ghost
    for (int i = 0; i < 6; i++) {
        Entity* ghost = new Entity(
            { ORIGIN.x + 300.0f, ORIGIN.y - 200.0f },
            { 100.0f, 100.0f },
            "assets/game/gosth.png",
            ATLAS,
            GHOST_ATLAS_DIMENSIONS,
            ghostAnimationAtlas,
            NPC
        );

        ghost->setAIState(IDLE);
        ghost->setColliderDimensions({
            ghost->getScale().x / 2.5f,
            ghost->getScale().y / 2.5f
            });
        ghost->setDirection(LEFT);
        ghost->setSpeed(50.0f);

        gState.ghosts.push_back(ghost);
    }

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
    fillLevelData(gMapLibrary);

    if (!gMapLibrary.empty())
    {
        loadLevel(0);
    }

    SetTargetFPS(FPS);
}

// control physics apply
bool isDebugFlying()
{
    return IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_LEFT) ||
        IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN);
}

/*
void advanceLevelOrQuit()
{
    gGameMode = MODE_INVENTORY;
}
*/

// removed switchcase :( to support multi level loading
void loadLevel(int levelNumber)
{
    if (gMapLibrary.empty()) return;

    if (levelNumber < 0 || levelNumber >= (int)gMapLibrary.size())
    {
        levelNumber = 0;
    }

    LoadedMap nextLevel;
    if (!loadLevelDataFromTxt(gMapLibrary[levelNumber], nextLevel))
    {
        return;
    }

    gCurrentLevel = levelNumber;
    gCurrentMapPath = gMapLibrary[levelNumber];
    gLoadedMap = nextLevel;

    if (gState.map != nullptr) {
        delete gState.map; //clean up
        gState.map = nullptr;
    }

    gState.currentWidth = gLoadedMap.width;
    gState.currentHeight = gLoadedMap.height;

    float halfWidth = (gLoadedMap.width * TILE_DIMENSION) / 2.0f;
    float halfHeight = (gLoadedMap.height * TILE_DIMENSION) / 2.0f;

    //gState.xochitl->setPosition({ ORIGIN.x - halfWidth + TILE_DIMENSION, ORIGIN.y - halfHeight + TILE_DIMENSION });

    for (size_t i = 0; i < gState.buttons.size(); i++)
    {
        delete gState.buttons[i];
    }
    gState.buttons.clear();

    for (size_t i = 0; i < gState.lootButtons.size(); i++)
    {
        delete gState.lootButtons[i];
    }
    gState.lootButtons.clear();

    for (size_t i = 0; i < gState.lanterns.size(); i++)
    {
        delete gState.lanterns[i];
    }
    gState.lanterns.clear();

    Vector2 playerSpawn = { ORIGIN.x - halfWidth + TILE_DIMENSION, ORIGIN.y - halfHeight + TILE_DIMENSION };

    for (int y = 0; y < gLoadedMap.height; y++)
    {
        for (int x = 0; x < gLoadedMap.width; x++)
        {
            int index = y * gLoadedMap.width + x;

            if (gLoadedMap.data[index] == 5)
            {
                float worldX = ORIGIN.x - halfWidth + x * TILE_DIMENSION + TILE_DIMENSION / 2.0f;
                float worldY = ORIGIN.y - halfHeight + y * TILE_DIMENSION + TILE_DIMENSION / 2.0f;

                Entity* button = new Entity(
                    { worldX, worldY },
                    { 75.0f, 75.0f },
                    "assets/game/tile_0061.png",
                    BLOCK
                );

                gState.buttons.push_back(button);

                Entity* lantern = new Entity(
                    { worldX, worldY },
                    { 1.0f, 1.0f },
                    "assets/game/tile_0061.png",
                    BLOCK
                );

                gState.lanterns.push_back(lantern);

                gLoadedMap.data[index] = 0;

                if (gState.buttons.size() == 1)
                {
                    int offsets[4][2] = {
                        { 1, 0 },
                        { -1, 0 },
                        { 0, 1 },
                        { 0, -1 }
                    };

                    for (int i = 0; i < 4; i++)
                    {
                        int spawnX = x + offsets[i][0];
                        int spawnY = y + offsets[i][1];

                        if (spawnX >= 0 && spawnX < gLoadedMap.width && spawnY >= 0 && spawnY < gLoadedMap.height)
                        {
                            int spawnIndex = spawnY * gLoadedMap.width + spawnX;

                            if (gLoadedMap.data[spawnIndex] == 0)
                            {
                                playerSpawn = {
                                    ORIGIN.x - halfWidth + spawnX * TILE_DIMENSION + TILE_DIMENSION / 2.0f,
                                    ORIGIN.y - halfHeight + spawnY * TILE_DIMENSION + TILE_DIMENSION / 2.0f
                                };
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    gState.xochitl->setPosition(playerSpawn);

    // dumb fix if broken spawn on door insta exit
    gState.exitLockTimer = 15.0f;

    gState.map = new Map(
        gLoadedMap.width, gLoadedMap.height,
        gLoadedMap.data.data(),
        "assets/game/tileset.png",
        TILE_DIMENSION,
        4, 1,
        ORIGIN
    );

    int lootCount = GetRandomValue(2, 5);

    for (int i = 0; i < lootCount; i++)
    {
        int tileX = 1;
        int tileY = 1;
        bool found = false;

        // try to find a spot to place loot
        for (int tries = 0; tries < 200; tries++)
        {
            tileX = GetRandomValue(1, gLoadedMap.width - 2);
            tileY = GetRandomValue(1, gLoadedMap.height - 2);

            int index = tileY * gLoadedMap.width + tileX;

            if (gLoadedMap.data[index] == 0)
            {
                float worldX = ORIGIN.x - halfWidth + tileX * TILE_DIMENSION + TILE_DIMENSION / 2.0f;
                float worldY = ORIGIN.y - halfHeight + tileY * TILE_DIMENSION + TILE_DIMENSION / 2.0f;

                Vector2 playerPos = gState.xochitl->getPosition();
                float dx = worldX - playerPos.x;
                float dy = worldY - playerPos.y;
                float dist2 = dx * dx + dy * dy;

                if (dist2 > TILE_DIMENSION * TILE_DIMENSION * 4.0f)
                {
                    Entity* lootButton = new Entity(
                        { worldX, worldY },
                        { 75.0f, 75.0f },
                        "assets/game/tile_0000.png",
                        BLOCK
                    );

                    gState.lootButtons.push_back(lootButton);
                    found = true;
                    break;
                }
            }
        }
    }

    // spawn lightsources

    int lanternCount = GetRandomValue(2, 4);

    for (int i = 0; i < lanternCount; i++)
    {
        int tileX = 1;
        int tileY = 1;
        bool found = false;

        for (int tries = 0; tries < 200; tries++)
        {
            tileX = GetRandomValue(1, gLoadedMap.width - 2);
            tileY = GetRandomValue(1, gLoadedMap.height - 2);

            int index = tileY * gLoadedMap.width + tileX;

            if (gLoadedMap.data[index] == 0)
            {
                float worldX = ORIGIN.x - halfWidth + tileX * TILE_DIMENSION + TILE_DIMENSION / 2.0f;
                float worldY = ORIGIN.y - halfHeight + tileY * TILE_DIMENSION + TILE_DIMENSION / 2.0f;

                Vector2 playerPos = gState.xochitl->getPosition();
                float dx = worldX - playerPos.x;
                float dy = worldY - playerPos.y;
                float dist2 = dx * dx + dy * dy;

                if (dist2 > TILE_DIMENSION * TILE_DIMENSION * 6.0f)
                {
                    bool overlapsOther = false;

                    for (size_t j = 0; j < gState.lanterns.size(); j++)
                    {
                        Vector2 otherPos = gState.lanterns[j]->getPosition();
                        float odx = worldX - otherPos.x;
                        float ody = worldY - otherPos.y;
                        float otherDist2 = odx * odx + ody * ody;

                        if (otherDist2 < TILE_DIMENSION * TILE_DIMENSION * 9.0f)
                        {
                            overlapsOther = true;
                            break;
                        }
                    }

                    if (!overlapsOther)
                    {
                        Entity* lantern = new Entity(
                            { worldX, worldY },
                            { 55.0f, 55.0f },
                            "assets/game/tile_0061.png",
                            BLOCK
                        );

                        gState.lanterns.push_back(lantern);
                        found = true;
                        break;
                    }
                }
            }
        }
    }

    for (size_t i = 0; i < gState.ghosts.size(); i++) {
        gState.ghosts[i]->deactivate();
    }

    int ghostCount = GetRandomValue(1, 4);
    if (ghostCount > (int)gState.ghosts.size()) ghostCount = (int)gState.ghosts.size();

    // alternate spawning so we account for blocks
    for (int i = 0; i < ghostCount; i++) {
        int tileX = 1;
        int tileY = 1;
        bool found = false;

        for (int tries = 0; tries < 200; tries++) {
            tileX = GetRandomValue(1, gLoadedMap.width - 2);
            tileY = GetRandomValue(1, gLoadedMap.height - 2);

            int index = tileY * gLoadedMap.width + tileX;

            if (gLoadedMap.data[index] == 0) {
                float worldX = ORIGIN.x - halfWidth + tileX * TILE_DIMENSION + TILE_DIMENSION / 2.0f;
                float worldY = ORIGIN.y - halfHeight + tileY * TILE_DIMENSION + TILE_DIMENSION / 2.0f;

                Vector2 playerPos = gState.xochitl->getPosition();
                float dx = worldX - playerPos.x;
                float dy = worldY - playerPos.y;
                float dist2 = dx * dx + dy * dy;

                if (dist2 > TILE_DIMENSION * TILE_DIMENSION * 9.0f) {
                    bool overlapsOther = false;

                    for (int j = 0; j < i; j++) {
                        Vector2 otherPos = gState.ghosts[j]->getPosition();
                        float odx = worldX - otherPos.x;
                        float ody = worldY - otherPos.y;
                        float otherDist2 = odx * odx + ody * ody;

                        if (otherDist2 < TILE_DIMENSION * TILE_DIMENSION * 4.0f) {
                            overlapsOther = true;
                            break;
                        }
                    }

                    if (!overlapsOther) {
                        gState.ghosts[i]->setPosition({ worldX, worldY });
                        found = true;
                        break;
                    }
                }
            }
        }

        if (!found) {
            gState.ghosts[i]->setPosition({ ORIGIN.x + halfWidth - TILE_DIMENSION * (i + 2), ORIGIN.y - halfHeight + TILE_DIMENSION * 2.0f });
        }

        int aiPick = GetRandomValue(0, 1);
        if (aiPick == 0) {
            gState.ghosts[i]->setAIType(WANDERER);
            gState.ghosts[i]->setAIState(WALKING);
        }
        else {
            gState.ghosts[i]->setAIType(FOLLOWER);
            gState.ghosts[i]->setAIState(IDLE);
        }

        gState.ghosts[i]->activate();
    }
}

//respawn if killed
void resetGameToLevel1()
{
    gState.hearts = MAX_HEARTS;
    gState.hitCooldown = 0.0f;
    loadLevel(0);
}

void fillLevelData(std::vector<std::string>& mapLibrary)
{
    mapLibrary.clear();

    DIR* dir = opendir("maps");
    if (dir == nullptr) return;

    dirent* entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        std::string name = entry->d_name;

        if (name == "." || name == "..") continue;

        if (name.size() >= 4 && name.substr(name.size() - 4) == ".txt")
        {
            mapLibrary.push_back("maps/" + name);
        }
    }

    closedir(dir);

    std::sort(mapLibrary.begin(), mapLibrary.end());
}

// we just use file prefix since i'm too lazy to implement via comment or directory
std::string getBiomeFilePrefix(int biome)
{
    if (biome == BIOME_WATER) return "water_";
    if (biome == BIOME_RIVER) return "river_";
    if (biome == BIOME_URBAN) return "urban_";
    if (biome == BIOME_SUBURBAN) return "suburban_";
    if (biome == BIOME_INDUSTRIAL) return "industrial_";
    if (biome == BIOME_FARMLAND) return "farmland_";
    if (biome == BIOME_WILDERNESS) return "wilderness_";
    if (biome == BIOME_ROAD) return "road_";
    if (biome == BIOME_SPECIAL) return "special_";

    return "";
}

void fillBiomeLevelData(std::vector<std::string>& mapLibrary, int biome)
{
    mapLibrary.clear();

    std::string prefix = getBiomeFilePrefix(biome);

    DIR* dir = opendir("maps");
    if (dir == nullptr) return;

    dirent* entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        std::string name = entry->d_name;

        if (name == "." || name == "..") continue;

        if (name.size() >= 4 && name.substr(name.size() - 4) == ".txt")
        {
            if (!prefix.empty() && name.find(prefix) == 0)
            {
                mapLibrary.push_back("maps/" + name);
            }
        }
    }

    closedir(dir);

    std::sort(mapLibrary.begin(), mapLibrary.end());
}

void fillGenericLevelData(std::vector<std::string>& mapLibrary)
{
    mapLibrary.clear();

    DIR* dir = opendir("maps");
    if (dir == nullptr) return;

    dirent* entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        std::string name = entry->d_name;

        if (name == "." || name == "..") continue;

        if (name.size() >= 4 && name.substr(name.size() - 4) == ".txt")
        {
            if (name.find("level") == 0)
            {
                mapLibrary.push_back("maps/" + name);
            }
        }
    }

    closedir(dir);

    std::sort(mapLibrary.begin(), mapLibrary.end());
}

void loadLootLevelForCurrentBiome()
{
    fillBiomeLevelData(gMapLibrary, gState.currentBiome);

    if (gMapLibrary.empty())
    {
        fillGenericLevelData(gMapLibrary);
    }

    if (gMapLibrary.empty())
    {
        fillLevelData(gMapLibrary);
    }

    if (!gMapLibrary.empty())
    {
        int pick = GetRandomValue(0, (int)gMapLibrary.size() - 1);
        loadLevel(pick);
    }
}

// strip comment lines before processing
std::string stripComment(const std::string& line)
{
    size_t comment = line.find('#');

    if (comment == std::string::npos)
    {
        return line;
    }

    return line.substr(0, comment);
}

// switch from loaded file
bool loadLevelDataFromTxt(const std::string& filePath, LoadedMap& level)
{
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    std::vector<std::vector<unsigned int>> rows;
    std::string line;

    while (std::getline(file, line))
    {
        line = stripComment(line);

        if (line.empty()) continue;

        std::stringstream ss(line);
        std::vector<unsigned int> row;
        unsigned int tile;

        while (ss >> tile)
        {
            row.push_back(tile);
        }

        if (!row.empty())
        {
            rows.push_back(row);
        }
    }

    if (rows.empty()) return false;

    // use dynamic width and height instead
    int width = (int)rows[0].size();
    int height = (int)rows.size();

    for (const auto& row : rows)
    {
        if ((int)row.size() != width)
        {
            return false;
        }
    }

    level.width = width;
    level.height = height;
    level.data.clear();
    level.data.reserve(width * height);

    for (const auto& row : rows)
    {
        for (unsigned int tile : row)
        {
            level.data.push_back(tile);
        }
    }

    return true;
}

// load from files
void loadLootData()
{
    gState.armorLootNames.clear();
    gState.armorLootProtection.clear();
    gState.armorLootMobility.clear();

    gState.meleeLootNames.clear();
    gState.meleeLootDamage.clear();
    gState.meleeLootSpeed.clear();

    gState.firearmLootNames.clear();
    gState.firearmLootDamage.clear();
    gState.firearmLootSpeed.clear();

    gState.flashlightLootNames.clear();
    gState.flashlightLootRadius.clear();
    gState.flashlightLootRange.clear();

    std::ifstream armorFile("loot/armor.txt");
    std::string name;
    int first;
    int second;
    std::string line;

    while (std::getline(armorFile, line))
    {
        line = stripComment(line);

        std::stringstream ss(line);

        if (ss >> name >> first >> second)
        {
            gState.armorLootNames.push_back(name);
            gState.armorLootProtection.push_back(first);
            gState.armorLootMobility.push_back(second);
        }
    }

    std::ifstream meleeFile("loot/melee.txt");

    while (std::getline(meleeFile, line))
    {
        line = stripComment(line);

        std::stringstream ss(line);

        if (ss >> name >> first >> second)
        {
            gState.meleeLootNames.push_back(name);
            gState.meleeLootDamage.push_back(first);
            gState.meleeLootSpeed.push_back(second);
        }
    }

    std::ifstream firearmFile("loot/firearm.txt");

    while (std::getline(firearmFile, line))
    {
        line = stripComment(line);

        std::stringstream ss(line);

        if (ss >> name >> first >> second)
        {
            gState.firearmLootNames.push_back(name);
            gState.firearmLootDamage.push_back(first);
            gState.firearmLootSpeed.push_back(second);
        }
    }

    // flashlight format: name angle range
    std::ifstream flashlightFile("loot/flashlight.txt");

    while (std::getline(flashlightFile, line))
    {
        line = stripComment(line);

        std::stringstream ss(line);

        if (ss >> name >> first >> second)
        {
            gState.flashlightLootNames.push_back(name);
            gState.flashlightLootRadius.push_back(first);
            gState.flashlightLootRange.push_back(second);
        }
    }
}

// optimize this to lower loading time
void loadBiomeTextData()
{
    for (int i = 0; i < 10; i++)
    {
        gState.biomeTravelTexts[i].clear();
    }

    std::ifstream file("data/biome_travel_texts.txt");
    if (!file.is_open()) return;

    std::string line;

    while (std::getline(file, line))
    {
        line = stripComment(line);

        if (line.empty()) continue;

        size_t split = line.find('|');
        if (split == std::string::npos) continue;

        int biome = std::stoi(line.substr(0, split));
        std::string text = line.substr(split + 1);

        if (biome >= 0 && biome < 10)
        {
            gState.biomeTravelTexts[biome].push_back(text);
        }
    }
}

void loadGlobalBiomeData()
{
    gState.globalBiomeMap.clear();

    std::ifstream file("data/global_biomes.txt");
    if (!file.is_open()) return;

    int x;
    int y;
    int biome;
    std::string line;

    while (std::getline(file, line))
    {
        line = stripComment(line);

        std::stringstream ss(line);

        if (ss >> x >> y >> biome)
        {
            gState.globalBiomeMap[getBiomeKey(x, y)] = biome;
        }
    }
}

void saveGlobalBiomeData()
{
    std::ofstream file("data/global_biomes.txt");
    if (!file.is_open()) return;

    for (auto it = gState.globalBiomeMap.begin(); it != gState.globalBiomeMap.end(); ++it)
    {
        std::string key = it->first;
        size_t split = key.find(',');

        if (split == std::string::npos) continue;

        std::string x = key.substr(0, split);
        std::string y = key.substr(split + 1);

        file << x << " " << y << " " << it->second << "\n";
    }
}

std::string getBiomeKey(int x, int y)
{
    return std::to_string(x) + "," + std::to_string(y);
}

std::string getBiomeName(int biome)
{
    if (biome == BIOME_WATER) return "Water";
    if (biome == BIOME_RIVER) return "River";
    if (biome == BIOME_URBAN) return "Urban";
    if (biome == BIOME_SUBURBAN) return "Suburban";
    if (biome == BIOME_INDUSTRIAL) return "Industrial";
    if (biome == BIOME_FARMLAND) return "Farmland";
    if (biome == BIOME_WILDERNESS) return "Wilderness";
    if (biome == BIOME_ROAD) return "Road";
    if (biome == BIOME_SPECIAL) return "Landmark"; // implement some feature to detect landmarks and special event text

    return "Unknown";
}

//location name on map
std::string getMapLocationName()
{
    std::string name = gCurrentMapPath;

    size_t slash = name.find_last_of("/\\");
    if (slash != std::string::npos)
    {
        name = name.substr(slash + 1);
    }

    size_t dot = name.find_last_of(".");
    if (dot != std::string::npos)
    {
        name = name.substr(0, dot);
    }

    for (size_t i = 0; i < name.size(); i++)
    {
        if (name[i] == '_')
        {
            name[i] = ' ';
        }
    }

    if (!name.empty())
    {
        name[0] = (char)toupper(name[0]);
    }

    return name;
}

void drawAIStatus(const std::string& promptType)
{
    if (!IsWindowReady()) return;

    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    DrawRectangle(250, 210, 500, 180, Fade(BLACK, 0.85f));
    DrawRectangleLines(250, 210, 500, 180, WHITE);

    DrawCircle(310, 300, 18.0f, SKYBLUE);
    DrawCircleLines(310, 300, 24.0f, WHITE);

    DrawText("Calling AI", 350, 250, 36, WHITE);
    DrawText(TextFormat("Type: %s", promptType.c_str()), 350, 300, 24, GRAY);
    DrawText("Please wait...", 350, 335, 22, GRAY);

    EndDrawing();
}

std::string callAI(const std::string& promptType, const std::string& parameters)
{
    // disable AI
    if (!gState.aiEnabled)
    {
        return "";
    }
    // adjust this to work with your python instalL!
    #ifdef _WIN32
        std::string command = "py -3 ai/gemini_bridge.py --key %GEMINI_API_KEY% --type " + promptType + " --output data/ai_output.txt";
    #else
        std::string command = "python3 ai/gemini_bridge.py --key \"$GEMINI_API_KEY\" --type " + promptType + " --output data/ai_output.txt";
    #endif

    if (!parameters.empty())
    {
        command += " " + parameters;
    }

    drawAIStatus(promptType);

    int result = system(command.c_str());

    if (result != 0) return "";

    std::ifstream outputFile("data/ai_output.txt");
    if (!outputFile.is_open()) return "";

    std::stringstream buffer;
    buffer << outputFile.rdbuf();

    return buffer.str();
}

int getBiomeAt(int x, int y)
{
    std::string key = getBiomeKey(x, y);

    if (gState.globalBiomeMap.find(key) == gState.globalBiomeMap.end())
    {
        generateGlobalBiomesAroundPlayerDirection();
        loadGlobalBiomeData();

        if (gState.globalBiomeMap.find(key) == gState.globalBiomeMap.end())
        {
            gState.globalBiomeMap[key] = generateBiomeAt(x, y);
            saveGlobalBiomeData();
        }
    }

    return gState.globalBiomeMap[key];
}

// fallback if no AI
int generateBiomeAt(int x, int y)
{
    if (x == 0 && y == 0) return BIOME_URBAN;

    std::string parameters = "";
    parameters += "--grid-x " + std::to_string(x);
    parameters += " --grid-y " + std::to_string(y);
    parameters += " --start-latitude " + std::to_string(gState.startLatitude);
    parameters += " --start-longitude " + std::to_string(gState.startLongitude);
    parameters += " --latitude " + std::to_string(gState.latitude);
    parameters += " --longitude " + std::to_string(gState.longitude);
    parameters += " --start-city \"" + gState.currentLocation + "\"";

    std::string aiOutput = callAI("biome", parameters);

    std::stringstream aiStream(aiOutput);
    int aiBiome = BIOME_UNKNOWN;
    aiStream >> aiBiome;

    if (aiBiome >= BIOME_WATER && aiBiome <= BIOME_SPECIAL)
    {
        return aiBiome;
    }

    int distance = abs(x) + abs(y);

    if (distance == 1) return BIOME_SUBURBAN;

    if (x % 17 == 0 && y % 5 == 0) return BIOME_WATER;
    if (x % 9 == 0 || y % 11 == 0) return BIOME_RIVER;

    int roll = GetRandomValue(0, 99);

    if (distance <= 3)
    {
        if (roll < 30) return BIOME_SUBURBAN;
        if (roll < 55) return BIOME_INDUSTRIAL;
        if (roll < 75) return BIOME_ROAD;
        if (roll < 90) return BIOME_FARMLAND;
        return BIOME_WILDERNESS;
    }

    if (roll < 10) return BIOME_SUBURBAN;
    if (roll < 22) return BIOME_INDUSTRIAL;
    if (roll < 50) return BIOME_FARMLAND;
    if (roll < 85) return BIOME_WILDERNESS;
    if (roll < 95) return BIOME_ROAD;

    return BIOME_SPECIAL;
}

bool isBiomePassable(int biome)
{
    if (biome == BIOME_WATER) return false;

    return true;
}

std::string getTravelTextForBiome(int biome)
{
    if (biome >= 0 && biome < 10 && !gState.biomeTravelTexts[biome].empty())
    {
        int pick = GetRandomValue(0, (int)gState.biomeTravelTexts[biome].size() - 1);
        return gState.biomeTravelTexts[biome][pick];
    }
    // fallback
    return "You travel another careful kilometer through the ruined country.";
}

// we care about direction since we dont want to waste AI gen on grids behind us
void generateGlobalBiomesAroundPlayerDirection()
{
    int anchorX = gState.worldGridX;
    int anchorY = gState.worldGridY;

    if (gState.travelDirection == TRAVEL_NORTH)
    {
        anchorX = gState.worldGridX;
        anchorY = gState.worldGridY - 2;
    }
    else if (gState.travelDirection == TRAVEL_EAST)
    {
        anchorX = gState.worldGridX + 2;
        anchorY = gState.worldGridY;
    }
    else if (gState.travelDirection == TRAVEL_SOUTH)
    {
        anchorX = gState.worldGridX;
        anchorY = gState.worldGridY + 2;
    }
    else if (gState.travelDirection == TRAVEL_WEST)
    {
        anchorX = gState.worldGridX - 2;
        anchorY = gState.worldGridY;
    }

    std::string parameters = "";
    parameters += "--grid-x " + std::to_string(anchorX);
    parameters += " --grid-y " + std::to_string(anchorY);
    parameters += " --player-grid-x " + std::to_string(gState.worldGridX);
    parameters += " --player-grid-y " + std::to_string(gState.worldGridY);
    parameters += " --latitude " + std::to_string(gState.latitude);
    parameters += " --longitude " + std::to_string(gState.longitude);
    parameters += " --direction \"" + getTravelDirectionName() + "\"";
    parameters += " --radius-cells 2";
    parameters += " --cell-count 25";

    std::string result = callAI("global_biomes", parameters);

    loadGlobalBiomeData();
    saveGlobalBiomeData();

    gState.travelLog = "AI generated nearby global biome cells and appended them to the shared world map.";
    gState.travelDebugLog = "";
}

// we don't need water generation
int getRandomPassableBiome()
{
    int biome = GetRandomValue(BIOME_URBAN, BIOME_SPECIAL);

    if (biome == BIOME_WATER)
    {
        biome = BIOME_SUBURBAN;
    }

    return biome;
}

void generateEncounterData()
{
    std::string parameters = "";
    parameters += "--count 8";
    parameters += " --day " + std::to_string(gState.day);
    parameters += " --hour " + std::to_string(gState.hour);
    parameters += " --biome \"" + getBiomeName(gState.currentBiome) + "\"";

    std::string result = callAI("encounters", parameters);
}

void generateLootDataWithAI()
{
    std::string parameters = "";
    parameters += "--armor-count 4";
    parameters += " --melee-count 5";
    parameters += " --firearm-count 3";
    parameters += " --flashlight-count 3";
    parameters += " --name-limit 36";

    std::string result = callAI("loot_table", parameters);

    loadLootData();
}

void generateLootMapsForBiome(int biome)
{
    std::string parameters = "";
    parameters += "--biome \"" + getBiomeName(biome) + "\"";
    parameters += " --prefix \"" + getBiomeFilePrefix(biome) + "\"";
    parameters += " --map-count 5";
    parameters += " --map-width 35";
    parameters += " --map-height 22";

    std::string result = callAI("loot_maps", parameters);

    fillLevelData(gMapLibrary);
}

void generateStartupAIData()
{
    //generateEncounterData();
    //we have enough loot, uncomment to gen more
    //generateLootDataWithAI();
    generateLootMapsForBiome(gState.currentBiome);
    generateLootMapsForBiome(getRandomPassableBiome());
    generateLootMapsForBiome(getRandomPassableBiome());
}

void generateTimedAIDataIfNeeded()
{
    if (gState.day - gState.lastAIGenerationDay < 5) //increase or decrease to increase days betwen AI GEN
    {
        return;
    }

    //generateEncounterData();
    //generateLootDataWithAI();
    generateLootMapsForBiome(getRandomPassableBiome());
    generateLootMapsForBiome(getRandomPassableBiome());

    gState.lastAIGenerationDay = gState.day;
    saveGame();
}

void saveGame()
{
    #ifdef _WIN32
        system("mkdir saves >nul 2>nul");
    #else
        system("mkdir -p saves");
    #endif

    std::ofstream file("saves/save.txt");
    if (!file.is_open()) return;

    file << "day " << gState.day << "\n";
    file << "hour " << gState.hour << "\n";
    file << "minute " << gState.minute << "\n";

    file << "lastAIGenerationDay " << gState.lastAIGenerationDay << "\n";

    file << "healthStat " << gState.healthStat << "\n";
    file << "foodStat " << gState.foodStat << "\n";
    file << "energyStat " << gState.energyStat << "\n";

    file << "distanceTravelled " << gState.distanceTravelled << "\n";
    file << "worldGridX " << gState.worldGridX << "\n";
    file << "worldGridY " << gState.worldGridY << "\n";
    file << "localKmX " << gState.localKmX << "\n";
    file << "localKmY " << gState.localKmY << "\n";

    file << "travelDirection " << gState.travelDirection << "\n";
    file << "currentBiome " << gState.currentBiome << "\n";

    file << "aiEnabled " << gState.aiEnabled << "\n";

    file << "startCity " << gState.startCity << "\n";
    file << "startLatitude " << gState.startLatitude << "\n";
    file << "startLongitude " << gState.startLongitude << "\n";
    file << "latitude " << gState.latitude << "\n";
    file << "longitude " << gState.longitude << "\n";

    file << "currentLocation " << gState.currentLocation << "\n";
    file << "travelLog " << gState.travelLog << "\n";
    file << "travelDebugLog " << gState.travelDebugLog << "\n";

    file << "armorName " << gState.armorName << "\n";
    file << "armorProtection " << gState.armorProtection << "\n";
    file << "armorMobility " << gState.armorMobility << "\n";

    file << "flashlightName " << gState.flashlightName << "\n";
    file << "flashlightRadius " << gState.flashlightRadius << "\n";
    file << "flashlightRange " << gState.flashlightRange << "\n";

    file << "firearmName " << gState.firearmName << "\n";
    file << "firearmDamage " << gState.firearmDamage << "\n";
    file << "firearmSpeed " << gState.firearmSpeed << "\n";

    file << "meleeName " << gState.meleeName << "\n";
    file << "meleeDamage " << gState.meleeDamage << "\n";
    file << "meleeSpeed " << gState.meleeSpeed << "\n";

    file << "ammo " << gState.ammo << "\n";
}

void loadGame()
{
    std::ifstream file("saves/save.txt");
    if (!file.is_open()) return;

    std::string key;

    while (file >> key)
    {
        if (key == "day") file >> gState.day;
        else if (key == "hour") file >> gState.hour;
        else if (key == "minute") file >> gState.minute;

        else if (key == "lastAIGenerationDay") file >> gState.lastAIGenerationDay;

        else if (key == "healthStat") file >> gState.healthStat;
        else if (key == "foodStat") file >> gState.foodStat;
        else if (key == "energyStat") file >> gState.energyStat;

        else if (key == "distanceTravelled") file >> gState.distanceTravelled;
        else if (key == "worldGridX") file >> gState.worldGridX;
        else if (key == "worldGridY") file >> gState.worldGridY;
        else if (key == "localKmX") file >> gState.localKmX;
        else if (key == "localKmY") file >> gState.localKmY;
        else if (key == "travelDirection") file >> gState.travelDirection;
        else if (key == "currentBiome") file >> gState.currentBiome;
        else if (key == "aiEnabled") file >> gState.aiEnabled;
        else if (key == "startCity") file >> gState.startCity;
        else if (key == "startLatitude") file >> gState.startLatitude;
        else if (key == "startLongitude") file >> gState.startLongitude;
        else if (key == "latitude") file >> gState.latitude;
        else if (key == "longitude") file >> gState.longitude;

        else if (key == "currentLocation")
        {
            file.ignore();
            std::getline(file, gState.currentLocation);
        }
        else if (key == "travelLog")
        {
            file.ignore();
            std::getline(file, gState.travelLog);
        }
        else if (key == "travelDebugLog")
        {
            file.ignore();
            std::getline(file, gState.travelDebugLog);
        }

        else if (key == "armorName")
        {
            file.ignore();
            std::getline(file, gState.armorName);
        }
        else if (key == "armorProtection") file >> gState.armorProtection;
        else if (key == "armorMobility") file >> gState.armorMobility;

        else if (key == "flashlightName")
        {
            file.ignore();
            std::getline(file, gState.flashlightName);
        }
        else if (key == "flashlightRadius") file >> gState.flashlightRadius;
        else if (key == "flashlightRange") file >> gState.flashlightRange;

        else if (key == "firearmName")
        {
            file.ignore();
            std::getline(file, gState.firearmName);
        }
        else if (key == "firearmDamage") file >> gState.firearmDamage;
        else if (key == "firearmSpeed") file >> gState.firearmSpeed;

        else if (key == "meleeName")
        {
            file.ignore();
            std::getline(file, gState.meleeName);
        }
        else if (key == "meleeDamage") file >> gState.meleeDamage;
        else if (key == "meleeSpeed") file >> gState.meleeSpeed;

        else if (key == "ammo") file >> gState.ammo;
    }

    if (gState.healthStat > 100) gState.healthStat = 100;
    if (gState.foodStat > 100) gState.foodStat = 100;
    if (gState.energyStat > 100) gState.energyStat = 100;

    if (gState.healthStat < 0) gState.healthStat = 0;
    if (gState.foodStat < 0) gState.foodStat = 0;
    if (gState.energyStat < 0) gState.energyStat = 0;
}

std::string getTravelDirectionName()
{
    if (gState.travelDirection == TRAVEL_NORTH) return "North";
    if (gState.travelDirection == TRAVEL_EAST) return "East";
    if (gState.travelDirection == TRAVEL_SOUTH) return "South";
    if (gState.travelDirection == TRAVEL_WEST) return "West";

    return "Unknown";
}

bool moveTravelPosition()
{
    int nextWorldGridX = gState.worldGridX;
    int nextWorldGridY = gState.worldGridY;
    int nextLocalKmX = gState.localKmX;
    int nextLocalKmY = gState.localKmY;

    if (gState.travelDirection == TRAVEL_NORTH)
    {
        nextLocalKmY -= 1;

        if (nextLocalKmY < 0)
        {
            nextLocalKmY = 4;
            nextWorldGridY -= 1;
        }
    }
    else if (gState.travelDirection == TRAVEL_EAST)
    {
        nextLocalKmX += 1;

        if (nextLocalKmX > 4)
        {
            nextLocalKmX = 0;
            nextWorldGridX += 1;
        }
    }
    else if (gState.travelDirection == TRAVEL_SOUTH)
    {
        nextLocalKmY += 1;

        if (nextLocalKmY > 4)
        {
            nextLocalKmY = 0;
            nextWorldGridY += 1;
        }
    }
    else if (gState.travelDirection == TRAVEL_WEST)
    {
        nextLocalKmX -= 1;

        if (nextLocalKmX < 0)
        {
            nextLocalKmX = 4;
            nextWorldGridX -= 1;
        }
    }

    int nextBiome = getBiomeAt(nextWorldGridX, nextWorldGridY);

    if (!isBiomePassable(nextBiome))
    {
        gState.travelLog = TextFormat(
            "You cannot travel %s. Impassable %s blocks the way.",
            getTravelDirectionName().c_str(),
            getBiomeName(nextBiome).c_str()
        );

        gState.travelDebugLog = TextFormat(
            "Blocked at global cell [%i, %i].",
            nextWorldGridX,
            nextWorldGridY
        );

        return false;
    }

    gState.worldGridX = nextWorldGridX;
    gState.worldGridY = nextWorldGridY;
    gState.localKmX = nextLocalKmX;
    gState.localKmY = nextLocalKmY;
    gState.currentBiome = nextBiome;

    return true;
}

//temp for testing since we'll load from file later
void setRandomStartCity()
{
    gState.startCity = GetRandomValue(0, 2);
    //forcing this for demo, delete once you have API!!!
    gState.startCity = START_CHICAGO;

    gState.worldGridX = 0;
    gState.worldGridY = 0;
    gState.localKmX = 0;
    gState.localKmY = 0;

    if (gState.startCity == START_NEW_YORK)
    {
        gState.startLatitude = 40.7128f;
        gState.startLongitude = -74.0060f;
        gState.latitude = gState.startLatitude;
        gState.longitude = gState.startLongitude;
        gState.currentLocation = "New York Outskirts";
        gState.travelDirection = TRAVEL_WEST;
        gState.travelLog = "You leave the edge of New York with the skyline behind you and the roads ahead gone quiet.";
    }
    else if (gState.startCity == START_CHICAGO)
    {
        gState.startLatitude = 41.8781f;
        gState.startLongitude = -87.6298f;
        gState.latitude = gState.startLatitude;
        gState.longitude = gState.startLongitude;
        gState.currentLocation = "Chicago Outskirts";
        gState.travelDirection = TRAVEL_WEST;
        gState.travelLog = "You leave the edge of Chicago with the lake wind at your back and the dead streets thinning behind you.";
    }
    else
    {
        gState.startLatitude = 34.0522f;
        gState.startLongitude = -118.2437f;
        gState.latitude = gState.startLatitude;
        gState.longitude = gState.startLongitude;
        gState.currentLocation = "Los Angeles Outskirts";
        gState.travelDirection = TRAVEL_EAST;
        gState.travelLog = "You leave the edge of Los Angeles with the basin fading behind you and the dry roads stretching ahead.";
    }
    setWorldGridFromCoordinate(gState.latitude, gState.longitude);
}

void setWorldGridFromCoordinate(float latitude, float longitude)
{
    float latitudeRadians = latitude * 0.0174533f;
    float kmPerLongitude = KM_PER_LATITUDE * cosf(latitudeRadians);

    if (kmPerLongitude < 1.0f)
    {
        kmPerLongitude = 1.0f;
    }

    float kmX = (longitude - GLOBAL_ORIGIN_LONGITUDE) * kmPerLongitude;
    float kmY = (latitude - GLOBAL_ORIGIN_LATITUDE) * KM_PER_LATITUDE;

    gState.worldGridX = (int)(kmX / GLOBAL_CELL_KM);
    gState.worldGridY = (int)(kmY / GLOBAL_CELL_KM);

    gState.localKmX = (int)kmX % 5;
    gState.localKmY = (int)kmY % 5;

    if (gState.localKmX < 0) gState.localKmX += 5;
    if (gState.localKmY < 0) gState.localKmY += 5;
}

void updateCoordinateFromWorldGrid()
{
    float totalKmX = (float)(gState.worldGridX * 5 + gState.localKmX);
    float totalKmY = (float)(gState.worldGridY * 5 + gState.localKmY);

    gState.latitude = GLOBAL_ORIGIN_LATITUDE + (totalKmY / KM_PER_LATITUDE);

    float latitudeRadians = gState.latitude * 0.0174533f;
    float kmPerLongitude = KM_PER_LATITUDE * cosf(latitudeRadians);

    if (kmPerLongitude < 1.0f)
    {
        kmPerLongitude = 1.0f;
    }

    gState.longitude = GLOBAL_ORIGIN_LONGITUDE + (totalKmX / kmPerLongitude);
}

// helper to fit text
void drawWrappedText(const std::string& text, int x, int y, int maxWidth, int fontSize, int lineSpacing, Color color)
{
    std::istringstream stream(text);
    std::string word;
    std::string line;
    int drawY = y;

    while (stream >> word)
    {
        std::string testLine = line.empty() ? word : line + " " + word;

        if (MeasureText(testLine.c_str(), fontSize) > maxWidth)
        {
            if (!line.empty())
            {
                DrawText(line.c_str(), x, drawY, fontSize, color);
                drawY += fontSize + lineSpacing;
            }
            line = word;
        }
        else
        {
            line = testLine;
        }
    }

    if (!line.empty())
    {
        DrawText(line.c_str(), x, drawY, fontSize, color);
    }
}

void drawLocalBiomeDebugGrid(int x, int y)
{
    DrawRectangle(x, y, 120, 100, Fade(BLACK, 0.5f));

    int cellWidth = 18;
    int cellHeight = 16;

    for (int row = 0; row < 5; row++)
    {
        int gy = gState.worldGridY + 2 - row;

        for (int col = 0; col < 5; col++)
        {
            int gx = gState.worldGridX - 2 + col;
            std::string key = getBiomeKey(gx, gy);
            std::string value = "*";
            Color color = WHITE;

            // better player location indicator
            if (gx == gState.worldGridX && gy == gState.worldGridY)
            {
                value = TextFormat("%i", gState.currentBiome);
                color = RED;
            }
            else if (gState.globalBiomeMap.find(key) != gState.globalBiomeMap.end())
            {
                value = TextFormat("%i", gState.globalBiomeMap[key]);
            }

            DrawText(
                value.c_str(),
                x + 12 + col * cellWidth,
                y + 10 + row * cellHeight,
                18,
                color
            );
        }
    }
}

void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose())
    {
        gAppStatus = TERMINATED;
        return;
    }

    if (IsKeyPressed(KEY_G))
    {
        gState.godMode = !gState.godMode;
    }

    if (IsKeyPressed(KEY_O))
    {
        gState.aiEnabled = !gState.aiEnabled;
        saveGame();
    }

    switch (gGameMode)
    {
    case MODE_MENU:
        if (IsKeyPressed(KEY_ENTER))
        {
            gState.hearts = MAX_HEARTS;
            gState.hitCooldown = 0.0f;
            loadLevel(0);
            gPreviousTicks = (float)GetTime();
            gTimeAccumulator = 0.0f;
            gGameMode = MODE_INVENTORY;
        }
        break;

    case MODE_MAP:
    {
        gState.xochitl->resetMovement();

        if (IsKeyDown(KEY_A)){
        gState.xochitl->moveLeft();
        gLightDirection = { -1.0f, 0.0f };
        }
        else if (IsKeyDown(KEY_D)){
        gState.xochitl->moveRight();
        gLightDirection = { 1.0f, 0.0f };
        }

        if (IsKeyDown(KEY_W)){
        gState.xochitl->moveUp();
        gLightDirection = { 0.0f, -1.0f };
        }
        if (IsKeyDown(KEY_S)){
        gState.xochitl->moveDown();
        gLightDirection = { 0.0f, 1.0f };
        }

        //if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;

        if (GetLength(gState.xochitl->getMovement()) > 1.0f)
            gState.xochitl->normaliseMovement();
        /*
        if (IsKeyPressed(KEY_ONE) && gMapLibrary.size() > 0)   loadLevel(0);
        if (IsKeyPressed(KEY_TWO) && gMapLibrary.size() > 1)   loadLevel(1);
        if (IsKeyPressed(KEY_THREE) && gMapLibrary.size() > 2) loadLevel(2);
        */


        // inventory
        if (IsKeyPressed(KEY_I))
        {
            gGameMode = MODE_INVENTORY;
            break;
        }

        // combat
        /*
        if (IsKeyPressed(KEY_C))
        {
            gGameMode = MODE_COMBAT;
            break;
        }
        */

        // fly to test
        Vector2 debugPosition = gState.xochitl->getPosition(); // help debug test fly

        if (IsKeyDown(KEY_RIGHT)) debugPosition.x += DEBUG_FLY_SPEED;
        if (IsKeyDown(KEY_LEFT))  debugPosition.x -= DEBUG_FLY_SPEED;
        if (IsKeyDown(KEY_UP))    debugPosition.y -= DEBUG_FLY_SPEED;
        if (IsKeyDown(KEY_DOWN))  debugPosition.y += DEBUG_FLY_SPEED;

        gState.xochitl->setPosition(debugPosition);
        break;
    }

    case MODE_INVENTORY:
        // directions
        if (IsKeyPressed(KEY_W))
        {
            gState.travelDirection = TRAVEL_NORTH;
            gState.travelLog = "You turn north, watching the road and tree line for anything moving wrong.";
            gState.travelDebugLog = "";
            saveGame();
        }
        else if (IsKeyPressed(KEY_D))
        {
            gState.travelDirection = TRAVEL_EAST;
            gState.travelLog = "You turn east, following what remains of the road into the quiet distance.";
            gState.travelDebugLog = "";
            saveGame();
        }
        else if (IsKeyPressed(KEY_S))
        {
            gState.travelDirection = TRAVEL_SOUTH;
            gState.travelLog = "You turn south, keeping the sun and ruined signs as your only bearings.";
            gState.travelDebugLog = "";
            saveGame();
        }
        else if (IsKeyPressed(KEY_A))
        {
            gState.travelDirection = TRAVEL_WEST;
            gState.travelLog = "You turn west, back toward older roads and darker blocks of abandoned buildings.";
            gState.travelDebugLog = "";
            saveGame();
        }
        // travel interactions
        if (IsKeyPressed(KEY_L))
        {
            loadLootLevelForCurrentBiome();
            gPreviousTicks = (float)GetTime();
            gTimeAccumulator = 0.0f;
            gGameMode = MODE_MAP;
        }
        else if (IsKeyPressed(KEY_R))
        {
            gState.hour += 8;
            while (gState.hour >= 24)
            {
                gState.hour -= 24;
                gState.day += 1;
            }

            gState.energyStat = 100;

            if (gState.foodStat > 0)
            {
                gState.foodStat -= 15;
                if (gState.foodStat < 0) gState.foodStat = 0;

                gState.healthStat += 25;
                if (gState.healthStat > 100) gState.healthStat = 100;

                gState.travelLog = "You sleep through the worst hours, eat what you can spare, and wake up steadier than before.";
            }
            else
            {
                gState.travelLog = "You sleep for hours, but without food your body only gets weaker around the edges. The rest clears your head, not your wounds.";
            }
            generateTimedAIDataIfNeeded();
            // it will double save if we ai and save but i dont wanna fix
            saveGame();
        }
        else if (IsKeyPressed(KEY_T))
        {
            if (gState.energyStat < 8)
            {
                gState.travelLog = "You are too exhausted to keep travelling. You need real sleep before you can move safely again.";
            }
            else
            {
                gState.hour += 1;
                while (gState.hour >= 24)
                {
                    gState.hour -= 24;
                    gState.day += 1;
                }

                if (!moveTravelPosition())
                {
                    saveGame();
                    break;
                }

                gState.distanceTravelled += 1;
                updateCoordinateFromWorldGrid();

                gState.energyStat -= 8;
                if (gState.energyStat < 0) gState.energyStat = 0;

                gState.foodStat -= 3;
                if (gState.foodStat < 0) gState.foodStat = 0;

                gState.currentLocation = "Roadside Blocks";
                std::string biomeText = getTravelTextForBiome(gState.currentBiome);

                gState.currentLocation = getBiomeName(gState.currentBiome);

                gState.travelLog = biomeText;

                gState.travelDebugLog = TextFormat(
                    "You travel 1 km %s. Global cell [%i, %i], local km [%i, %i]. Biome: %s.",
                    getTravelDirectionName().c_str(),
                    gState.worldGridX,
                    gState.worldGridY,
                    gState.localKmX,
                    gState.localKmY,
                    getBiomeName(gState.currentBiome).c_str()
                );
            }
            generateTimedAIDataIfNeeded();
            saveGame();
        }
        break;

    case MODE_COMBAT:
        if (IsKeyPressed(KEY_ONE))
        {
            if (gState.currentEnemy != nullptr && gState.combatMeleeCooldown <= 0.0f && gState.energyStat > 0)
            {
                gState.energyStat -= 1;
                if (gState.energyStat < 0) gState.energyStat = 0;

                gState.combatEnemyHealth -= gState.meleeDamage * 4;
                PlaySound(gState.zombieHitSound);
                if (gState.combatEnemyHealth < 0) gState.combatEnemyHealth = 0;

                gState.combatMeleeCooldown = 1.2f / (float)gState.meleeSpeed;

                if (gState.combatEnemyHealth <= 0)
                {
                    gState.currentEnemy->deactivate();
                    gState.currentEnemy = nullptr;
                    gState.xochitl->setPosition(gState.combatStoredPlayerPosition);
                    gState.actionMessage = "Zombie killed";
                    gState.actionMessageTimer = 2.5f;
                    gGameMode = MODE_MAP;
                }
            }
        }
        else if (IsKeyPressed(KEY_TWO))
        {
            if (gState.currentEnemy != nullptr && (gState.ammo > 0 || gState.godMode) && gState.combatFirearmCooldown <= 0.0f && gState.energyStat > 0)
            {
                gState.energyStat -= 1;
                if (gState.energyStat < 0) gState.energyStat = 0;

                if (!gState.godMode)
                {
                    gState.ammo -= 1;
                }
                gState.combatEnemyHealth -= gState.firearmDamage * 5;
                PlaySound(gState.zombieHitSound);
                if (gState.combatEnemyHealth < 0) gState.combatEnemyHealth = 0;

                gState.combatFirearmCooldown = 1.2f / (float)gState.firearmSpeed;

                if (gState.combatEnemyHealth <= 0)
                {
                    gState.currentEnemy->deactivate();
                    gState.currentEnemy = nullptr;
                    gState.xochitl->setPosition(gState.combatStoredPlayerPosition);
                    gState.actionMessage = "Zombie killed";
                    gState.actionMessageTimer = 2.5f;
                    gGameMode = MODE_MAP;
                }
            }
        }
        else if (IsKeyPressed(KEY_THREE))
        {
            if (GetRandomValue(0, 99) < 60)
            {
                gState.currentEnemy = nullptr;
                gState.xochitl->setPosition(gState.combatStoredPlayerPosition);
                gGameMode = MODE_MAP;
            }
            else
            {
                if (!gState.godMode)
                {
                    gState.healthStat -= 8;
                    PlaySound(gState.playerHitSound);
                    if (gState.healthStat <= 0)
                    {
                        gGameMode = MODE_LOSE;
                    }
                }
                if (gState.healthStat <= 0)
                {
                    gGameMode = MODE_LOSE;
                }
            }
        }
        break;
        break;
    
    case MODE_GEAR_COMPARE:
        if (IsKeyPressed(KEY_Y))
        {
            if (gState.pendingGearType == "armor")
            {
                gState.armorName = gState.pendingGearName;
                gState.armorProtection = gState.pendingGearFirst;
                gState.armorMobility = gState.pendingGearSecond;
            }
            else if (gState.pendingGearType == "melee")
            {
                gState.meleeName = gState.pendingGearName;
                gState.meleeDamage = gState.pendingGearFirst;
                gState.meleeSpeed = gState.pendingGearSecond;
            }
            else if (gState.pendingGearType == "firearm")
            {
                gState.firearmName = gState.pendingGearName;
                gState.firearmDamage = gState.pendingGearFirst;
                gState.firearmSpeed = gState.pendingGearSecond;
            }
            else if (gState.pendingGearType == "flashlight")
            {
                gState.flashlightName = gState.pendingGearName;
                gState.flashlightRadius = gState.pendingGearFirst;
                gState.flashlightRange = gState.pendingGearSecond;
            }

            gState.actionMessage = TextFormat("Equipped %s", gState.pendingGearName.c_str());
            gState.actionMessageTimer = 2.5f;
            saveGame();
            gGameMode = MODE_MAP;
        }
        else if (IsKeyPressed(KEY_N))
        {
            gState.actionMessage = TextFormat("Kept current %s", gState.pendingGearType.c_str());
            gState.actionMessageTimer = 2.5f;
            saveGame();
            gGameMode = MODE_MAP;
        }
        break;

    case MODE_LOSE:
        if (IsKeyPressed(KEY_ENTER))
        {
            gGameMode = MODE_MENU;
        }
        break;
    }
}

void update()
{
    UpdateMusicStream(gState.bgm);
    if (gGameMode != MODE_MAP && gGameMode != MODE_COMBAT) return; //in menu or gear compare
    // Delta time
    float ticks = (float)GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    // Fixed timestep
    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {

        // blood effect cooldown
        if (gDamageTimer > 0.0f)
        {
            gDamageTimer -= FIXED_TIMESTEP;
            if (gDamageTimer < 0.0f) gDamageTimer = 0.0f;
        }

        // combat, kinda inspired by undertale

        if (gGameMode == MODE_COMBAT)
        {
            gState.combatTimer += FIXED_TIMESTEP;
            gState.combatAttackTimer += FIXED_TIMESTEP;

            // weapon cooldown to avoid spamming
            if (gState.combatMeleeCooldown > 0.0f) gState.combatMeleeCooldown -= FIXED_TIMESTEP;
            if (gState.combatFirearmCooldown > 0.0f) gState.combatFirearmCooldown -= FIXED_TIMESTEP;
            if (gState.combatMeleeCooldown < 0.0f) gState.combatMeleeCooldown = 0.0f;
            if (gState.combatFirearmCooldown < 0.0f) gState.combatFirearmCooldown = 0.0f;

            float moveSpeed = 220.0f;
            if (IsKeyDown(KEY_A)) gState.combatHeartPosition.x -= moveSpeed * FIXED_TIMESTEP;
            if (IsKeyDown(KEY_D)) gState.combatHeartPosition.x += moveSpeed * FIXED_TIMESTEP;
            if (IsKeyDown(KEY_W)) gState.combatHeartPosition.y -= moveSpeed * FIXED_TIMESTEP;
            if (IsKeyDown(KEY_S)) gState.combatHeartPosition.y += moveSpeed * FIXED_TIMESTEP;

            if (gState.combatHeartPosition.x < gState.combatBox.x) gState.combatHeartPosition.x = gState.combatBox.x;
            if (gState.combatHeartPosition.x > gState.combatBox.x + gState.combatBox.width) gState.combatHeartPosition.x = gState.combatBox.x + gState.combatBox.width;
            if (gState.combatHeartPosition.y < gState.combatBox.y) gState.combatHeartPosition.y = gState.combatBox.y;
            if (gState.combatHeartPosition.y > gState.combatBox.y + gState.combatBox.height) gState.combatHeartPosition.y = gState.combatBox.y + gState.combatBox.height;

            gState.xochitl->setPosition(gState.combatHeartPosition);

            if (gState.combatAttackTimer >= gState.combatAttackInterval && !gState.combatAttackLive && gState.combatTelegraphDuration <= 0.0f)
            {
                gState.combatAttackTimer = 0.0f;
                gState.combatAttackLive = false;

                gState.combatAttackType = GetRandomValue(0, 2);

                if (gState.combatAttackType == 0)
                {
                    gState.combatTelegraphDuration = 0.7f;
                    gState.combatAttackDuration = 0.45f;

                    int edge = GetRandomValue(0, 3);
                    if (edge <= 1)
                    {
                        float laneY = gState.combatBox.y + GetRandomValue(20, (int)gState.combatBox.height - 20);
                        gState.combatEnemyAttack = { gState.combatBox.x, laneY - 10.0f, gState.combatBox.width, 20.0f };
                    }
                    else
                    {
                        float laneX = gState.combatBox.x + GetRandomValue(20, (int)gState.combatBox.width - 20);
                        gState.combatEnemyAttack = { laneX - 10.0f, gState.combatBox.y, 20.0f, gState.combatBox.height };
                    }
                }
                else if (gState.combatAttackType == 1)
                {
                    gState.combatTelegraphDuration = 0.7f;
                    gState.combatAttackDuration = 0.8f;

                    int edgePick = GetRandomValue(0, 3);
                    if (edgePick == 0) {
                        gState.combatBallPosition = { gState.combatBox.x + 20.0f, gState.combatBox.y + (float)GetRandomValue(20, (int)gState.combatBox.height - 20) };
                    }
                    else if (edgePick == 1) {
                        gState.combatBallPosition = { gState.combatBox.x + gState.combatBox.width - 20.0f, gState.combatBox.y + (float)GetRandomValue(20, (int)gState.combatBox.height - 20) };
                    }
                    else if (edgePick == 2) {
                        gState.combatBallPosition = { gState.combatBox.x + (float)GetRandomValue(20, (int)gState.combatBox.width - 20), gState.combatBox.y + 20.0f };
                    }
                    else {
                        gState.combatBallPosition = { gState.combatBox.x + (float)GetRandomValue(20, (int)gState.combatBox.width - 20), gState.combatBox.y + gState.combatBox.height - 20.0f };
                    }

                    gState.combatBallStart = gState.combatBallPosition;
                    gState.combatBallTarget = gState.combatHeartPosition;
                }
                else
                {
                    gState.combatTelegraphDuration = 1.2f;
                    gState.combatAttackDuration = 1.5f;

                    int edgePick = GetRandomValue(0, 3);
                    if (edgePick == 0) {
                        gState.combatLerperPosition = { gState.combatBox.x + 20.0f, gState.combatBox.y + (float)GetRandomValue(20, (int)gState.combatBox.height - 20) };
                    }
                    else if (edgePick == 1) {
                        gState.combatLerperPosition = { gState.combatBox.x + gState.combatBox.width - 20.0f, gState.combatBox.y + (float)GetRandomValue(20, (int)gState.combatBox.height - 20) };
                    }
                    else if (edgePick == 2) {
                        gState.combatLerperPosition = { gState.combatBox.x + (float)GetRandomValue(20, (int)gState.combatBox.width - 20), gState.combatBox.y + 20.0f };
                    }
                    else {
                        gState.combatLerperPosition = { gState.combatBox.x + (float)GetRandomValue(20, (int)gState.combatBox.width - 20), gState.combatBox.y + gState.combatBox.height - 20.0f };
                    }

                    gState.currentEnemy->setPosition(gState.combatLerperPosition);
                    gState.currentEnemy->setAIType(LERPER);
                    gState.currentEnemy->setAIState(IDLE);
                    gState.currentEnemy->setLerpFactor(0.45f);
                }
            }

            if (gState.combatTelegraphDuration > 0.0f)
            {
                gState.combatTelegraphDuration -= FIXED_TIMESTEP;

                if (gState.combatTelegraphDuration <= 0.0f)
                {
                    gState.combatAttackLive = true;
                }
            }
            else if (gState.combatAttackLive && gState.combatAttackDuration > 0.0f)
            {
                gState.combatAttackDuration -= FIXED_TIMESTEP;

                if (gState.combatAttackType == 0)
                {
                    Rectangle heartRect = {
                        gState.combatHeartPosition.x - 8.0f,
                        gState.combatHeartPosition.y - 8.0f,
                        16.0f,
                        16.0f
                    };

                    if (CheckCollisionRecs(heartRect, gState.combatEnemyAttack))
                    {
                        if (!gState.godMode)
                        {
                            gState.healthStat -= gState.combatEnemyDamage;
                            gDamageTimer = 0.5f;
                            PlaySound(gState.playerHitSound);
                        }
                        gState.combatAttackDuration = 0.0f;
                        gState.combatAttackLive = false;
                    }
                }
                else if (gState.combatAttackType == 1)
                {
                    float t = 1.0f - (gState.combatAttackDuration / 0.8f);
                    if (t < 0.0f) t = 0.0f;
                    if (t > 1.0f) t = 1.0f;

                    gState.combatBallPosition.x = gState.combatBallStart.x + (gState.combatBallTarget.x - gState.combatBallStart.x) * t;
                    gState.combatBallPosition.y = gState.combatBallStart.y + (gState.combatBallTarget.y - gState.combatBallStart.y) * t;

                    float dx = gState.combatBallPosition.x - gState.combatHeartPosition.x;
                    float dy = gState.combatBallPosition.y - gState.combatHeartPosition.y;
                    float hitRadius = gState.combatBallRadius + 8.0f;
                    if (dx * dx + dy * dy <= hitRadius * hitRadius)
                    {
                        if (!gState.godMode)
                        {
                            gState.healthStat -= gState.combatEnemyDamage;
                            gDamageTimer = 0.5f;
                            PlaySound(gState.playerHitSound);
                        }
                        gState.combatAttackDuration = 0.0f;
                        gState.combatAttackLive = false;
                    }
                }
                else
                {
                    gState.currentEnemy->update(
                        FIXED_TIMESTEP,
                        gState.xochitl,
                        nullptr,
                        nullptr,
                        0,
                        nullptr,
                        0
                    );

                    gState.combatLerperPosition = gState.currentEnemy->getPosition();

                    float dx = gState.combatLerperPosition.x - gState.combatHeartPosition.x;
                    float dy = gState.combatLerperPosition.y - gState.combatHeartPosition.y;
                    float hitRadius = gState.combatLerperRadius + 8.0f;
                    if (dx * dx + dy * dy <= hitRadius * hitRadius)
                    {
                        if (!gState.godMode)
                        {
                            gState.healthStat -= gState.combatEnemyDamage;
                            gDamageTimer = 0.5f;
                            PlaySound(gState.playerHitSound);
                        }
                        gState.combatAttackDuration = 0.0f;
                        gState.combatAttackLive = false;
                    }
                }

                if (gState.healthStat <= 0)
                {
                    gGameMode = MODE_LOSE;
                    return;
                }

                if (gState.combatAttackDuration <= 0.0f)
                {
                    gState.combatAttackLive = false;
                }
            }

            deltaTime -= FIXED_TIMESTEP;
            continue;
        }


        if (gState.hitCooldown > 0) {
            gState.hitCooldown -= FIXED_TIMESTEP;
        }

        // restore health if gpd
        if (gState.godMode)
        {
            gState.healthStat = 100;
        }

        if (gState.actionMessageTimer > 0.0f)
        {
            gState.actionMessageTimer -= FIXED_TIMESTEP;
            if (gState.actionMessageTimer < 0.0f) gState.actionMessageTimer = 0.0f;
        }

        if (gState.exitLockTimer > 0.0f)
        {
            gState.exitLockTimer -= FIXED_TIMESTEP;
            if (gState.exitLockTimer < 0.0f) gState.exitLockTimer = 0.0f;
        }

        // only physic if not debug
        if (!isDebugFlying()) {
            gState.xochitl->update(
                FIXED_TIMESTEP, // delta time / fixed timestep
                nullptr,        // player
                gState.map,     // map
                nullptr,
                0,               // col. entity count
                nullptr,
                0
            );
        }

        // button as exits
        for (size_t i = 0; i < gState.buttons.size(); i++)
        {
            if (gState.xochitl->isColliding(gState.buttons[i]) && gState.exitLockTimer <= 0.0f)
            {
                gGameMode = MODE_INVENTORY;
                return;
            }
        }

        for (size_t i = 0; i < gState.lootButtons.size(); i++)
        {
            if (!gState.lootButtons[i]->isActive()) continue;

            if (gState.xochitl->isColliding(gState.lootButtons[i]))
            {
                // randomize loot (future feature can have loot tied to diff places)
                int lootRoll = gState.godMode ? 99 : GetRandomValue(0, 99); //debug loot god

                if (lootRoll < 55)
                {
                    int foodGain = GetRandomValue(8, 18);
                    gState.foodStat += foodGain;
                    if (gState.foodStat > 100) gState.foodStat = 100;

                    gState.actionMessage = TextFormat("Found food: +%i", foodGain);
                }
                else if (lootRoll < 82)
                {
                    int ammoGain = GetRandomValue(3, 8);
                    gState.ammo += ammoGain;

                    gState.actionMessage = TextFormat("Found ammo: +%i", ammoGain);
                }
                else
                {
                    int gearRoll = GetRandomValue(0, 99);

                    if (gearRoll < 30 && !gState.armorLootNames.empty())
                    {
                        int pick = GetRandomValue(0, (int)gState.armorLootNames.size() - 1);

                        gState.pendingGearType = "armor";
                        gState.pendingGearName = gState.armorLootNames[pick];
                        gState.pendingGearFirst = gState.armorLootProtection[pick];
                        gState.pendingGearSecond = gState.armorLootMobility[pick];
                        gGameMode = MODE_GEAR_COMPARE;

                        gState.actionMessage = TextFormat("Found armor: %s", gState.armorName.c_str());
                    }
                    else if (gearRoll < 60 && !gState.meleeLootNames.empty())
                    {
                        int pick = GetRandomValue(0, (int)gState.meleeLootNames.size() - 1);

                        gState.pendingGearType = "melee";
                        gState.pendingGearName = gState.meleeLootNames[pick];
                        gState.pendingGearFirst = gState.meleeLootDamage[pick];
                        gState.pendingGearSecond = gState.meleeLootSpeed[pick];
                        gGameMode = MODE_GEAR_COMPARE;

                        gState.actionMessage = TextFormat("Found melee: %s", gState.meleeName.c_str());
                    }
                    else if (gearRoll < 85 && !gState.firearmLootNames.empty())
                    {
                        int pick = GetRandomValue(0, (int)gState.firearmLootNames.size() - 1);

                        gState.pendingGearType = "firearm";
                        gState.pendingGearName = gState.firearmLootNames[pick];
                        gState.pendingGearFirst = gState.firearmLootDamage[pick];
                        gState.pendingGearSecond = gState.firearmLootSpeed[pick];
                        gGameMode = MODE_GEAR_COMPARE;

                        gState.actionMessage = TextFormat("Found firearm: %s", gState.firearmName.c_str());
                    }
                    else if (!gState.flashlightLootNames.empty())
                    {
                        int pick = GetRandomValue(0, (int)gState.flashlightLootNames.size() - 1);

                        gState.pendingGearType = "flashlight";
                        gState.pendingGearName = gState.flashlightLootNames[pick];
                        gState.pendingGearFirst = gState.flashlightLootRadius[pick];
                        gState.pendingGearSecond = gState.flashlightLootRange[pick];
                        gGameMode = MODE_GEAR_COMPARE;

                        gState.actionMessage = TextFormat("Found light: %s", gState.flashlightName.c_str());
                    }
                    else
                    {
                        int ammoGain = GetRandomValue(2, 5);
                        gState.ammo += ammoGain;
                        gState.actionMessage = TextFormat("Found ammo: +%i", ammoGain);
                    }
                }

                gState.actionMessageTimer = 2.5f;
                gState.lootButtons[i]->deactivate();
                saveGame();

                if (gGameMode == MODE_GEAR_COMPARE)
                {
                    return;
                }
            }
        }

        for (size_t i = 0; i < gState.ghosts.size(); i++) {
            if (!gState.ghosts[i]->isActive()) continue;

            gState.ghosts[i]->update(
                FIXED_TIMESTEP,
                gState.xochitl,
                gState.map,
                nullptr,
                0,
                nullptr,
                0
            );

            // combat trigger on collision

            if (gState.xochitl->isColliding(gState.ghosts[i])) {
                gState.currentEnemy = gState.ghosts[i];

                gState.combatStoredPlayerPosition = gState.xochitl->getPosition();
                gState.combatStoredEnemyPosition = gState.currentEnemy->getPosition();

                gState.combatHeartPosition = {
                    gState.combatBox.x + gState.combatBox.width / 2.0f,
                    gState.combatBox.y + gState.combatBox.height / 2.0f
                };
                gState.combatHeartVelocity = { 0.0f, 0.0f };

                gState.combatTimer = 0.0f;
                gState.combatAttackTimer = 0.0f;
                gState.combatAttackDuration = 0.0f;
                gState.combatTelegraphDuration = 0.0f;
                gState.combatAttackLive = false;
                gState.combatAttackType = 0;

                gState.combatEnemyAttack = { 0.0f, 0.0f, 0.0f, 0.0f };

                gState.combatBallPosition = { 0.0f, 0.0f };
                gState.combatBallStart = { 0.0f, 0.0f };
                gState.combatBallTarget = { 0.0f, 0.0f };
                gState.combatBallRadius = 12.0f;
                gState.combatMeleeCooldown = 0.0f;
                gState.combatFirearmCooldown = 0.0f;

                int edgePick = GetRandomValue(0, 3);
                if (edgePick == 0) {
                    gState.combatLerperPosition = { gState.combatBox.x + 20.0f, gState.combatBox.y + (float)GetRandomValue(20, (int)gState.combatBox.height - 20) };
                }
                else if (edgePick == 1) {
                    gState.combatLerperPosition = { gState.combatBox.x + gState.combatBox.width - 20.0f, gState.combatBox.y + (float)GetRandomValue(20, (int)gState.combatBox.height - 20) };
                }
                else if (edgePick == 2) {
                    gState.combatLerperPosition = { gState.combatBox.x + (float)GetRandomValue(20, (int)gState.combatBox.width - 20), gState.combatBox.y + 20.0f };
                }
                else {
                    gState.combatLerperPosition = { gState.combatBox.x + (float)GetRandomValue(20, (int)gState.combatBox.width - 20), gState.combatBox.y + gState.combatBox.height - 20.0f };
                }

                gState.combatLerperStart = gState.combatLerperPosition;
                gState.combatLerperTarget = gState.combatHeartPosition;
                gState.combatLerperRadius = 14.0f;

                gState.combatEnemySpeed = gState.currentEnemy->getSpeed();
                gState.combatEnemyDamage = 10;
                if (gState.combatEnemySpeed < 80.0f) gState.combatEnemyDamage = 8;
                else if (gState.combatEnemySpeed < 140.0f) gState.combatEnemyDamage = 12;
                else gState.combatEnemyDamage = 16;

                gState.combatEnemyMaxHealth = 18 + (int)(gState.combatEnemySpeed / 10.0f);
                gState.combatEnemyHealth = gState.combatEnemyMaxHealth;

                gState.combatAttackInterval = 1.6f - (gState.combatEnemySpeed / 350.0f);
                if (gState.combatAttackInterval < 0.6f) gState.combatAttackInterval = 0.6f;

                gGameMode = MODE_COMBAT;
                return;
            }
        }

        deltaTime -= FIXED_TIMESTEP;

        Vector2 currentPlayerPosition = gState.xochitl->getPosition();

        panCamera(&gState.camera, &currentPlayerPosition);

        /*
        // if we want to button end
        if (!gState.button->isActive()) {
            advanceLevelOrQuit();
            return;
        }
        */
    }

    gTimeAccumulator = deltaTime;
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    if (gGameMode == MODE_MAP) {
        // borrow from shaders
        BeginMode2D(gState.camera);

        gLightPosition = gState.xochitl->getPosition();

        float lightAngle = (float)gState.flashlightRadius;
        float lightRange = 140.0f + (float)gState.flashlightRange * 70.0f;

        gLightShader.begin();

        gLightShader.setVector2("lightPosition", gLightPosition);
        gLightShader.setVector2("lightDirection", gLightDirection);
        gLightShader.setFloat("lightAngle", lightAngle);
        gLightShader.setFloat("lightRange", lightRange);
        gLightShader.setFloat("isCharging", 1.0f);

        // support more lightsources
        int lanternCount = (int)gState.lanterns.size();
        if (lanternCount > MAX_MAP_LANTERNS) lanternCount = MAX_MAP_LANTERNS;

        gLightShader.setInt("lanternCount", lanternCount);

        for (int i = 0; i < lanternCount; i++)
        {
            Vector2 lanternPosition = gState.lanterns[i]->getPosition();
            gLightShader.setVector2(TextFormat("lanternPositions[%i]", i), lanternPosition);
        }

        float mapWidth = gState.currentWidth * TILE_DIMENSION;
        float mapHeight = gState.currentHeight * TILE_DIMENSION;
        float backgroundX = ORIGIN.x - mapWidth / 2.0f;
        float backgroundY = ORIGIN.y - mapHeight / 2.0f;

        DrawTexturePro(
            gState.mapBackground,
            { 0.0f, 0.0f, (float)gState.mapBackground.width, (float)gState.mapBackground.height },
            { backgroundX, backgroundY, mapWidth, mapHeight },
            { 0.0f, 0.0f },
            0.0f,
            WHITE
        );

        // map is too dark lol
        /*
        DrawRectangle(
            (int)backgroundX,
            (int)backgroundY,
            (int)mapWidth,
            (int)mapHeight,
            Fade(WHITE, 0.08f)
        );
        */

        gState.map->render();

        for (size_t i = 0; i < gState.buttons.size(); i++)
        {
            gState.buttons[i]->render();
        }
        /*
        for (size_t i = 0; i < gState.lanterns.size(); i++)
        {
            gState.lanterns[i]->render();
        }
        */

        for (size_t i = 0; i < gState.lootButtons.size(); i++)
        {
            if (gState.lootButtons[i]->isActive())
            {
                gState.lootButtons[i]->render();
            }
        }

        for (size_t i = 0; i < gState.ghosts.size(); i++) {
            if (gState.ghosts[i]->isActive()) {
                gState.ghosts[i]->render();
            }
        }

        gState.xochitl->render();

        gLightShader.end();

        EndMode2D();

        // DEBUG
        DrawText(getMapLocationName().c_str(), 20, 20, 30, WHITE);

        DrawText(TextFormat("Health: %i", gState.healthStat), 20, 90, 30, RED);

        if (gState.actionMessageTimer > 0.0f)
        {
            DrawRectangle(20, 130, 420, 45, Fade(BLACK, 0.55f));
            DrawText(gState.actionMessage.c_str(), 35, 140, 24, WHITE);
        }
    }

    else if (gGameMode == MODE_COMBAT)
    {
        if (gState.godMode)
        {
            gState.healthStat = 100;
        }
        DrawText("COMBAT", SCREEN_WIDTH / 2 - 70, 40, 40, WHITE);

        DrawRectangleRec(gState.combatBox, Fade(BLACK, 0.6f));
        DrawRectangleLinesEx(gState.combatBox, 3.0f, WHITE);

        // preview attack hints
        if (gState.combatAttackType == 0)
        {
            if (gState.combatTelegraphDuration > 0.0f)
            {
                float alpha = 1.0f - (gState.combatTelegraphDuration / 0.7f);
                if (alpha < 0.0f) alpha = 0.0f;
                if (alpha > 1.0f) alpha = 1.0f;

                float flash = ((int)(gState.combatTelegraphDuration * 18.0f) % 2 == 0) ? 1.0f : 0.45f;
                DrawRectangleRec(gState.combatEnemyAttack, Fade(RED, alpha * flash * 0.8f));
            }
            else if (gState.combatAttackLive && gState.combatAttackDuration > 0.0f)
            {
                DrawRectangleRec(gState.combatEnemyAttack, RED);
            }
        }
        else if (gState.combatAttackType == 1)
        {
            if (gState.combatTelegraphDuration > 0.0f)
            {
                float alpha = 1.0f - (gState.combatTelegraphDuration / 0.7f);
                if (alpha < 0.0f) alpha = 0.0f;
                if (alpha > 1.0f) alpha = 1.0f;

                float flash = ((int)(gState.combatTelegraphDuration * 18.0f) % 2 == 0) ? 1.0f : 0.45f;
                DrawCircle((int)gState.combatBallPosition.x, (int)gState.combatBallPosition.y, gState.combatBallRadius, Fade(RED, alpha * flash * 0.5f));
                DrawCircle((int)gState.combatHeartPosition.x, (int)gState.combatHeartPosition.y, gState.combatBallRadius, Fade(RED, alpha * flash * 0.8f));
                DrawLineEx(gState.combatBallPosition, gState.combatHeartPosition, 2.0f, Fade(RED, alpha * flash * 0.6f));
            }
            else if (gState.combatAttackLive && gState.combatAttackDuration > 0.0f)
            {
                DrawCircle((int)gState.combatBallPosition.x, (int)gState.combatBallPosition.y, gState.combatBallRadius, RED);
            }
        }
        else if (gState.combatAttackType == 2)
        {
            DrawCircle((int)gState.combatLerperPosition.x, (int)gState.combatLerperPosition.y, gState.combatLerperRadius, Fade(RED, 0.35f));

            if (gState.combatTelegraphDuration > 0.0f)
            {
                float alpha = 1.0f - (gState.combatTelegraphDuration / 1.2f);
                if (alpha < 0.0f) alpha = 0.0f;
                if (alpha > 1.0f) alpha = 1.0f;

                float flash = ((int)(gState.combatTelegraphDuration * 18.0f) % 2 == 0) ? 1.0f : 0.45f;
                DrawCircle((int)gState.combatLerperPosition.x, (int)gState.combatLerperPosition.y, gState.combatLerperRadius, Fade(RED, alpha * flash * 0.5f));
                DrawCircle((int)gState.combatHeartPosition.x, (int)gState.combatHeartPosition.y, gState.combatLerperRadius, Fade(RED, alpha * flash * 0.8f));
            }

            if (gState.combatAttackLive && gState.combatAttackDuration > 0.0f)
            {
                DrawCircle((int)gState.combatLerperPosition.x, (int)gState.combatLerperPosition.y, gState.combatLerperRadius, RED);
            }
        }

        DrawCircle((int)gState.combatHeartPosition.x, (int)gState.combatHeartPosition.y, 8.0f, WHITE);

        // enemy healthbar
        DrawText(TextFormat("Enemy HP: %i / %i", gState.combatEnemyHealth, gState.combatEnemyMaxHealth), 700, 90, 22, WHITE);
        DrawRectangle(700, 120, 220, 18, Fade(WHITE, 0.2f));
        if (gState.combatEnemyMaxHealth > 0)
        {
            float hpRatio = (float)gState.combatEnemyHealth / (float)gState.combatEnemyMaxHealth;
            if (hpRatio < 0.0f) hpRatio = 0.0f;
            if (hpRatio > 1.0f) hpRatio = 1.0f;
            DrawRectangle(700, 120, (int)(220.0f * hpRatio), 18, RED);
        }



        // cool down

        DrawText(TextFormat("Health: %i", gState.healthStat), 80, 470, 24, WHITE);
        DrawText(TextFormat("Energy: %i", gState.energyStat), 80, 500, 24, WHITE);
        DrawText(TextFormat("Ammo: %i", gState.ammo), 80, 530, 24, WHITE);

        Rectangle meleeBox = { 690.0f, 462.0f, 250.0f, 30.0f };
        Rectangle firearmBox = { 690.0f, 492.0f, 250.0f, 30.0f };
        Rectangle fleeBox = { 690.0f, 522.0f, 250.0f, 30.0f };

        DrawRectangleRec(meleeBox, Fade(BLACK, 0.25f));
        DrawRectangleRec(firearmBox, Fade(BLACK, 0.25f));
        DrawRectangleRec(fleeBox, Fade(BLACK, 0.25f));

        DrawText(TextFormat("1 - Melee (%i dmg)", gState.meleeDamage * 4), 700, 470, 24, WHITE);

        if (gState.ammo > 0)
        {
            DrawText(TextFormat("2 - Firearm (%i dmg, -1 bullet)", gState.firearmDamage * 5), 700, 500, 24, WHITE);
        }
        else
        {
            DrawText("2 - Firearm (NO BULLETS)", 700, 500, 24, GRAY);
        }

        DrawText("3 - Flee", 700, 530, 24, WHITE);

        {
            float meleeMax = 1.2f / (float)gState.meleeSpeed;
            float meleeRatio = 0.0f;
            if (meleeMax > 0.0f) meleeRatio = gState.combatMeleeCooldown / meleeMax;
            if (meleeRatio < 0.0f) meleeRatio = 0.0f;
            if (meleeRatio > 1.0f) meleeRatio = 1.0f;

            if (meleeRatio > 0.0f)
            {
                DrawRectangle(
                    (int)meleeBox.x,
                    (int)meleeBox.y,
                    (int)(meleeBox.width * meleeRatio),
                    (int)meleeBox.height,
                    Fade(RED, 0.35f)
                );
            }
        }

        {
            float firearmMax = 1.2f / (float)gState.firearmSpeed;
            float firearmRatio = 0.0f;
            if (firearmMax > 0.0f) firearmRatio = gState.combatFirearmCooldown / firearmMax;
            if (firearmRatio < 0.0f) firearmRatio = 0.0f;
            if (firearmRatio > 1.0f) firearmRatio = 1.0f;

            if (firearmRatio > 0.0f || gState.ammo <= 0)
            {
                float shownRatio = firearmRatio;
                if (gState.ammo <= 0 && shownRatio < 1.0f) shownRatio = 1.0f;

                DrawRectangle(
                    (int)firearmBox.x,
                    (int)firearmBox.y,
                    (int)(firearmBox.width * shownRatio),
                    (int)firearmBox.height,
                    Fade(RED, 0.35f)
                );
            }
        }

        DrawText("1 - Melee", 700, 470, 24, WHITE);
        DrawText("2 - Firearm", 700, 500, 24, WHITE);
        DrawText("3 - Flee", 700, 530, 24, WHITE);
    }

    //INVENTORY BUT TECHNICALLY INTERFACE
    else if (gGameMode == MODE_INVENTORY)
    {
        DrawRectangle(20, 20, 220, 145, Fade(BLACK, 0.5f));
        DrawText(TextFormat("Health: %i", gState.healthStat), 35, 35, 24, WHITE);
        DrawText(TextFormat("Food: %i", gState.foodStat), 35, 65, 24, WHITE);
        DrawText(TextFormat("Energy: %i", gState.energyStat), 35, 95, 24, WHITE);

        DrawText(TextFormat("AI: %s", gState.aiEnabled ? "ON" : "OFF"), 35, 125, 18, gState.aiEnabled ? GREEN : RED);

        DrawRectangle(250, 40, 500, 300, Fade(BLACK, 0.5f));
        DrawText(gState.currentLocation.c_str(), 270, 60, 30, WHITE);
        DrawText(TextFormat("Day %i  %02i:%02i", gState.day, gState.hour, gState.minute), 270, 100, 24, GRAY);
        DrawText(TextFormat("Distance: %i km", gState.distanceTravelled), 270, 130, 24, GRAY);
        DrawText(TextFormat("Direction: %s", getTravelDirectionName().c_str()), 270, 155, 20, GRAY);

        DrawText(TextFormat("Grid: %i,%i  Local: %i,%i", gState.worldGridX, gState.worldGridY, gState.localKmX, gState.localKmY), 270, 180, 20, GRAY);
        DrawText(TextFormat("Coord: %.4f, %.4f", gState.latitude, gState.longitude), 270, 205, 18, GRAY);

        drawLocalBiomeDebugGrid(620, 145);

        drawWrappedText(gState.travelLog, 270, 230, 340, 22, 6, WHITE);

        DrawRectangle(20, 460, 220, 95, Fade(BLACK, 0.5f));
        drawWrappedText(gState.travelDebugLog, 35, 475, 190, 18, 4, GRAY);

        DrawRectangle(770, 140, 210, 300, Fade(BLACK, 0.5f));
        DrawText("Inventory", 790, 160, 28, WHITE);

        DrawText(TextFormat("Armor: %s", gState.armorName.c_str()), 790, 200, 20, WHITE);
        DrawText(TextFormat("Protection: %i  Mobility: %i", gState.armorProtection, gState.armorMobility), 790, 222, 16, GRAY);

        DrawText(TextFormat("Flashlight: %s", gState.flashlightName.c_str()), 790, 252, 20, WHITE);
        DrawText(TextFormat("Angle: %i  Range: %i", gState.flashlightRadius, gState.flashlightRange), 790, 274, 16, GRAY);

        DrawText(TextFormat("Firearm: %s", gState.firearmName.c_str()), 790, 304, 20, WHITE);
        DrawText(TextFormat("Damage: %i  Speed: %i", gState.firearmDamage, gState.firearmSpeed), 790, 326, 16, GRAY);

        DrawText(TextFormat("Melee: %s", gState.meleeName.c_str()), 790, 356, 20, WHITE);
        DrawText(TextFormat("Damage: %i  Speed: %i", gState.meleeDamage, gState.meleeSpeed), 790, 378, 16, GRAY);

        DrawText(TextFormat("Ammo: %i", gState.ammo), 790, 408, 20, WHITE);
        // DrawText(TextFormat("Supplies: %i", gState.supplies), 790, 432, 20, WHITE);

        DrawRectangle(760, 500, 220, 80, Fade(BLACK, 0.5f));
        DrawText("L - Loot", 780, 505, 20, WHITE);
        DrawText("T - Travel", 780, 530, 20, WHITE);
        DrawText("R - Rest", 880, 505, 20, WHITE);
        DrawText("WASD - Direction  O - AI", 780, 555, 18, GRAY);
    }

    // compare stats to decide keep or lose
    else if (gGameMode == MODE_GEAR_COMPARE)
    {
        DrawRectangle(180, 90, 640, 420, Fade(BLACK, 0.85f));
        DrawRectangleLines(180, 90, 640, 420, WHITE);

        DrawText("Found Gear", 390, 120, 38, WHITE);
        DrawText(gState.pendingGearName.c_str(), 390, 165, 28, WHITE);

        std::string currentName = "";
        int currentFirst = 0;
        int currentSecond = 0;
        const char* firstLabel = "";
        const char* secondLabel = "";

        if (gState.pendingGearType == "armor")
        {
            currentName = gState.armorName;
            currentFirst = gState.armorProtection;
            currentSecond = gState.armorMobility;
            firstLabel = "Protection";
            secondLabel = "Mobility";
        }
        else if (gState.pendingGearType == "melee")
        {
            currentName = gState.meleeName;
            currentFirst = gState.meleeDamage;
            currentSecond = gState.meleeSpeed;
            firstLabel = "Damage";
            secondLabel = "Speed";
        }
        else if (gState.pendingGearType == "firearm")
        {
            currentName = gState.firearmName;
            currentFirst = gState.firearmDamage;
            currentSecond = gState.firearmSpeed;
            firstLabel = "Damage";
            secondLabel = "Speed";
        }
        else if (gState.pendingGearType == "flashlight")
        {
            currentName = gState.flashlightName;
            currentFirst = gState.flashlightRadius;
            currentSecond = gState.flashlightRange;
            firstLabel = "Angle";
            secondLabel = "Range";
        }

        DrawText("Current", 250, 225, 26, GRAY);
        DrawText(currentName.c_str(), 250, 260, 22, WHITE);
        DrawText(TextFormat("%s: %i", firstLabel, currentFirst), 250, 300, 22, WHITE);
        DrawText(TextFormat("%s: %i", secondLabel, currentSecond), 250, 330, 22, WHITE);

        DrawText("New", 560, 225, 26, GRAY);
        DrawText(gState.pendingGearName.c_str(), 560, 260, 22, WHITE);

        Color firstColor = WHITE;
        Color secondColor = WHITE;

        if (gState.pendingGearFirst > currentFirst) firstColor = GREEN;
        else if (gState.pendingGearFirst < currentFirst) firstColor = RED;

        if (gState.pendingGearSecond > currentSecond) secondColor = GREEN;
        else if (gState.pendingGearSecond < currentSecond) secondColor = RED;

        DrawText(TextFormat("%s: %i", firstLabel, gState.pendingGearFirst), 560, 300, 22, firstColor);
        DrawText(TextFormat("%s: %i", secondLabel, gState.pendingGearSecond), 560, 330, 22, secondColor);

        DrawText("Swap gear?", 405, 395, 28, WHITE);
        DrawText("Y - Swap", 330, 440, 24, GREEN);
        DrawText("N - Keep Current", 500, 440, 24, RED);
        }

    else {
        //yeeted from lunar lander
        int titleSize = 60;
        const char* result;


        //if (gGameMode == MODE_WIN) result = "YOU WIN!";
        if (gGameMode == MODE_LOSE) result = "YOU LOSE";
        else if (gGameMode == MODE_INVENTORY) result = "INVENTORY";
        else if (gGameMode == MODE_COMBAT) result = "COMBAT";
        else result = "RISE OF THE AI:";
        // "A BRAINROT PLATFORMER"

        DrawText(result, SCREEN_WIDTH / 2 - MeasureText(result, 60) / 2, SCREEN_HEIGHT / 2 - 50, 60, WHITE);
        DrawText("Press ENTER to Continue", SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 + 30, 20, GRAY);
    }

    if (gDamageTimer > 0.0f)
    {
        float damageAmount = gDamageTimer / 0.5f;
        if (damageAmount < 0.0f) damageAmount = 0.0f;
        if (damageAmount > 1.0f) damageAmount = 1.0f;

        damageAmount *= 0.55f;

        gDamageShader.begin();
        gDamageShader.setFloat("damageAmount", damageAmount);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
        gDamageShader.end();
    }

    EndDrawing();
}

void shutdown() 
{
    delete gState.xochitl;
    for (size_t i = 0; i < gState.ghosts.size(); i++) {
        delete gState.ghosts[i];
    }
    for (size_t i = 0; i < gState.buttons.size(); i++)
    {
        delete gState.buttons[i];
    }
    for (size_t i = 0; i < gState.lootButtons.size(); i++)
    {
        delete gState.lootButtons[i];
    }
    for (size_t i = 0; i < gState.lanterns.size(); i++)
    {
        delete gState.lanterns[i];
    }
    delete gState.map;

    UnloadMusicStream(gState.bgm);
    UnloadSound(gState.jumpSound);
    UnloadSound(gState.playerHitSound);
    UnloadSound(gState.zombieHitSound);
    gLightShader.unload();
    gDamageShader.unload();

    UnloadTexture(gState.mapBackground);
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