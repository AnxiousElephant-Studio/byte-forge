# Runtime module

> Language: **English** | [Português (Brasil)](../../../../pt-br/engine/modules/runtime/README.md)

The runtime module ties the core engine modules together. It is one class, `runtime_t`, that owns the platform, the renderer, the Lua state, the audio engine and the frame timer, initializes them in a fixed order, creates windows with a rendering context attached, and disposes everything at the end. It lives in the `byteforge::runtime` namespace and does not wrap any third-party library itself.

An application creates one `runtime_t` and reaches every core service through its public members (`runtime.platform`, `runtime.renderer`, `runtime.scripting`, `runtime.audio`, `runtime.frame_manager`).

## Files

| File                                                            | Contents                                   |
| --------------------------------------------------------------- | ------------------------------------------ |
| `engine/include/byte-forge-engine/modules/runtime/runtime.h`    | `runtime_t` declaration and constructor    |
| `engine/source/modules/runtime/runtime.cpp`                     | Initialization, windows, render, dispose   |

Modules owned by the runtime:

| Member          | Type                       | Documentation                              |
| --------------- | -------------------------- | ------------------------------------------ |
| `frame_settings`| `time::frame_settings_t`   | [time](../time/README.md)                  |
| `frame_manager` | `time::frame_manager_t`    | [time](../time/README.md)                  |
| `platform`      | `platform::platform_t`     | [platform](../platform/README.md)          |
| `renderer`      | `rendering::renderer_t`    | [rendering](../rendering/README.md)        |
| `scripting`     | `scripting::scripting_t`   | [scripting](../scripting/README.md)        |
| `audio`         | `audio::audio_engine_t`    | [audio](../audio/README.md)                |

## Concepts

### Ownership by value

Every module is a plain member of `runtime_t`, not a pointer. They are constructed in declaration order when the runtime is constructed, but constructors only set fields to empty values. Nothing touches SDL, OpenGL, Lua or miniaudio until `initialize()` runs.

Other code never creates its own copy of these services. It receives a pointer or reference to the runtime's member, as the editor does with `runtime.audio` and `runtime.scripting.get_lua_state()`.

### The close flag

`should_close` starts as `false`. The constructor passes its address to the platform (`platform(display_descriptor, &should_close)`), and `platform_t::request_close()` sets it to `true`. The platform calls `request_close()` itself when SDL reports a quit event, and an application can call it too. The application's main loop runs while `should_close` is `false`.

### Initialization order

```
platform.initialize()       SDL video and events subsystems
scripting.initialize()      Lua state and standard libraries
audio.initialize()          miniaudio engine on the default device
renderer.initialize()       OpenGL context attributes (core profile and version)
frame_manager.reset()       frame clock starts, target frame time computed
```

### Dispose order

```
audio.dispose()             stops every sound, frees the miniaudio engine
scripting.dispose()         closes the Lua state
renderer.dispose()          destroys every OpenGL context
platform.dispose()          destroys every window, quits SDL
```

The platform goes last, so the windows and SDL still exist while the renderer destroys the OpenGL contexts that belong to them. The frame manager and frame settings hold no resources and are not disposed.

## `runtime_t`

```cpp
class BF_RUNTIME_API runtime_t
{
public:
    bool should_close;
    time::frame_settings_t frame_settings;
    time::frame_manager_t frame_manager;
    platform::platform_t platform;
    rendering::renderer_t renderer;
    scripting::scripting_t scripting;
    audio::audio_engine_t audio;

    runtime_t(platform::display_descriptor_t display_descriptor, time::frame_settings_t frame_settings);

    bool initialize();
    platform::window_t* create_window(const platform::window_descriptor_t& descriptor);
    void destroy_window(u32 window_id);
    void render();
    void dispose();
};
```

There is no destructor. Cleanup happens only in `dispose()`, which the owner has to call.

### State

| Field            | Meaning                                                              |
| ---------------- | -------------------------------------------------------------------- |
| `should_close`   | `true` once a close was requested. The main loop checks it           |
| `frame_settings` | The settings passed to the constructor. Holds `target_fps`           |
| `frame_manager`  | Frame timer. Provides `delta_time` and limits the frame rate         |
| `platform`       | SDL setup, the window manager and event polling                      |
| `renderer`       | One OpenGL context per window, frame begin and end                   |
| `scripting`      | The single Lua state                                                 |
| `audio`          | The miniaudio engine                                                 |

