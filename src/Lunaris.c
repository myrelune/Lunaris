#include "../include/Lunaris.h"

#include "glad/gl.h"
#include "GLFW/glfw3.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#ifdef _WIN32
    #include <windows.h>
    #else
    #include <time.h>
#endif

typedef struct Vertex {
    Vec2 position;
    Color color;
} Vertex;

static GLFWwindow* g_window = NULL;

static GLuint g_defaultShader = 0;

static GLuint g_batchVAO = 0;
static GLuint g_batchVBO = 0;

static int g_windowWidth = 0;
static int g_windowHeight = 0;

static Vertex* g_vertices = NULL;
static size_t g_vertexCount = 0;
static size_t g_vertexCapacity = 0;

static size_t g_batchCapacity = 1000000;

static int g_targetFPS = 0;
static double g_frameStartTime = 0.0;
static float frameTime = 0.0f;

static const char* defaultVertexShader =
"#version 330 core\n"
"\n"
"layout(location = 0) in vec2 aPosition;\n"
"layout(location = 1) in vec4 aColor;\n"
"\n"
"out vec4 vColor;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPosition, 0.0, 1.0);\n"
"    vColor = aColor;\n"
"}\n";

static const char* defaultFragmentShader =
"#version 330 core\n"
"\n"
"in vec4 vColor;\n"
"\n"
"out vec4 FragColor;\n"
"\n"
"void main()\n"
"{\n"
"    FragColor = vColor;\n"
"}\n";

static GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertexSource, NULL);

    glCompileShader(vertexShader);

    int success;
    char infoLog[512];

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);

        fprintf(
            stderr,
            "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n",
            infoLog
        );

        glDeleteShader(vertexShader);

        return 0;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);

    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);

        fprintf(
            stderr,
            "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n",
            infoLog
        );

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return 0;
    }

    GLuint shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);

        fprintf(
            stderr,
            "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n",
            infoLog
        );

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(shaderProgram);

        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

static bool GrowVertexBuffer(size_t requiredCapacity) {
    if (requiredCapacity <= g_vertexCapacity)
        return true;

    size_t newCapacity = g_vertexCapacity;

    if (newCapacity == 0)
        newCapacity = 10000;

    while (newCapacity < requiredCapacity)
        newCapacity *= 2;

    Vertex* newVertices = (Vertex*)realloc(
        g_vertices,
        newCapacity * sizeof(Vertex)
    );

    if (!newVertices)
        return false;

    g_vertices = newVertices;
    g_vertexCapacity = newCapacity;

    // FIX: Resize the GPU VBO to match the new capacity!
    glBindBuffer(GL_ARRAY_BUFFER, g_batchVBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        g_vertexCapacity * sizeof(Vertex),
        NULL,
        GL_DYNAMIC_DRAW
    );
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
}

static void FlushBatch();

static bool PushVertex(Vertex vertex) {
    if (g_vertexCount >= g_batchCapacity)
        FlushBatch();

    if (!GrowVertexBuffer(g_vertexCount + 1))
        return false;

    g_vertices[g_vertexCount] = vertex;

    g_vertexCount++;

    return true;
}

static void CreateBatch() {
    glGenVertexArrays(1, &g_batchVAO);

    glGenBuffers(1, &g_batchVBO);

    glBindVertexArray(g_batchVAO);

    glBindBuffer(GL_ARRAY_BUFFER, g_batchVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        g_batchCapacity * sizeof(Vertex),
        NULL,
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, position)
    );

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, color)
    );

    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

static Vec2 ScreenToNDC(Vec2 point) {
    Vec2 result;

    result.x =
        (point.x / (float)g_windowWidth) * 2.0f - 1.0f;

    result.y =
        1.0f - (point.y / (float)g_windowHeight) * 2.0f;

    return result;
}


