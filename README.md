# A gamedev starter kit for OpenGL game development

- glfw for window management
- glad for OpenGL wrangling (via glfw)
- glm for vectors, matrices, and related math
- imgui for UI interfaces
- miniaudio for audio
- stb for image loading and more
- bifrost (my own library) for helpful OpenGL functions

## About

A simple hobby OpenGL-based game framework written in C++.

## Dependencies

1. [cmake](https://cmake.org/download/)
2. [Visual Studio](https://visualstudio.microsoft.com/) (Windows)
3. make (Linux)

## Usage

1. Clone repo
2. Update git submodules
    
```
git submodule update --init --recursive
```

3. Use cmake to create build files
    
```
mkdir build && cd build
cmake ..
```

4. Use build tool or cmake to build project

```
cd build
cmake --build .

# creates game.exe by default
```

## About

src/game.cpp contains the game-related code
src/main.cpp contains the GLFW and IMGUI initialization, both can be edited to the heart's desire.
