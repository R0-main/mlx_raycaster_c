# MLX Raycaster in C

## Overview
This project is a simple first-person raycaster written in C on top of the [MiniLibX](https://harm-smits.github.io/42docs/libs/minilibx) graphics library. It renders textured walls, sky, and floor tiles, and exposes both a 3D viewport and a 2D top-down debug view so you can inspect how ray casting works step by step. The entire engine lives in `src/main.c`, making it easy to study or extend the rendering pipeline.

## Features
- **Textured rendering** – Walls, floor, and sky are sampled from XPM textures to create a retro 3D look. Texture swapping at runtime is supported with a single key press.
- **Dual windows** – The engine opens a main window for the 3D scene and a secondary debug window that renders the tile map and ray paths.
- **Interactive controls** – Move around the map with `WASD`, tilt the camera pitch with the arrow keys, and toggle wall textures with `B` to see lighting and shading changes instantly.
- **Built-in map definition** – Levels are described with a compact `MAP_WIDTH` × `MAP_HEIGHT` integer array, making it straightforward to sketch new rooms and obstacles.

## Repository structure
```
.
├── assets/              # Textures used for walls, floor, and sky
├── libs/                # External libraries (ft_libc and MiniLibX) pulled in by the Makefile
├── src/
│   └── main.c           # Raycaster implementation and game loop
├── Makefile             # Build script that configures dependencies and compiles the binary
└── README.md            # Project documentation (this file)
```

## Prerequisites
- A POSIX environment with `clang` (or GCC) and `make`
- X11 development packages (`libx11`, `libxext`, `zlib`) – required by MiniLibX
- Git – the Makefile clones the required submodules automatically on first build

## Building
```bash
make
```
The build system downloads MiniLibX and `ft_libc` (if missing), compiles the project, and produces the `mlx_raycatser_c` executable in the repository root.

## Running
After building, launch the raycaster with:
```bash
./mlx_raycatser_c
```
Two windows will appear: the main viewport titled **"RayCaster in C"** and a debug window titled **"DEBUG RayCaster in C"**.

## Controls
| Key | Action |
| --- | --- |
| `W` / `S` | Move the player forward / backward |
| `A` / `D` | Rotate the camera left / right |
| `↑` / `↓` | Raise / lower the camera pitch |
| `B` | Swap between the available wall textures |
| Window close button | Exit the application |

## Customising the map
Maps are stored as a flattened array of `0`s (empty space) and `1`s (walls). Edit the array in `main.c` to design your own layout:

```c
#define MAP_WIDTH 10
#define MAP_HEIGHT 7

char map[MAP_HEIGHT * MAP_WIDTH] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 0, 0, 0, 1, 0, 0, 1,
    1, 0, 0, 0, 1, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};
```

The draw loop automatically uses the updated data, so rebuilding and running the program is all you need to see your new map.

## Example: Initialising a scene
The snippet below shows how the engine initialises the rendering context, loads textures, and starts the main loop:

```c
t_data data;
int    texture_size;

bzero(&data, sizeof(t_data));

// Player setup
data.player.position.x = SCREEN_WIDTH / 2 - SIZE;
data.player.position.y = SCREEN_HEIGHT / 2 - SIZE;
data.player.rotation_angle = 45 * (PI / 180);

data.map = map;

// MiniLibX context and windows
data.mlx = mlx_init();
data.win = mlx_new_window(data.mlx, SCREEN_WIDTH, SCREEN_HEIGHT, "RayCaster in C");
data.debug_win = mlx_new_window(data.mlx, SCREEN_WIDTH, SCREEN_HEIGHT, "DEBUG RayCaster in C");

data.rendering_buffer = mlx_new_image(data.mlx, SCREEN_WIDTH, SCREEN_HEIGHT);
data.debug_rendering_buffer = mlx_new_image(data.mlx, SCREEN_WIDTH, SCREEN_HEIGHT);

// Texture loading
data.wall_texture = mlx_xpm_file_to_image(data.mlx, "assets/building.xpm", &texture_size, &texture_size);
data.other = mlx_xpm_file_to_image(data.mlx, "assets/romain.xpm", &texture_size, &texture_size);
data.floor = mlx_xpm_file_to_image(data.mlx, "assets/wall2.xpm", &texture_size, &texture_size);
data.sky = mlx_xpm_file_to_image(data.mlx, "assets/sky.xpm", &texture_size, &texture_size);

// Event loop
mlx_loop_hook(data.mlx, loop, &data);
mlx_loop(data.mlx);
```

Use this as a starting point for integrating new gameplay logic, adding more textures, or experimenting with different camera controls. Happy hacking!
