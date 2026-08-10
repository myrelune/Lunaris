#ifndef LUNARIS_H
#define LUNARIS_H

#include <stdbool.h>

/**
 * @brief Represents a 2D vector with X and Y coordinates.
 */
typedef struct Vec2 {
    float x, y;
} Vec2;

/**
 * @brief Represents an RGBA color value.
 */
typedef struct Color {
    float r, g, b, a;
} Color;

/** @brief Predefined Red color */
static const Color RED   = {1, 0, 0, 1};
/** @brief Predefined Green color */
static const Color GREEN = {0, 1, 0, 1};
/** @brief Predefined Blue color */
static const Color BLUE  = {0, 0, 1, 1};
/** @brief Predefined White color */
static const Color WHITE = {1, 1, 1, 1};
/** @brief Predefined Black color */
static const Color BLACK = {0, 0, 0, 1};

/**
 * @brief Initializes the GLFW window and OpenGL context.
 *
 * @param width Width of the window in pixels.
 * @param height Height of the window in pixels.
 * @param title Title displayed on the window border.
 * @return true If initialization was successful.
 * @return false If initialization failed.
 */
bool InitWindow(int width, int height, const char* title);

/**
 * @brief Checks if the window has been signaled to close.
 *
 * @return true If the close flag is set.
 * @return false Otherwise.
 */
bool WindowShouldClose();

/**
 * @brief Prepares the frame for rendering. Must be called at the start of every frame.
 */
void BeginDrawing();

/**
 * @brief Finalizes the frame rendering, flushes batches, and swaps buffers.
 */
void EndDrawing();

/**
 * @brief Closes the window and frees all allocated engine resources.
 */
void CloseWindow();

/**
 * @brief Gets the current width of the screen/window in pixels.
 *
 * @return int Screen width.
 */
int GetScreenWidth();

/**
 * @brief Gets the current height of the screen/window in pixels.
 *
 * @return int Screen height.
 */
int GetScreenHeight();

/**
 * @brief Sets the target frames per second (FPS) cap for the engine.
 *
 * @param fps Target FPS (0 for uncapped).
 */
void SetTargetFPS(int fps);

/**
 * @brief Gets the time elapsed in seconds since the last frame (Delta Time).
 *
 * @return float Frame time in seconds.
 */
float GetFrameTime();

/**
 * @brief Gets the total elapsed time in seconds since GLFW initialization.
 *
 * @return float Total time in seconds.
 */
float GetTime();

/**
 * @brief Draws a 2D triangle.
 *
 * @param center Center position of the triangle.
 * @param radius Radius from the center to the vertices.
 * @param rotation Rotation angle in radians.
 * @param color Color of the triangle.
 */
void DrawTriangle(Vec2 center, float radius, float rotation, Color color);

/**
 * @brief Draws a 2D rectangle.
 *
 * @param position Center position of the rectangle.
 * @param size Width and height of the rectangle.
 * @param color Color of the rectangle.
 */
void DrawRectangle(Vec2 position, Vec2 size, Color color);

/**
 * @brief Draws a 2D circle.
 *
 * @param center Center position of the circle.
 * @param radius Radius of the circle.
 * @param color Color of the circle.
 */
void DrawCircle(Vec2 center, float radius, Color color);

#endif
