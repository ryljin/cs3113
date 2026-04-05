//I used the solutions for excercise 2 as a base to structure my code
/**

* Author: [Ryan Jin]

* Assignment: Pong Clone

* Date due: [02/14/2026]

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/
#include "raylib.h"
#include <math.h>

// Enums
enum AppStatus { TERMINATED, RUNNING };

// Global Constants
constexpr int SCREEN_WIDTH = 800 * 1.5f,
SCREEN_HEIGHT = 450 * 1.5f,
FPS = 60;

constexpr float FP_DAY = 1.0f, // cbange this to make faster or slower
DP_YEAR = 365.0f,
SUN_ROTATE = (2 * PI) / 25.0f, // roughly 25 days per rotation
SUN_PULSE = (2 * PI) / 10.0f,
SUN_PULSE_AMPLITUDE = 0.05f,
EARTH_ORBIT = (2 * PI) / DP_YEAR,
EARTH_ROTATE = 2 * PI,
EARTH_ORBIT_RADIUS = 220.0f,
MOON_ORBIT_RADIUS = 60.0f, //icons are too big for scale
MOON_ORBIT_ROTATE = (2 * PI) / 27; // from tidal locking

constexpr char EARTH[] = "assets/earth.png";
constexpr char MOON[] = "assets/moon.png";
constexpr char SUN[] = "assets/sun.png";

constexpr char SPACE[] = "#0A1022"; // "space color"

constexpr Vector2 ORIGIN = {
    SCREEN_WIDTH / 2,
    SCREEN_HEIGHT / 2
};

// Global Variables
AppStatus gAppStatus = RUNNING;

float gAngle = 0.0f,
gPulseTime = 0.0f;

// New Space variables

float gDay = 0.0f;

float gSunRotate = 0.0f;
float gSunPulse = 0.0f;

float gEarthOrbit = 0.0f;
float gEarthRotate = 0.0f;

float gMoonOrbitRotate = 0.0f;

Vector2 gSunPos = ORIGIN;
Vector2 gEarthPos;
Vector2 gMoonPos;

Texture2D gSunTexture;
Texture2D gEarthTexture;
Texture2D gMoonTexture;

// Function Declarations
Color ColorFromHex(const char* hex);
void initialise();
void processInput();
void update();
void render();
void shutdown();

#include <stdio.h>
#include <stdlib.h>


// You can ignore this function, it's just to get nice colours :)
Color ColorFromHex(const char* hex)
{
    // Skip leading '#', if present
    if (hex[0] == '#') hex++;

    // Default alpha = 255 (opaque)
    unsigned int r = 0,
        g = 0,
        b = 0,
        a = 255;

    // 6‑digit form: RRGGBB
    if (sscanf(hex, "%02x%02x%02x", &r, &g, &b) == 3) {
        return (Color) {
            (unsigned char)r,
                (unsigned char)g,
                (unsigned char)b,
                (unsigned char)a
        };
    }

    // 8‑digit form: RRGGBBAA
    if (sscanf(hex, "%02x%02x%02x%02x", &r, &g, &b, &a) == 4) {
        return (Color) {
            (unsigned char)r,
                (unsigned char)g,
                (unsigned char)b,
                (unsigned char)a
        };
    }

    // Fallback – return white so you notice something went wrong
    return RAYWHITE;
}


void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Earth Moon Sun Model");

    gSunTexture = LoadTexture(SUN);
    gMoonTexture = LoadTexture(MOON);
    gEarthTexture = LoadTexture(EARTH);

    SetTargetFPS(FPS);
}

void processInput()
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update()
{
    /*
    gPulseTime += PULSE_INCREMENT;
    gScaleFactor = BASE_SIZE + MAX_AMPLITUDE * sin(gPulseTime / PULSE_SPEED);

    gAngle += ORBIT_SPEED;
    gPosition.x = ORIGIN.x + RADIUS * cos(gAngle);
    gPosition.y = ORIGIN.y + RADIUS * sin(gAngle);
    */

    float deltaDay = 1.0f / FP_DAY; 
    gDay += deltaDay;

    // sun
    gSunRotate += SUN_ROTATE * deltaDay;
    gSunPulse += SUN_PULSE * deltaDay;

    float sunScale = 1.0f + SUN_PULSE_AMPLITUDE * sin(gSunPulse);

    // earth
    gEarthOrbit += EARTH_ORBIT * deltaDay;
    gEarthRotate += EARTH_ROTATE * deltaDay;

    gEarthPos.x = gSunPos.x + EARTH_ORBIT_RADIUS * cos(gEarthOrbit);
    gEarthPos.y = gSunPos.y + EARTH_ORBIT_RADIUS * sin(gEarthOrbit);

    // moon
    gMoonOrbitRotate += MOON_ORBIT_ROTATE * deltaDay;

    gMoonPos.x = gEarthPos.x + MOON_ORBIT_RADIUS * cos(gMoonOrbitRotate);
    gMoonPos.y = gEarthPos.y + MOON_ORBIT_RADIUS * sin(gMoonOrbitRotate);

}

void render()
{
    BeginDrawing();

    float spacePulse = (sin(gSunPulse) + 1.0f) / 2.0f; //normalize to 0-1
    float brightness = 0.85f + 0.15f * spacePulse;

    Color base = ColorFromHex(SPACE);

    Color space_color = {
    (unsigned char)(base.r * brightness),
    (unsigned char)(base.g * brightness),
    (unsigned char)(base.b * brightness),
    255
    };

    ClearBackground(space_color); //https://www.color-hex.com/color-palette/30647

    //

    float sunScale = 1.0f + 0.05f * sin(gSunPulse);

    Rectangle sunSource = {
        0,
        0,
        (float)gSunTexture.width,
        (float)gSunTexture.height
    };

    Rectangle earthSource = {
        0,
        0,
        (float)gEarthTexture.width,
        (float)gEarthTexture.height
    };

    Rectangle moonSource = {
        0,
        0,
        (float)gMoonTexture.width,
        (float)gMoonTexture.height
    };

    Rectangle sunDest = {
        gSunPos.x,
        gSunPos.y,
        gSunTexture.width * sunScale * 0.4f,
        gSunTexture.height * sunScale * 0.4f
    };

    Rectangle earthDest = {
        gEarthPos.x,
        gEarthPos.y,
        gEarthTexture.width * 0.15f,
        gEarthTexture.height * 0.15f
    };

    Rectangle moonDest = {
        gMoonPos.x,
        gMoonPos.y,
        gMoonTexture.width * 0.08f,
        gMoonTexture.height * 0.08f
    };

    Vector2 moonOrigin = {
        moonDest.width / 2,
        moonDest.height / 2
    };

    Vector2 sunOrigin = {
        sunDest.width / 2,
        sunDest.height / 2
    };

    Vector2 earthOrigin = {
        earthDest.width / 2,
        earthDest.height / 2
    };

    DrawTexturePro(gSunTexture, sunSource, sunDest, sunOrigin, gSunRotate * RAD2DEG, WHITE);

    DrawTexturePro(gEarthTexture, earthSource, earthDest, earthOrigin, gEarthRotate * RAD2DEG, WHITE);

    DrawTexturePro(gMoonTexture, moonSource, moonDest, moonOrigin, gMoonOrbitRotate * RAD2DEG, WHITE);

    EndDrawing();
}

void shutdown()
{
    // small fix to unload
    UnloadTexture(gSunTexture);
    UnloadTexture(gEarthTexture);
    UnloadTexture(gMoonTexture);

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