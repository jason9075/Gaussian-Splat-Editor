# Gaussian Splat Editor (WIP)

Gaussian Splat Editor is an OpenGL tool designed for rendering Gaussian Splat .ply files and allowing users to remove unwanted splats.

## Example Video

[![Example Video](./imgs/gs.gif)](https://youtu.be/_gb3ja7yWPo?si=VWrYYp2CCI9ugnqk)

## Features

- **Render Gaussian Splat .ply Files**: Visualize complex point cloud data using Gaussian splatting.
- **Selective Splat Removal**: Easily remove splats that are not needed or are incorrectly placed.
- **Real-time Editing**: Utilize OpenGL for fast rendering and interactive manipulation.

## Build Instructions

### Requirements

- **CMake** 3.10 or higher
- **OpenGL** development libraries
- **SDL2** for window management and input
- **GLEW** for handling OpenGL extensions (macOS only)
- **glm** for mathematics
- **nlohmann_json** for JSON parsing
- **imgui** for GUI rendering

### Installation

To build the project, follow these steps:

```bash
# Active nix environment
nix develop -c zsh

# If you need to clean the build directory, use the -r option
bash ./build.sh -r

# Build the project
bash ./build.sh

# Run the application
./build/playground.app
```

## Ref:

This project is inspired by and builds upon the work of several existing projects:

[antimatter15's webgl viewer](https://github.com/antimatter15/splat):Use texture to send info in shader.

[kishimisu's webgl viewer](https://github.com/kishimisu/Gaussian-Splatting-WebGL):Nice implements, draw mutiple instance to generate gaussian vertex.

[hyperlogic's opengl viewer](https://github.com/hyperlogic/splatapult):Use Geometry shader to generate gaussian vertex.
