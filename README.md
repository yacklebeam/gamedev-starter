# Bifrost, an OpenGL Game Framework written in C++

- glfw for window management
- glad for OpenGL wrangling (via glfw)
- glm for vectors, matrices, and related math
- imgui for UI interfaces
- miniaudio for audio

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

3. Rename `game.cpp.example` to `game.cpp`
4. Use cmake to create build files
    
```
mkdir build && cd build
cmake ..
```

5. Use build tool or cmake to build project

```
cd build
cmake --build .

# creates bifrost.exe by default
```
