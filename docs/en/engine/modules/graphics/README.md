# Graphics module

> Language: **English** | [Português (Brasil)](../../../../pt-br/engine/modules/graphics/README.md)

The graphics module names the graphics APIs the engine could draw with. Today it holds a single enum, `e_graphic_backends`, used to tell the platform layer which kind of window to create. It lives in the `byteforge::graphics` namespace and is header-only. The drawing itself is done by the [rendering module](../rendering/README.md).

## Files

| File                                                                  | Contents                     |
| --------------------------------------------------------------------- | ---------------------------- |
| `engine/include/byte-forge-engine/modules/graphics/e_graphic_backends.h` | `e_graphic_backends` enum |

Related pieces in other modules:

| File                                                      | Role                                                    |
| --------------------------------------------------------- | ------------------------------------------------------- |
| `modules/platform/display/display_descriptor.h`           | Stores the chosen backend                               |
| `modules/platform/display/window/window_manager.cpp`      | Adds `SDL_WINDOW_OPENGL` to new windows for `OpenGL`    |
| `applications/editor/source/editor.cpp`                   | Builds the runtime with `e_graphic_backends::OpenGL`    |

## `e_graphic_backends`

```cpp
enum class e_graphic_backends : u8
{
    None       = 0,
    OpenGL     = 1,
    DirectX11  = 2,
    DirectX12  = 3,
    Vulkan     = 4,
    WebGPU     = 5,
    Metal      = 6,
    Software   = 7
};
```

The enum is scoped and one byte wide. The values are fixed explicitly, so they stay the same if the list is reordered.

| Value       | Meaning in the current code                                    |
| ----------- | -------------------------------------------------------------- |
| `None`      | No backend. Windows are created without an OpenGL flag         |
| `OpenGL`    | The only backend with an effect: windows get `SDL_WINDOW_OPENGL` |
| `DirectX11` | Declared, not referenced anywhere else                         |
| `DirectX12` | Declared, not referenced anywhere else                         |
| `Vulkan`    | Declared, not referenced anywhere else                         |
| `WebGPU`    | Declared, not referenced anywhere else                         |
| `Metal`     | Declared, not referenced anywhere else                         |
| `Software`  | Declared, not referenced anywhere else                         |

## Usage

The backend is chosen once, when the runtime is built, through `platform::display_descriptor_t`:

```cpp
struct display_descriptor_t
{
public:
    graphics::e_graphic_backends graphic_backend;

    explicit display_descriptor_t(const graphics::e_graphic_backends graphic_backend) : graphic_backend(graphic_backend) {}
};
```

The editor passes `OpenGL`:

```cpp
editor_t::editor_t()
    : runtime(platform::display_descriptor_t(graphics::e_graphic_backends::OpenGL), time::frame_settings_t(target_fps))
```

`platform_t` keeps the descriptor, and `window_manager_t::create_window()` reads it when it builds the SDL window flags. When the backend is `OpenGL` it adds `SDL_WINDOW_OPENGL`, which SDL needs before an OpenGL context can be created for that window.

## Notes

- Only the window flags read the enum. `renderer_t` always creates OpenGL contexts, whatever backend is in the descriptor. With any value other than `OpenGL`, the window lacks `SDL_WINDOW_OPENGL` and `renderer_t::attach_window()` is expected to fail when it tries to create a context.
- The header includes only `api/types.h` and declares no exported symbols, so it needs no `BF_GRAPHICS_API`.
