#ifndef LUNARIS_H
#define LUNARIS_H

#include <stdbool.h>

typedef struct Vec2 {
    float x, y;
} Vec2;

typedef struct Color {
    float r, g, b, a;
} Color;

static const Color RED   = {1, 0, 0, 1};
static const Color GREEN = {0, 1, 0, 1};
static const Color BLUE  = {0, 0, 1, 1};
static const Color WHITE = {1, 1, 1, 1};
static const Color BLACK = {0, 0, 0, 1};

bool InitWindow(int width, int height, const char* title);

bool WindowShouldClose();

void BeginDrawing();

void EndDrawing();

void CloseWindow();

int GetScreenWidth();

int GetScreenHeight();

void SetTargetFPS(int fps);

float GetFrameTime();

float GetTime();

void DrawTriangle(Vec2 center, float radius, float rotation, Color color);

void DrawRectangle(Vec2 position, Vec2 size, Color color);

void DrawCircle(Vec2 center, float radius, Color color);

#endif
