# Player

> Language: **English** | [Português (Brasil)](../../../pt-br/applications/player/README.md)

The player runs a ByteForge project without the editor. It is the executable a finished game ships with.

```
player.exe [path/to/project.bfproject]
```

Without an argument it uses the first `.bfproject` file in the current folder. It then:

1. Loads the project and makes its folder the working directory, so asset paths resolve like in the editor.
2. Starts the engine runtime with the project's target FPS and vsync, and opens a 1280x720 resizable window titled with the project name.
3. Creates the sprite renderer with the project's sprite shaders, the point light renderer and the light map.
4. Loads the project's start level and starts playing.

Every frame it polls input, runs the scene (update and render) straight into the window and swaps. When a script calls `scene.load_level(path)`, the player stops, loads that level and starts again. Stamps spawned from Lua use `level::stamp_file_t::instantiate`.

The source is in `applications/player`: `player_application_t` owns the runtime, the scene context and the renderers; `main.cpp` only finds the project file.