bool InitWindow(int width, int height, const char* title) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");

        return false;
    }

    g_window = glfwCreateWindow(
        width,
        height,
        title,
        NULL,
        NULL
    );

    if (!g_window) {
        fprintf(stderr, "Failed to create GLFW window\n");

        glfwTerminate();

        return false;
    }

    glfwMakeContextCurrent(g_window);

    glfwSwapInterval(0);

    if (!gladLoadGL(glfwGetProcAddress)) {
        fprintf(stderr, "Failed to load OpenGL\n");

        glfwDestroyWindow(g_window);

        glfwTerminate();

        g_window = NULL;

        return false;
    }

    g_windowWidth = width;
    g_windowHeight = height;

    glViewport(0, 0, width, height);

    g_defaultShader = CreateShaderProgram(
        defaultVertexShader,
        defaultFragmentShader
    );

    if (g_defaultShader == 0) {
        glfwDestroyWindow(g_window);

        glfwTerminate();

        g_window = NULL;

        return false;
    }

    CreateBatch();

    if (!GrowVertexBuffer(g_batchCapacity)) {
        glDeleteBuffers(1, &g_batchVBO);

        glDeleteVertexArrays(1, &g_batchVAO);

        glDeleteProgram(g_defaultShader);

        glfwDestroyWindow(g_window);

        glfwTerminate();

        g_window = NULL;

        return false;
    }

    glClearColor(0, 0, 0, 1);

    return true;
}

bool WindowShouldClose() {
    return glfwWindowShouldClose(g_window);
}

static void SleepSeconds(double seconds) {
    if (seconds <= 0.0)
        return;

    #ifdef _WIN32
        DWORD milliseconds = (DWORD)(seconds * 1000.0);

        if (milliseconds > 0)
            Sleep(milliseconds);
    #else
        struct timespec ts;

        ts.tv_sec = (time_t)seconds;
        ts.tv_nsec = (long)((seconds - (double)ts.tv_sec) * 1000000000.0);

        nanosleep(&ts, NULL);
    #endif
}

static void LimitFrameRate(void) {
    if (g_targetFPS <= 0)
        return;

    double targetFrameTime =
        1.0 / (double)g_targetFPS;

    double elapsed =
        glfwGetTime() - g_frameStartTime;

    double remaining =
        targetFrameTime - elapsed;

    if (remaining <= 0.0)
        return;

    /*
     * Sleep most of the remaining time.
     *
     * Leave about 1ms for the precision spin below.
     */
    if (remaining > 0.001)
        SleepSeconds(remaining - 0.001);

    /*
     * Precisely wait for the rest.
     */
    while ((glfwGetTime() - g_frameStartTime) < targetFrameTime) {
        // spin
    }
}

void BeginDrawing() {
    g_frameStartTime = glfwGetTime();

    glfwPollEvents();

    g_vertexCount = 0;

    int width;
    int height;

    glfwGetFramebufferSize(g_window, &width, &height);

    if (width != g_windowWidth || height != g_windowHeight) {
        g_windowWidth = width;
        g_windowHeight = height;

        glViewport(0, 0, width, height);
    }

    glClear(
        GL_COLOR_BUFFER_BIT |
        GL_DEPTH_BUFFER_BIT
    );
}

static void FlushBatch() {
    if (g_vertexCount == 0)
        return;

    glUseProgram(g_defaultShader);

    glBindVertexArray(g_batchVAO);

    glBindBuffer(GL_ARRAY_BUFFER, g_batchVBO);

    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        g_vertexCount * sizeof(Vertex),
        g_vertices
    );

    glDrawArrays(
        GL_TRIANGLES,
        0,
        (GLsizei)g_vertexCount
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    g_vertexCount = 0;
}

void EndDrawing() {
    FlushBatch();

    glfwSwapBuffers(g_window);

    LimitFrameRate();

    double now = glfwGetTime();

    g_frameTime =
        (float)(now - g_frameStartTime);
}

void CloseWindow() {
    if (g_vertices) {
        free(g_vertices);

        g_vertices = NULL;

        g_vertexCount = 0;
        g_vertexCapacity = 0;
    }

    if (g_batchVBO) {
        glDeleteBuffers(1, &g_batchVBO);

        g_batchVBO = 0;
    }

    if (g_batchVAO) {
        glDeleteVertexArrays(1, &g_batchVAO);

        g_batchVAO = 0;
    }

    if (g_defaultShader) {
        glDeleteProgram(g_defaultShader);

        g_defaultShader = 0;
    }

    if (g_window) {
        glfwDestroyWindow(g_window);

        g_window = NULL;
    }

    glfwTerminate();
}