### `runtime_t(display_descriptor, frame_settings)`

Stores `frame_settings`, sets `should_close` to `false`, and constructs the platform with the display descriptor and the address of `should_close`. The other members are default-constructed. Nothing is initialized yet.

### `initialize()`

1. Calls `platform.initialize()`, then `scripting.initialize()`, then `audio.initialize()`, stopping at the first one that returns `false`. In that case it returns `false` right away, and the modules after the failing one are not initialized.
2. Calls `renderer.initialize()`, which cannot fail.
3. Calls `frame_manager.reset(frame_settings)`, which records the current tick count and computes the target frame time.
4. Returns `true`.

The runtime logs nothing itself. Each module logs its own failure (`Could not initialize SDL: ...`, `Could not create the Lua state.`, `Could not initialize the audio engine (...)`). Modules that were already initialized when a later one fails are not rolled back here, so the owner must still call `dispose()`.

### `create_window(descriptor)`

1. Asks the platform's window manager to create a window from `descriptor` and the platform's display descriptor. If that returns `nullptr`, returns `nullptr`.
2. Attaches the window to the renderer, which creates an OpenGL context for it. If that fails, destroys the window and returns `nullptr`.
3. Returns the window.

The window is owned by the window manager. The returned pointer is a way to reach it, not ownership.

### `destroy_window(window_id)`

Detaches the window from the renderer (destroying its OpenGL context), then destroys the window in the window manager. `window_id` is the SDL window ID, `window_t::sdl_window_id`.

### `render()`

Goes through every window in the window manager. For each one, `renderer.begin_frame()` makes its context current, sets the viewport and clears it with the renderer's clear color. If that succeeds, `renderer.end_frame()` swaps its buffers. A window whose context can't be made current is skipped.

`render()` draws nothing besides the clear color. The editor does not call it; it calls `renderer.begin_frame()` and `renderer.end_frame()` itself so that it can draw the scene and the UI in between.

### `dispose()`

Disposes audio, scripting, the renderer and the platform, in that order. Each of these `dispose()` functions checks its own state, so calling the runtime's `dispose()` after a partial or failed `initialize()` is safe.

## Usage

The editor owns a `runtime_t` and builds it in its constructor:

```cpp
editor_t::editor_t()
    : runtime(platform::display_descriptor_t(graphics::e_graphic_backends::OpenGL), time::frame_settings_t(target_fps))
```

`editor_t::initialize()` calls `runtime.initialize()` first and then `runtime.create_window()`. If either fails it logs a fatal error and returns `false`, and `main()` calls `editor.dispose()` anyway, which ends with `runtime.dispose()`.

The main loop uses the members directly (shortened here, without the profiler and input lines):

```cpp
while (!runtime.should_close)
{
    runtime.frame_manager.start_frame();
    runtime.platform.poll_events(input_callback, window_callback);
    scene_player.update(runtime.frame_manager.delta_time);
    runtime.audio.update();

    if (runtime.renderer.begin_frame(*window))
    {
        // scene and UI
        runtime.renderer.end_frame(*window);
    }

    runtime.frame_manager.end_frame();
}
```

The window callback calls `runtime.platform.request_close()` when a window reports `CloseRequested`, which ends the loop.

`editor_t::dispose()` disposes the scene player, the scene, the caches, the sprite renderer and the UI context first, and calls `runtime.dispose()` last. The scene player removes its Lua references while the Lua state is still open.

## Notes

- The dispose order is not the exact reverse of the initialization order. Audio is disposed first and the renderer, which is initialized last, is disposed after scripting.
- If `platform.initialize()` fails, `initialize()` returns before `scripting.initialize()` and `audio.initialize()` run. Their `dispose()` calls then find a null state and a null engine and do nothing.
- `frame_manager.reset()` runs at the end of `initialize()`, so everything the application does between `initialize()` and its first `end_frame()` (creating the window, loading shaders) counts toward the first frame's `delta_time`, which the frame manager caps at `0.05` seconds.
- `runtime_t` does not delete its copy operations. Copying it would copy the module members, including their raw pointers.
- `render()` is not used by the editor.
