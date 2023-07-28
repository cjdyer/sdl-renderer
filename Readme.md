# SDL Renderer

This is a simple 3D raycasting engine built with SDL3 and C++. The engine takes an array of booleans to create a map and then uses a multi-threaded raycasting technique to render the 3D scene in real-time.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

You need to have `cmake` and a C++ compiler installed on your system. Also, make sure to clone the repo with the `--recursive` flag to fetch the required submodules (SDL and yaml-cpp).

```bash
git clone --recursive https://github.com/cjdyer/sdl-renderer.git
```

### Building

To build the project:

1. Create a build directory and navigate into it:

```bash
mkdir build
cd build
```

2. Run CMake to configure the project and generate a build system:

```bash
cmake ..
```

3. Build the project:

```bash 
make
```

4. Run the application:

```bash
./sdl_renderer
```

## Configuration

You can adjust the application settings such as window size, field of view, acceleration, max velocity and map layout in the config.yaml file.