int GetScreenWidth() {
    return g_windowWidth;
}

int GetScreenHeight() {
    return g_windowHeight;
}

void SetTargetFPS(int fps) {
    if (fps < 0)
        fps = 0

    g_targetFPS = fps;
}

float GetFrameTime(void) {
    return g_frameTime;
}

float GetTime() {
    return (float)glfwGetTime();
}

void DrawTriangle(Vec2 center, float radius, float rotation, Color color) {
    Vec2 a;
    Vec2 b;
    Vec2 c;

    if (rotation == 0.0f) {
        a = (Vec2){
            center.x,
            center.y - radius
        };

        b = (Vec2){
            center.x - radius * 0.8660254f,
            center.y + radius * 0.5f
        };

        c = (Vec2){
            center.x + radius * 0.8660254f,
            center.y + radius * 0.5f
        };
    } else {
        float cosAngle = cosf(rotation);
        float sinAngle = sinf(rotation);

        Vec2 localA = {
            0.0f,
            -radius
        };

        Vec2 localB = {
            -radius * 0.8660254f,
            radius * 0.5f
        };

        Vec2 localC = {
            radius * 0.8660254f,
            radius * 0.5f
        };

        a = (Vec2){
            center.x +
                localA.x * cosAngle -
                localA.y * sinAngle,

            center.y +
                localA.x * sinAngle +
                localA.y * cosAngle
        };

        b = (Vec2){
            center.x +
                localB.x * cosAngle -
                localB.y * sinAngle,

            center.y +
                localB.x * sinAngle +
                localB.y * cosAngle
        };

        c = (Vec2){
            center.x +
                localC.x * cosAngle -
                localC.y * sinAngle,

            center.y +
                localC.x * sinAngle +
                localC.y * cosAngle
        };
    }

    a = ScreenToNDC(a);
    b = ScreenToNDC(b);
    c = ScreenToNDC(c);

    PushVertex((Vertex){a, color});
    PushVertex((Vertex){b, color});
    PushVertex((Vertex){c, color});
}

void DrawRectangle(Vec2 position, Vec2 size, Color color) {
    Vec2 halfSize = {
        size.x * 0.5f,
        size.y * 0.5f
    };

    Vec2 topLeft = {
        position.x - halfSize.x,
        position.y - halfSize.y
    };

    Vec2 topRight = {
        position.x + halfSize.x,
        position.y - halfSize.y
    };

    Vec2 bottomRight = {
        position.x + halfSize.x,
        position.y + halfSize.y
    };

    Vec2 bottomLeft = {
        position.x - halfSize.x,
        position.y + halfSize.y
    };

    topLeft = ScreenToNDC(topLeft);
    topRight = ScreenToNDC(topRight);
    bottomRight = ScreenToNDC(bottomRight);
    bottomLeft = ScreenToNDC(bottomLeft);

    PushVertex((Vertex){topLeft, color});
    PushVertex((Vertex){topRight, color});
    PushVertex((Vertex){bottomRight, color});

    PushVertex((Vertex){topLeft, color});
    PushVertex((Vertex){bottomRight, color});
    PushVertex((Vertex){bottomLeft, color});
}


void DrawCircle(Vec2 center, float radius, Color color) {
    const int segments = 32;
    const float PI = 3.14159265359f;

    for (int i = 0; i < segments; i++) {
        float angle1 =
            ((float)i / segments) * 2.0f * PI;

        float angle2 =
            ((float)(i + 1) / segments) * 2.0f * PI;

        Vec2 a = center;

        Vec2 b = {
            center.x + cosf(angle1) * radius,
            center.y + sinf(angle1) * radius
        };

        Vec2 c = {
            center.x + cosf(angle2) * radius,
            center.y + sinf(angle2) * radius
        };

        a = ScreenToNDC(a);
        b = ScreenToNDC(b);
        c = ScreenToNDC(c);

        PushVertex((Vertex){a, color});
        PushVertex((Vertex){b, color});
        PushVertex((Vertex){c, color});
    }
}
