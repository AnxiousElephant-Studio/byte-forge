# Rendering module

> Language: **English** | [Português (Brasil)](../../../../pt-br/engine/modules/rendering/README.md)

The rendering module draws the engine's 2D content with OpenGL 4.6 core. Everything lives in the `byteforge::rendering` namespace, split in folders that are meant to become separate modules later:

| Folder                 | What it holds                                                                                      | Depends on                                  |
| ---------------------- | -------------------------------------------------------------------------------------------------- | ------------------------------------------- |
| `rendering/opengl/`    | Thin OpenGL layer: function loading, render state, framebuffer, render pass, shader, mesh, texture | glad, math, logging, io (shaders), image (textures) |
| `rendering/window/`    | The bridge with a platform window: `opengl_window_context_t` (SDL) and `renderer_t`                | `rendering/opengl`, platform, SDL3          |
| `rendering/lighting/`  | `point_light_t`, plain light data                                                                  | math                                        |
| `rendering/material/`  | `material_t`: surface file + shading model (Unlit, Lit) compiled into one program                 | `rendering/opengl`, io, math                |
| `rendering/sprite/`, `collider/`, `font/`, `texture/` | 2D drawing on top of the OpenGL layer, and the texture and font caches | `rendering/opengl`, math, io, image         |

Nothing under `rendering/opengl/` includes SDL, `platform`, the ECS or the scene. It only needs a current OpenGL context and the functions loaded, so it can be used with a window from SDL, GLFW or anything else (see [Using the OpenGL layer without the platform module](#using-the-opengl-layer-without-the-platform-module)). `rendering/window/` is the only place that knows about SDL and `platform::window_t`.

The OpenGL functions are loaded with [glad](https://github.com/Dav1dde/glad). Fonts are baked with `stb_truetype`, and images are decoded by the [image module](../image/README.md).

## Files

| File                                                                                              | Contents                                               |
| ------------------------------------------------------------------------------------------------- | ------------------------------------------------------ |
| `engine/include/byte-forge-engine/modules/rendering/opengl/functions/opengl_functions.h`          | `opengl_functions_t`, `opengl_function_loader_t`       |
| `engine/include/byte-forge-engine/modules/rendering/opengl/state/opengl_render_state.h`           | `opengl_render_state_t` (blending, viewport, clear)    |
| `engine/include/byte-forge-engine/modules/rendering/opengl/framebuffer/opengl_framebuffer.h`      | `opengl_framebuffer_t` declaration                     |
| `engine/include/byte-forge-engine/modules/rendering/opengl/framebuffer/opengl_framebuffer_descriptor.h` | `opengl_framebuffer_descriptor_t`                |
| `engine/include/byte-forge-engine/modules/rendering/opengl/pass/opengl_render_pass.h`             | `opengl_render_pass_t` declaration                     |
| `engine/include/byte-forge-engine/modules/rendering/opengl/shader/opengl_shader.h`                | `opengl_shader_t` declaration                          |
| `engine/include/byte-forge-engine/modules/rendering/opengl/mesh/opengl_mesh.h`                    | `opengl_mesh_t` declaration                            |
| `engine/include/byte-forge-engine/modules/rendering/opengl/texture/opengl_texture.h`              | `opengl_texture_t` declaration                         |
| `engine/include/byte-forge-engine/modules/rendering/window/opengl_window_context.h`               | `opengl_window_context_t` declaration                  |
| `engine/include/byte-forge-engine/modules/rendering/window/renderer.h`                            | `renderer_t` declaration                               |
| `engine/include/byte-forge-engine/modules/rendering/lighting/point_light.h`                       | `point_light_t`                                        |
| `engine/include/byte-forge-engine/modules/rendering/material/material.h`                          | `material_t`, `e_material_shading_models`              |
| `engine/include/byte-forge-engine/modules/rendering/material/material_cache.h`                    | `material_cache_t`                                     |
| `engine/include/byte-forge-engine/modules/rendering/lighting/point_light_renderer.h`              | `point_light_renderer_t`                               |
| `engine/source/modules/rendering/material/material.cpp`                                           | Shader templates and assembly of the material          |
| `engine/include/byte-forge-engine/modules/rendering/sprite/sprite_renderer.h`                     | `sprite_renderer_t` declaration                        |
| `engine/include/byte-forge-engine/modules/rendering/font/font.h`                                  | `font_glyph_t`, `font_t` and the glyph range constants |
| `engine/include/byte-forge-engine/modules/rendering/font/font_cache.h`                            | `font_cache_t` declaration                             |
| `engine/include/byte-forge-engine/modules/rendering/texture/texture_cache.h`                      | `texture_cache_t` declaration                          |
| `engine/source/modules/rendering/opengl/functions/opengl_functions.cpp`                           | glad loading                                           |
| `engine/source/modules/rendering/opengl/state/opengl_render_state.cpp`                            | Blend, viewport and clear calls                        |
| `engine/source/modules/rendering/opengl/framebuffer/opengl_framebuffer.cpp`                       | Framebuffer and attachments                            |
| `engine/source/modules/rendering/opengl/pass/opengl_render_pass.cpp`                              | Start of a pass: bind, viewport, clear                 |
| `engine/source/modules/rendering/opengl/shader/opengl_shader.cpp`                                 | Shader compilation, linking and uniform setters        |
| `engine/source/modules/rendering/opengl/mesh/opengl_mesh.cpp`                                     | Vertex array and buffers, the unit quad                |
| `engine/source/modules/rendering/opengl/texture/opengl_texture.cpp`                               | Texture upload and binding                             |
| `engine/source/modules/rendering/window/opengl_window_context.cpp`                                | SDL context creation                                   |
| `engine/source/modules/rendering/window/renderer.cpp`                                             | Context list, frame begin and end                      |
| `engine/source/modules/rendering/sprite/sprite_renderer.cpp`                                      | Projection, quad drawing and text layout               |
| `engine/source/modules/rendering/collider/collider_renderer.cpp`                                  | Collider outlines                                      |
| `engine/source/modules/rendering/font/font.cpp`                                                   | Font baking and the `stb_truetype` implementation      |
| `engine/source/modules/rendering/font/font_cache.cpp`                                             | Font cache                                             |
| `engine/source/modules/rendering/texture/texture_cache.cpp`                                       | Texture cache                                          |

Related pieces in other modules:

| File                                                      | Role                                                 |
| --------------------------------------------------------- | ---------------------------------------------------- |
| `modules/runtime/runtime.h`                               | Owns the `renderer_t` and attaches it to new windows |
| `modules/scene/scene.cpp`                                 | `scene_t::render()` draws sprites and labels         |
| `modules/components/sprite_2D_component.h`                | Texture path, size and tint of a sprite              |
| `modules/components/label_2D_component.h`                 | Text, font path, font size and color of a label      |
| `modules/components/camera_2D_component.h`                | Zoom and "primary" flag of a camera                  |
| `modules/platform/display/window/window_manager.cpp`      | Adds `SDL_WINDOW_OPENGL` when the backend is OpenGL  |
| `applications/editor/assets/shaders/sprite.vert`, `.frag` | The shaders the editor gives to the sprite renderer  |

## Concepts

### How SDL and glad are hidden

`opengl_window_context_t` stores the SDL window and context as pointers to forward-declared types:

```cpp
struct SDL_Window;
struct SDL_GLContextState;
```

No public header of the module includes SDL or glad. The only `.cpp` that includes SDL is `opengl_window_context.cpp`. The `.cpp` files include `<glad/gl.h>` and use raw OpenGL calls. glad is built for the core profile, version 4.6 (`engine/xmake.lua`).

### One context per window

Every window gets its own OpenGL context. The first context is created alone. Every later one is created with `SDL_GL_SHARE_WITH_CURRENT_CONTEXT`, sharing with the first context in the list. Shared contexts see the same textures, buffers and shader programs, so a texture loaded while one window is current can be drawn in another. Vertex arrays are not shared by OpenGL, so a mesh is only usable in the context that created it.

The OpenGL functions are loaded once, after the first context exists, because glad needs a current context to query them. The function pointers live inside the engine library: an application that only uses the engine's OpenGL types does not need glad, but one that calls `gl*` itself has to load its own copy.

### The current context

OpenGL calls act on whatever context is current on the calling thread. None of the wrappers make a context current themselves. Creating, drawing and disposing shaders, meshes and textures only works while the right context is current. `renderer_t::begin_frame()` is what makes a window's context current.

### Frame flow

```
renderer.begin_frame(window)    -- make context current, set viewport, clear
sprite_renderer.begin(...)      -- enable alpha blending, upload projection
sprite_renderer.draw(...)       -- one draw call per quad
sprite_renderer.draw_text(...)  -- one draw call per visible glyph
(editor UI draws here)
renderer.end_frame(window)      -- swap buffers
```

### Coordinates

The sprite renderer uses Y-down screen coordinates and a `T * R * S` model matrix. Both conventions, the column-major layout and the orthographic projection are explained in the [math module](../math/README.md). Sprite positions are the **center** of the quad. Text positions are the **top-left** of the line.

### Ownership of GPU objects

The OpenGL wrappers and `font_t` have no destructor. GPU objects are released only by `dispose()` or `destroy()`, which the owner has to call while a context of the share group is still alive. The wrappers are plain structs and can be copied. A copy holds the same OpenGL ids, so disposing one copy leaves the other with dead ids.

## `renderer_t`

```cpp
class BF_RENDERING_API renderer_t
{
public:
    math::vec4_t clear_color;

    void initialize();
    bool attach_window(const platform::window_t& window);
    void detach_window(u32 window_id);
    bool begin_frame(const platform::window_t& window);
    void end_frame(const platform::window_t& window);
    void dispose();

    void clear(const math::vec4_t& color) const;
    void set_viewport(const math::recti_t& bounds) const;

private:
    opengl_window_context_t* find_window_context(u32 window_id);

    std::vector<opengl_window_context_t> window_contexts;
};
```

### State

| Field             | Meaning                                                                |
| ----------------- | ---------------------------------------------------------------------- |
| `clear_color`     | Color used by `begin_frame()`. Defaults to opaque black `(0, 0, 0, 1)` |
| `window_contexts` | One `opengl_window_context_t` per attached window, in attach order     |

### `initialize()`

Calls `opengl_window_context_t::set_context_attributes()`, which sets the SDL attributes for a 4.6 core context. It must run before the first window is attached.

### `attach_window(window)`

1. Creates an `opengl_window_context_t` for the window. If other contexts exist, the new one shares with `window_contexts.front()`.
2. If creation fails, logs `Could not create an OpenGL context for window <id>.`, destroys the partial context and returns `false`.
3. If the functions are not loaded yet, loads them with `opengl_functions_t::load(opengl_window_context_t::get_function_loader())`. On failure it logs `Could not load the OpenGL functions.`, destroys the context and returns `false`.
4. Sets vsync from `window.descriptor.use_vsync`. If SDL refuses, it logs the warning `Could not set vsync for window <id>.` and continues.
5. Adds the context to the list and returns `true`.

The new context is left current.

### `detach_window(window_id)`

Finds the context of that window, destroys it and removes it from the list. An unknown id does nothing.

### `begin_frame(window)`

1. Finds the window's context and makes it current. Returns `false` if there is no context or `SDL_GL_MakeCurrent` fails.
2. Binds the window framebuffer (`0`) and sets the viewport to the whole window, from `(0, 0)` to `window.get_size()`. That size is in physical pixels (`SDL_GetWindowSizeInPixels`).
3. Clears the color buffer with `clear_color` and returns `true`.

Only the color buffer is cleared. There is no depth or stencil clear.

### `end_frame(window)`

Swaps the window's buffers with `SDL_GL_SwapWindow`, which shows the frame. An unknown window does nothing.

### `dispose()`

Destroys every context and clears the list. The OpenGL functions stay loaded.

### `clear(color)` and `set_viewport(rect)`

Shortcuts for `opengl_render_state_t::clear_color()` and `opengl_render_state_t::set_viewport()`. They act on the current context. Blending is not part of the renderer: use `opengl_render_state_t`.

## `opengl_functions_t`

```cpp
using opengl_function_t        = void (*)();
using opengl_function_loader_t = opengl_function_t (*)(const char* function_name);

struct BF_RENDERING_API opengl_functions_t
{
public:
    static bool load(opengl_function_loader_t function_loader);
    static bool are_loaded();
};
```

OpenGL functions come from the driver at runtime, through the window library that created the context. `load()` passes the loader to `gladLoadGL` and returns `true` on success; a context must be current. `SDL_GL_GetProcAddress` and `glfwGetProcAddress` both have the `opengl_function_loader_t` shape, so the OpenGL layer does not need to know which window library is in use.

## `opengl_render_state_t`

OpenGL keeps one global state per context. These static helpers name each change, so a pass says what it needs:

| Member                         | OpenGL call                                                        |
| ------------------------------ | ------------------------------------------------------------------ |
| `enable_blending()`            | `glEnable(GL_BLEND)`                                               |
| `disable_blending()`           | `glDisable(GL_BLEND)`                                              |
| `use_alpha_blending()`         | `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`                |
| `use_additive_blending()`      | `glBlendFunc(GL_SRC_ALPHA, GL_ONE)`: overlapping colors add up     |
| `set_viewport(rect)`           | `glViewport` with the rectangle's position and size                |
| `clear_color(color)`           | `glClearColor` with the color, then `glClear(GL_COLOR_BUFFER_BIT)` |
| `clear_depth_and_stencil()`    | `glClear(GL_DEPTH_BUFFER_BIT \| GL_STENCIL_BUFFER_BIT)`            |

`sprite_renderer_t::begin()` and `collider_renderer_t::begin()` call `enable_blending()` and `use_alpha_blending()`.

## `opengl_framebuffer_t`

```cpp
struct opengl_framebuffer_descriptor_t
{
    bool use_depth_stencil;     // adds a 24-bit depth + 8-bit stencil renderbuffer
    bool read_color_as_opaque;  // reads the color texture with alpha 1 (GL_TEXTURE_SWIZZLE_A = GL_ONE)
};

class BF_RENDERING_API opengl_framebuffer_t
{
public:
    opengl_framebuffer_descriptor_t descriptor;

    explicit opengl_framebuffer_t(const opengl_framebuffer_descriptor_t& descriptor);

    bool resize(const math::vec2i_t& size);
    void dispose();
    void bind() const;
    static void bind_window();

    const opengl_texture_t& get_color_texture() const;
    math::vec2i_t get_size() const;
    math::recti_t get_bounds() const;
    bool is_valid() const;
};
```

A framebuffer is only a container. The images it draws into are its attachments: a color texture that a later pass can sample, and, when `use_depth_stencil` is set, a depth/stencil renderbuffer that is tested against but never sampled.

`resize(size)` creates the framebuffer on the first call and recreates the attachments only when the size changes, so calling it every frame with the window size is cheap. It checks `glCheckFramebufferStatus`, logs `Could not create the framebuffer with size: <w>x<h>. Cause: incomplete attachment (status: <hex>).` and disposes everything when the framebuffer is incomplete. It leaves the window framebuffer bound.

`bind_window()` binds framebuffer `0`, the one the window library created with the window. Framebuffers are not shared between contexts.

`read_color_as_opaque` exists for the editor: alpha blending leaves translucent edges with alpha below 1, and drawing the scene texture inside an ImGui panel would apply that alpha a second time.

## `opengl_render_pass_t`

A render pass is one round of drawing into one target. Every pass starts the same way, and these helpers do that part:

| Member                            | What it does                                                                                   |
| --------------------------------- | ---------------------------------------------------------------------------------------------- |
| `begin(framebuffer, clear_color)` | Binds the framebuffer, sets the viewport to `get_bounds()`, clears color (and depth/stencil if it has them) |
| `begin_window(window_size)`       | Binds the window framebuffer and sets the viewport to the whole window, without clearing      |
| `begin_window(window_size, color)`| Same, then clears the color                                                                    |

Blending and what is drawn are left to the pass. The sandbox (`applications/sandbox/source/main.cpp`) shows three passes: the scene into one framebuffer, a light map with additive blending into another, and both combined into the window.

## `point_light_t`

```cpp
struct point_light_t
{
    math::vec2_t position;   // pixels, same space as the scene projection
    f32 radius;              // pixels, default 250
    math::vec4_t color;      // default (1, 0.85, 0.6, 1)
    f32 intensity;           // default 1

    f32 get_diameter() const;
};
```

Plain data for a light that shines equally in every direction and fades out until its radius. It has no drawing code: the shader that draws it decides how the fade looks. `get_diameter()` is the side of the square quad that covers the light.

## `material_t`

A material is a surface function plus a shading model. The material file only decides the color of each pixel; the engine wraps it with a vertex shader, the inputs and the `main()` of the shading model, and compiles the result as a normal OpenGL program. Each material is its own program: there is no uber shader.

```cpp
class BF_RENDERING_API material_t
{
public:
    bool load(const char* material_surface_path, const e_material_shading_models material_shading_model);
    bool reload();
    void dispose();
    void bind() const;

    void set_projection(const math::mat4_t& projection) const;
    void set_model(const math::mat4_t& model) const;
    void set_time(f32 time_in_seconds) const;
    void set_light_map(const opengl_texture_t& light_map, u32 texture_slot) const;

    void set_float(const char* parameter_name, f32 value);
    void set_int(const char* parameter_name, i32 value);
    void set_vector_2(const char* parameter_name, const math::vec2_t& value);
    void set_vector_3(const char* parameter_name, const math::vec3_t& value);
    void set_vector_4(const char* parameter_name, const math::vec4_t& value);
    void set_texture(const char* parameter_name, const opengl_texture_t& texture, u32 texture_slot);
};
```

### The material file

The file declares its parameters as uniforms and defines `surface()`:

```glsl
uniform vec4 u_line_color;

void surface(in material_input_t surface_input, inout material_output_t surface_output)
{
    surface_output.base_color = u_line_color.rgb;
    surface_output.opacity    = surface_input.uv.x;
}
```

| `material_input_t` | Meaning                                   |
| ------------------ | ----------------------------------------- |
| `uv`               | Texture coordinate of the mesh            |
| `world_position`   | Position after `u_model`                  |
| `normal`           | Normal after `u_model`, normalized        |
| `time`             | Value of `set_time()`, in seconds         |

| `material_output_t` | Default      | Meaning                                    |
| ------------------- | ------------ | ------------------------------------------ |
| `base_color`        | `vec3(1.0)`  | Color that the shading model lights        |
| `opacity`           | `1.0`        | Alpha of the result                        |
| `emissive`          | `vec3(0.0)`  | Added after lighting, so it is never dark  |

`input` and `output` are reserved words in GLSL, so the parameters need other names. The file is inserted after a `#line 1`, so compile errors point to lines of the material file.

### Shading models

| `e_material_shading_models` | Final color                                                                    |
| --------------------------- | ------------------------------------------------------------------------------ |
| `Unlit`                     | `base_color + emissive`, with `opacity`                                        |
| `Lit`                       | `base_color * light + emissive`, where `light` is the light map sampled at the pixel's screen position (`set_light_map()`) |

`Lit` matches the light map the sandbox draws in its second pass: the material reads it directly instead of a separate combine pass.

### Uniforms and parameters

The vertex template uses `u_projection` and `u_model`; the fragment header uses `u_time`; `Lit` adds `u_light_map`. Their locations are looked up once in `reload()`. Parameter setters look up the location by name on the first call and cache it; an unknown name gives `-1`, which OpenGL ignores. Like the shader setters, they write into the bound program, so `bind()` first.

`reload()` reads the file again and keeps the old program if the new one fails, so a material can be edited while the application runs.

### Materials on sprites

`sprite_renderer_t::draw_with_material()` draws the sprite quad with a material instead of the sprite shader. Besides projection, model and time, it sets three parameters that the material can declare:

| Uniform         | Type        | Value                                  |
| --------------- | ----------- | -------------------------------------- |
| `u_texture`     | `sampler2D` | The sprite texture, on slot `0`        |
| `u_use_texture` | `bool`      | `true` when the sprite has a texture   |
| `u_tint`        | `vec4`      | The sprite tint                        |

Sprites with `is_lit` and no `material_path` use `get_default_lit_material()`, a `Lit` material built from `sprite_material_template` (texture times tint), so any sprite can receive light. `Lit` materials also get the light map on slot `1`, set by `set_material_frame()`; the `light_2D` scene system calls it after drawing the light map. Without lighting resources a `Lit` material reads an unbound texture, so use `Unlit` there. The editor's Content Browser "New Material" button writes a file that already uses these three uniforms.

## `material_cache_t`

Loads a material on first use and keeps it, like `texture_cache_t`. The key is the path plus the shading model, so the same file can be cached as `Unlit` and as `Lit`. A material that fails to load is kept as failed and returns `nullptr` until `invalidate(path)`, which removes both shading models of that path; the editor's "Reload Material" button calls it. `asset_manager_t` owns one. `reload_modified()` recompiles every cached material whose file changed on disk since it was loaded (failed ones included); the editor calls it about twice a second, so saving a material file updates the viewport.

## `point_light_renderer_t`

Draws `point_light_t` values into the bound target with its own embedded shader: `begin(projection)` enables additive blending, and each `draw(light)` draws a square quad of the light's diameter whose pixels fade linearly from the center to the radius. The scene's `light_2D` system uses it on the light map; the sandbox keeps its own light shader for study.

## `opengl_window_context_t`

```cpp
struct BF_RENDERING_API opengl_window_context_t
{
public:
    u32 window_id;
    SDL_Window* sdl_window;
    SDL_GLContextState* gl_context;

    static void set_context_attributes();
    static opengl_function_loader_t get_function_loader();

    bool create(const platform::window_t& window, const opengl_window_context_t* shared_context);
    bool make_current() const;
    bool set_vsync(bool enabled) const;
    void swap_buffers() const;
    void destroy();
};
```

### State

| Field        | Meaning                                                 |
| ------------ | ------------------------------------------------------- |
| `window_id`  | SDL id of the window this context draws to, `0` if none |
| `sdl_window` | The SDL window                                          |
| `gl_context` | The SDL OpenGL context, `nullptr` if not created        |

### `set_context_attributes()`

Sets the SDL attributes for the next contexts: core profile, major version `4`, minor version `6`. SDL reads these when a context is created, so they apply to every context created afterwards.

### `get_function_loader()`

Returns `SDL_GL_GetProcAddress` as an `opengl_function_loader_t`, for `opengl_functions_t::load()`.

### `create(window, shared_context)`

1. Stores the window id and SDL window.
2. If `shared_context` is not null, makes it current and sets `SDL_GL_SHARE_WITH_CURRENT_CONTEXT` to `1`.
3. Creates the context with `SDL_GL_CreateContext`, then sets the share attribute back to `0`.
4. Returns `true` if the context exists and could be made current.

### `make_current()`, `set_vsync()`, `swap_buffers()`

`make_current()` calls `SDL_GL_MakeCurrent` and returns `false` if there is no context. `set_vsync(enabled)` makes the context current and sets the swap interval to `1` or `0`. `swap_buffers()` calls `SDL_GL_SwapWindow`.

### `destroy()`

Destroys the SDL context if there is one, then clears all three fields. Safe to call twice.

## `opengl_shader_t`

```cpp
struct BF_RENDERING_API opengl_shader_t
{
public:
    u32 program_id;

    bool create(const char* vertex_shader_path, const char* fragment_shader_path);
    bool create_from_source(const char* vertex_shader_source, const char* fragment_shader_source);
    void bind() const;

    i32 get_uniform_location(const char* name) const;
    void set_matrix_4(i32 uniform_location, const math::mat4_t& matrix) const;
    void set_vector_2(i32 uniform_location, const math::vec2_t& vector) const;
    void set_vector_3(i32 uniform_location, const math::vec3_t& vector) const;
    void set_vector_4(i32 uniform_location, const math::vec4_t& vector) const;
    void set_float(i32 uniform_location, f32 value) const;
    void set_int(i32 uniform_location, i32 value) const;
    void set_bool(i32 uniform_location, bool value) const;

    void dispose();

private:
    bool link(u32 vertex_id, u32 fragment_id, const char* vertex_label, const char* fragment_label);
};
```

### State

| Field        | Meaning                                         |
| ------------ | ----------------------------------------------- |
| `program_id` | OpenGL program id, `0` if not created or failed |

### `create(vertex_path, fragment_path)`

Reads each file as text with `io::file_t`, compiles it and links the two stages. If a file can't be read, it logs `Could not read shader file '<path>'.`. If a stage doesn't compile, it logs `Could not compile shader '<path>': <info log>`. Both stages are always compiled before linking is attempted, so both errors can appear.

A relative path is relative to the process working directory.

### `create_from_source(vertex_source, fragment_source)`

Same as `create()`, but compiles the strings directly. Errors use `<embedded>` as the label.

### Linking

`link()` returns `false` without creating a program if either stage failed, after deleting the stage that did compile. Otherwise it creates the program, attaches and links both stages, and deletes the stage objects, which are no longer needed. If linking fails, it logs `Could not link shader program ('<vertex>', '<fragment>'): <info log>`, deletes the program and returns `false`.

The info log buffer is 1024 characters. Longer messages are cut.

### `bind()` and uniform setters

`bind()` calls `glUseProgram`. `get_uniform_location(name)` returns OpenGL's location, which is `-1` for a name that doesn't exist or was optimized out.

The setters call `glUniform*` with the location, so they write into the **currently bound** program, not necessarily this one. Bind the shader first. A location of `-1` is ignored by OpenGL.

| Setter         | OpenGL call                                    |
| -------------- | ---------------------------------------------- |
| `set_matrix_4` | `glUniformMatrix4fv`, one matrix, no transpose |
| `set_vector_2` | `glUniform2f`                                  |
| `set_vector_3` | `glUniform3f`                                  |
| `set_vector_4` | `glUniform4f`                                  |
| `set_float`    | `glUniform1f`                                  |
| `set_int`      | `glUniform1i`                                  |
| `set_bool`     | `glUniform1i` with `1` or `0`                  |

`set_matrix_4` passes `values` as is, which works because `mat4_t` is already column-major.

### `dispose()`

Deletes the program and sets `program_id` to `0`.

## `opengl_mesh_t`

```cpp
struct BF_RENDERING_API opengl_mesh_t
{
public:
    u32 vertex_array_id;
    u32 vertex_buffer_id;
    u32 index_buffer_id;
    u32 index_count;

    bool create(const f32* vertices, usize vertex_float_count,
                const u32* indices, usize index_count,
                const u32* attribute_sizes, usize attribute_count);
    bool create_quad();
    void draw() const;
    void dispose();
};
```

### State

| Field              | Meaning                             |
| ------------------ | ----------------------------------- |
| `vertex_array_id`  | The vertex array object (VAO)       |
| `vertex_buffer_id` | The vertex buffer (VBO)             |
| `index_buffer_id`  | The index buffer (EBO)              |
| `index_count`      | Number of indices drawn by `draw()` |

### `create(...)`

Builds an indexed mesh from interleaved float vertices.

1. Adds up `attribute_sizes` to get the stride in floats. For sizes `{ 3, 2 }` each vertex is 5 floats (20 bytes).
2. Generates the VAO and both buffers, binds the VAO, and uploads vertices and indices with `GL_STATIC_DRAW`.
3. For each attribute `i`, sets `glVertexAttribPointer(i, size, GL_FLOAT, ...)` at the running offset and enables it. Attribute `i` is shader location `i`.
4. Unbinds the VAO, stores `index_count` and returns `true`.

`vertex_float_count` is the number of floats, not vertices. There is no failure path: the result is always `true`.

### `create_quad()`

Creates the unit quad used by the sprite renderer: four vertices from `-0.5` to `0.5`, centered on the origin, each with a position (3 floats) and a texture coordinate (2 floats).

| Vertex | Position       | UV       |
| ------ | -------------- | -------- |
| 0      | `(-0.5,  0.5)` | `(0, 1)` |
| 1      | `( 0.5,  0.5)` | `(1, 1)` |
| 2      | `( 0.5, -0.5)` | `(1, 0)` |
| 3      | `(-0.5, -0.5)` | `(0, 0)` |

Indices `0, 1, 2, 2, 3, 0` make two triangles. With the Y-down projection, `y = -0.5` is the top edge on screen and has `v = 0`, which matches images whose first row is the top one.

### `draw()` and `dispose()`

`draw()` binds the VAO, draws `index_count` indices as `GL_TRIANGLES` with `GL_UNSIGNED_INT` indices, and unbinds it. `dispose()` deletes the VAO and both buffers and zeroes all fields.

## `opengl_texture_t`

```cpp
struct BF_RENDERING_API opengl_texture_t
{
public:
    u32 texture_id;
    math::vec2i_t size;

    bool load(const char* path);
    bool upload(const u8* rgba_pixels, const math::vec2i_t& pixel_size);
    void bind(u32 slot) const;
    void dispose();
};
```

### State

| Field        | Meaning                                       |
| ------------ | --------------------------------------------- |
| `texture_id` | OpenGL texture id, `0` if there is no texture |
| `size`       | Width and height in pixels                    |

### `load(path)`

Loads the file with `image::image_t` (always 4 channels) and passes the pixels to `upload()`. If the image can't be loaded it returns `false`; the image module has already logged the reason. The decoded pixels are freed when the function returns.

### `upload(rgba_pixels, pixel_size)`

1. Generates a texture id if there is none yet. An existing id is reused, so uploading again replaces the content.
2. Stores the size and uploads the pixels as `GL_RGBA` with `GL_UNSIGNED_BYTE` components.
3. Sets linear filtering for minification and magnification, and `GL_CLAMP_TO_EDGE` on both axes.

No mipmaps are generated. The function always returns `true`.

### `bind(slot)` and `dispose()`

`bind(slot)` activates texture unit `GL_TEXTURE0 + slot` and binds the texture there. `dispose()` deletes the texture and sets `texture_id` to `0`. `size` is left as it was.

## `font_t`

```cpp
constexpr u32 FONT_FIRST_CHARACTER = 32;
constexpr u32 FONT_CHARACTER_COUNT = 96;

struct font_glyph_t
{
    u16 x0, y0, x1, y1;
    f32 x_offset, y_offset, x_advance;
};

struct BF_RENDERING_API font_t
{
public:
    opengl_texture_t atlas;
    font_glyph_t glyphs[FONT_CHARACTER_COUNT];
    f32 size;

    bool load(const char* path, const f32 font_size);
    void dispose();
};
```

A font is one TrueType file baked at one pixel size into a texture atlas. It covers the 96 characters from code 32 (space) to 127.

### State

| Field    | Meaning                                                           |
| -------- | ----------------------------------------------------------------- |
| `atlas`  | 512x512 RGBA texture that holds every glyph                       |
| `glyphs` | One entry per character, indexed by `code - FONT_FIRST_CHARACTER` |
| `size`   | Pixel height the font was baked at, `0` if not loaded             |

Each `font_glyph_t` holds:

| Field                | Meaning                                                        |
| -------------------- | -------------------------------------------------------------- |
| `x0, y0, x1, y1`     | The glyph's rectangle in the atlas, in pixels                  |
| `x_offset, y_offset` | Where the rectangle starts relative to the pen on the baseline |
| `x_advance`          | How far the pen moves right after this glyph                   |

### `load(path, font_size)`

1. Disposes the previous atlas.
2. Reads the whole file as binary. On failure logs `Could not read font '<path>'.` and returns `false`.
3. Calls `stbtt_BakeFontBitmap` for the first font in the file, at `font_size` pixels, into a 512x512 one-channel coverage bitmap. If the result is `0` or negative (not every character fit), it logs `Could not fit font '<path>' at size <size> into the atlas.` and returns `false`.
4. Converts the coverage bitmap to RGBA: white color, coverage as alpha. Tinting the white glyph in the shader gives the text color.
5. Copies the baked rectangles, offsets and advances into `glyphs`, stores the size and uploads the atlas.

`stb_truetype` is compiled into the engine by `font.cpp`, which defines `STB_TRUETYPE_IMPLEMENTATION`.

### `dispose()`

Disposes the atlas texture and sets `size` to `0`. `glyphs` is left as it was.

## `sprite_renderer_t`

```cpp
class BF_RENDERING_API sprite_renderer_t
{
public:
    bool initialize(const char* vertex_shader_path, const char* fragment_shader_path);
    void begin(const math::vec2i_t& viewport_size, const math::vec2_t& camera_position, const f32 zoom, const bool use_camera);
    void draw(const opengl_texture_t* texture, const math::vec2_t& position, const math::vec2_t& size, f32 rotation_degrees, const math::vec4_t& tint);
    void draw_region(const opengl_texture_t* texture, const math::vec2_t& position, const math::vec2_t& size, f32 rotation_degrees, const math::vec4_t& tint, const math::vec4_t& uv_rect);
    void draw_text(const font_t& font, const char* text, const math::vec2_t& position, const math::vec4_t& color);
    void dispose();

private:
    opengl_shader_t shader;
    opengl_mesh_t quad;
    i32 projection_location;
    i32 model_location;
    i32 tint_location;
    i32 use_texture_location;
    i32 uv_rect_location;
};
```

### State

| Field        | Meaning                                             |
| ------------ | --------------------------------------------------- |
| `shader`     | The sprite shader program                           |
| `quad`       | The unit quad from `opengl_mesh_t::create_quad()`   |
| `*_location` | Cached uniform locations, `-1` until `initialize()` |

### The shader contract

The renderer does not ship its own shaders. The caller passes two files, and they must declare these names:

| Name                | Stage    | Type        | Set by                                            |
| ------------------- | -------- | ----------- | ------------------------------------------------- |
| location `0`        | vertex   | `vec3`      | Quad position                                     |
| location `1`        | vertex   | `vec2`      | Quad texture coordinate                           |
| `projection_matrix` | vertex   | `mat4`      | `begin()`                                         |
| `model_matrix`      | vertex   | `mat4`      | Every draw                                        |
| `uv_rect`           | vertex   | `vec4`      | Every draw: `(u, v, width, height)` of the region |
| `tint`              | fragment | `vec4`      | Every draw                                        |
| `use_texture`       | fragment | `bool`      | Every draw: `true` when a texture was given       |
| `in_texture`        | fragment | `sampler2D` | Set to unit `0` once in `initialize()`            |

The editor's shaders do exactly this:

```glsl
// sprite.vert
texture_coordinates = uv_rect.xy + in_texture_coordinates * uv_rect.zw;
gl_Position = projection_matrix * model_matrix * vec4(position, 1.0);

// sprite.frag
out_color = use_texture ? texture(in_texture, texture_coordinates) * tint : tint;
```

### `initialize(vertex_path, fragment_path)`

Creates the shader from the two files and the unit quad. Returns `false` if the shader fails (the shader already logged why). Then it looks up the five uniform locations, binds the shader and sets `in_texture` to texture unit `0`. A context must be current.

### `begin(viewport_size, camera_position, zoom, use_camera)`

1. Enables blending with `GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA` (standard alpha blending).
2. Picks the visible area:
   - Without a camera, or with `zoom <= 0`, it is `left = 0`, `right = width`, `top = 0`, `bottom = height`. One unit is one pixel and the origin is the top-left corner.
   - With a camera, the area is centered on `camera_position` and its half size is `viewport / 2 / zoom`. A zoom of `2` shows half as much, so everything looks twice as big.
3. Binds the shader and uploads `mat4_t::orthographic(left, right, bottom, top, -1, 1)`.

`begin()` does not set the viewport; it only uses `viewport_size` for the projection.

### `draw(texture, position, size, rotation_degrees, tint)`

Calls `draw_region()` with the full UV rectangle `(0, 0, 1, 1)`.

### `draw_region(texture, position, size, rotation_degrees, tint, uv_rect)`

1. Builds the model matrix `identity().translate(position).rotate_z(rotation).scale(size)`. The unit quad becomes a `size` rectangle centered on `position`, rotated around its center.
2. Uploads `model_matrix`, `tint`, `uv_rect`, and `use_texture = texture != nullptr`.
3. If there is a texture, binds it to unit `0`.
4. Draws the quad.

Passing `nullptr` as the texture draws a solid rectangle in the tint color. `uv_rect` is `(u, v, width, height)` in normalized coordinates, with `v = 0` at the top of the image.

### `draw_text(font, text, position, color)`

Lays out one line of text and draws one quad per glyph.

1. The pen starts at `position.x`. The baseline is at `position.y + font.size`, so `position` is roughly the top-left of the line.
2. For each byte of `text`, characters outside 32 to 127 are skipped without moving the pen.
3. For a glyph with a non-empty rectangle, it computes the center of the glyph box (`pen + offset + half size`) and its UV rectangle in the atlas, then calls `draw_region()` with the atlas, no rotation and `color` as the tint.
4. The pen moves right by `x_advance`, also for glyphs with an empty rectangle such as space.

There is no kerning, no line breaking and no handling of `\n`. Text is never rotated or scaled.

### `dispose()`

Disposes the quad and the shader.

## `texture_cache_t`

```cpp
class BF_RENDERING_API texture_cache_t
{
public:
    const opengl_texture_t* get(const char* path);
    usize get_count() const;
    usize get_failed_count() const;
    void invalidate(const char* path);
    void dispose();

private:
    std::unordered_map<std::string, opengl_texture_t> textures;
};
```

### State

| Field      | Meaning                                                                          |
| ---------- | -------------------------------------------------------------------------------- |
| `textures` | Every path ever requested, with its texture. Failed loads have `texture_id == 0` |

### `get(path)`

1. Returns `nullptr` for a null or empty path.
2. If the path is not in the map, loads it with `opengl_texture_t::load()` and stores the result **even if loading failed**.
3. Returns a pointer to the stored texture, or `nullptr` if its id is `0`.

A path that failed once is not retried on later calls; it keeps returning `nullptr` until it is invalidated. The key is the path string exactly as given, so `a.png` and `./a.png` are two entries. The returned pointer stays valid until that entry is removed.

### `get_count()` and `get_failed_count()`

`get_count()` is the number of entries, including failed ones. `get_failed_count()` counts the entries with `texture_id == 0`. The editor's status panel shows both.

### `invalidate(path)`

Disposes the texture for that path and removes the entry, so the next `get()` loads the file again. An unknown path does nothing.

### `dispose()`

Disposes every texture and clears the map.

## `font_cache_t`

```cpp
class BF_RENDERING_API font_cache_t
{
public:
    const font_t* get(const char* path, const f32 size);
    usize get_count() const;
    usize get_failed_count() const;
    void dispose();

private:
    std::unordered_map<std::string, font_t*> fonts;
};
```

### State

| Field   | Meaning                                                                      |
| ------- | ---------------------------------------------------------------------------- |
| `fonts` | Heap-allocated fonts keyed by `"<path>@<size>"`, with size printed as `%.1f` |

### `get(path, size)`

1. Returns `nullptr` for a null or empty path, or a size `<= 0`.
2. Builds the key with `"%s@%.1f"`, for example `assets/fonts/arial.ttf@32.0`, in a 300 character buffer.
3. If the key is new, allocates a `font_t`, loads it and stores it **even if loading failed**.
4. Returns the font, or `nullptr` if its atlas id is `0`.

The same file at two sizes gives two entries and two atlases. Sizes that print the same with one decimal share an entry, which was baked with the size requested first. A failed font is never retried; `font_cache_t` has no `invalidate()`.

### `get_count()`, `get_failed_count()` and `dispose()`

They work like the texture cache: entry count, entries whose atlas id is `0`, and dispose plus `delete` of every font followed by clearing the map.

## Usage

`runtime_t` owns the `renderer_t` and drives the window side:

- `runtime_t::initialize()` calls `renderer.initialize()` after platform, scripting and audio succeed.
- `runtime_t::create_window()` creates the SDL window and calls `attach_window()`. If that fails, the window is destroyed and `nullptr` is returned.
- `runtime_t::destroy_window()` calls `detach_window()` before destroying the window.
- `runtime_t::dispose()` disposes the renderer after audio and scripting, before the platform.

The editor (`editor_t`) owns the `sprite_renderer_t`, `texture_cache_t` and `font_cache_t`, and passes the caches to its UI through `editor_context_t`. During `initialize()` it calls `runtime.renderer.begin_frame(*window)` right after creating the window, which makes the window's context current before `sprite_renderer.initialize()` creates the shader and the quad.

Every editor frame:

```cpp
if (runtime.renderer.begin_frame(*window))
{
    scene.render(sprite_renderer, texture_cache, font_cache, window->get_size());

    debug_ui_context.begin_frame();
    draw_ui();
    debug_ui_context.end_frame();

    runtime.renderer.end_frame(*window);
}
```

`scene_t::render()` finds the first camera with `is_primary` set and passes its position and zoom to `begin()`; without one it uses pixel coordinates. Then it draws every sprite, with `size * transform.scale` and `transform.rotation`, followed by every label. Textures and fonts are requested from the caches during this loop, so they load on the frame they are first needed. A sprite with an empty `texture_path` is drawn as a solid tinted rectangle. A label whose font fails is skipped.

On shutdown the editor disposes the font cache, the texture cache and the sprite renderer before `runtime.dispose()` destroys the contexts.

## Using the OpenGL layer without the platform module

`rendering/opengl/` only needs a current context and the functions loaded. With GLFW, for example:

```cpp
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

GLFWwindow* window = glfwCreateWindow(1280, 720, "Demo", nullptr, nullptr);
glfwMakeContextCurrent(window);

rendering::opengl_functions_t::load(reinterpret_cast<rendering::opengl_function_loader_t>(glfwGetProcAddress));

rendering::opengl_framebuffer_t framebuffer;
rendering::opengl_shader_t shader;
rendering::opengl_mesh_t quad;
// ... same code as the sandbox, with glfwSwapBuffers(window) at the end of each frame
```

With SDL, without `platform_t`, the loader is `SDL_GL_GetProcAddress`. `renderer_t` and `opengl_window_context_t` are not used in either case. Today all folders are still built into the one `byte-forge-engine` library; the rule that `rendering/opengl/` includes nothing from `platform`, `ecs` or `scene` is what keeps a later split possible.

## Notes

- `runtime_t::render()` begins and ends a frame for every window with nothing in between, so it only clears and swaps. The editor does not call it and frames its own window instead.
- Each quad and each glyph is a separate draw call with its own uniform uploads. There is no batching or sorting: sprites are drawn in the order of the sprite system's entity list, and all labels come after all sprites.
- Textures use linear filtering without mipmaps, so scaled pixel art looks blurred and strongly minified textures can shimmer.
- Calling `create()` again on a shader or mesh that already holds OpenGL objects does not free the old ones; their ids are overwritten. A texture is different: `upload()` reuses its id, so loading again replaces the content.
