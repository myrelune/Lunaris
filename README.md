# Lunaris Engine

Lunaris is a lightweight C-based engine, utilizing OpenGL for graphics and GLFW for window/input management.

## Project Structure
*   `/include`: Public headers (e.g., `Lunaris.h`).
*   `/src`: Implementation source files.
*   `/libs`: Third-party dependencies (Glad, KHR).
*   `CMakeLists.txt`: Build configuration.

## Prerequisites
Ensure you have the following installed and available in your system PATH:
*   **CMake** (3.20+)
*   **A C Compiler** (MSVC for Windows, GCC/MinGW, or Clang)
*   **Git** (Used to fetch GLFW automatically)

## Build Instructions

1.  **Clone the Repository**:
    ```bash
    git clone [https://github.com/myrelune/Lunaris.git](https://github.com/myrelune/Lunaris.git)
    cd Lunaris
    ```

2.  **Generate Build Files**:
    Create a build directory to keep your source tree clean.
    ```bash
    mkdir build
    cd build
    cmake ..
    ```

3.  **Compile**:
    Run the build command. CMake will automatically download and configure the GLFW dependency during this step.
    ```bash
    # For Windows/Visual Studio users:
    cmake --build . --config Release

    # For Linux/macOS:
    cmake --build .
    ```

## Project Dependencies
*   **GLFW**: Handled automatically via CMake `FetchContent`.
*   **Glad**: Provided locally in the `/libs` directory.

## How to Use in Your Projects
Since this project is set up as a static library, you can consume it by adding this directory to your own `CMakeLists.txt`:

```cmake
# Add the engine to your project
add_subdirectory(path/to/lunaris)

# Link it to your executable
target_link_libraries(YourProjectName PRIVATE Lunaris)
