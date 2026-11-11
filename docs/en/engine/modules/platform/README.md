# Platform module

> Language: **English** | [Português (Brasil)](../../../../pt-br/engine/modules/platform/README.md)

The platform module is the engine's link to the operating system. It starts SDL3, creates and destroys windows, reads the OS event queue and turns SDL events into the engine's own window and input events. It also keeps a per-frame keyboard and mouse state that the editor and Lua scripts query. Everything lives in the `byteforge::platform` namespace and is split into two areas: `display/window` (windows and window events) and `input` (key codes, mouse buttons, input events and input state).

The module is not header-only. Every header that touches SDL only forward-declares SDL types, and the SDL headers are included in the `.cpp` files.

## Files

| File                                                                                    | Contents                                                     |
| --------------------------------------------------------------------------------------- | ------------------------------------------------------------ |
| `engine/include/byte-forge-engine/modules/platform/platform.h`                          | `platform_t` declaration                                     |
| `engine/include/byte-forge-engine/modules/platform/native_event_callback.h`             | `native_event_callback_t`, wrapper for raw SDL events        |
| `engine/include/byte-forge-engine/modules/platform/display/display_descriptor.h`        | `display_descriptor_t`, the chosen graphics backend          |
| `engine/include/byte-forge-engine/modules/platform/display/window/window.h`             | `window_t` declaration                                       |
| `engine/include/byte-forge-engine/modules/platform/display/window/window_descriptor.h`  | `window_descriptor_t` and `WINDOW_TITLE_MAX_LENGTH`          |
| `engine/include/byte-forge-engine/modules/platform/display/window/e_window_flags.h`     | `e_window_flags` and its bit operators                       |
| `engine/include/byte-forge-engine/modules/platform/display/window/window_manager.h`     | `window_manager_t` declaration                               |
| `engine/include/byte-forge-engine/modules/platform/display/window/window_event.h`       | `window_event_t`                                             |
| `engine/include/byte-forge-engine/modules/platform/display/window/e_window_event_types.h` | `e_window_event_types`                                      |
| `engine/include/byte-forge-engine/modules/platform/display/window/window_event_callback.h` | `window_event_callback_t`                                 |
| `engine/include/byte-forge-engine/modules/platform/display/window/window_event_parser.h` | `window_event_parser_t` declaration                         |
| `engine/include/byte-forge-engine/modules/platform/input/e_key_codes.h`                 | `e_key_codes` and `KEYCODES_TABLE_SIZE`                      |
| `engine/include/byte-forge-engine/modules/platform/input/e_mouse_buttons.h`             | `e_mouse_buttons` and `MOUSE_BUTTONS_TABLE_SIZE`             |
| `engine/include/byte-forge-engine/modules/platform/input/e_input_event_types.h`         | `e_input_event_types`                                        |
| `engine/include/byte-forge-engine/modules/platform/input/e_input_states.h`              | `e_input_states`                                             |
| `engine/include/byte-forge-engine/modules/platform/input/input_event.h`                 | `input_event_t`                                              |
| `engine/include/byte-forge-engine/modules/platform/input/input_event_callback.h`        | `input_event_callback_t`                                     |
| `engine/include/byte-forge-engine/modules/platform/input/input_event_parser.h`          | `input_event_parser_t` declaration                           |
| `engine/include/byte-forge-engine/modules/platform/input/input_names.h`                 | `input_names_t` declaration                                  |
| `engine/include/byte-forge-engine/modules/platform/input/input_state.h`                 | `input_state_t` declaration                                  |
| `engine/source/modules/platform/platform.cpp`                                           | SDL start and shutdown, the event loop                       |
| `engine/source/modules/platform/display/window/window.cpp`                              | `window_t` methods                                           |
| `engine/source/modules/platform/display/window/window_manager.cpp`                      | Window creation, lookup and destruction, SDL flag mapping    |
| `engine/source/modules/platform/display/window/window_event_parser.cpp`                 | SDL window events to `window_event_t`                        |
| `engine/source/modules/platform/input/input_event_parser.cpp`                           | Scancode and mouse button tables, SDL input events to `input_event_t` |
| `engine/source/modules/platform/input/input_names.cpp`                                  | Name tables for keys and mouse buttons                       |
| `engine/source/modules/platform/input/input_state.cpp`                                  | `input_state_t` methods                                      |

Related pieces in other modules:

| File                                                          | Role                                                         |
| ------------------------------------------------------------- | ------------------------------------------------------------ |
| `modules/runtime/runtime.h` and `runtime.cpp`                 | Own `platform_t`, the `should_close` flag and window creation |
| `modules/rendering/window/renderer.cpp`                       | Creates a GL context per window, applies `use_vsync`         |
| `modules/rendering/window/opengl_window_context.cpp`          | Uses `window_t::sdl_window` to create the GL context         |
| `modules/scene/scene_player.cpp`                              | Enables, disables and clears the input state                 |
| `modules/scene/scene_script_system.cpp`                       | Lua `input` table                                            |
| `applications/editor/source/editor.cpp`                       | Owns the `input_state_t`, runs the event loop                |
| `applications/editor/source/ui/debug_ui_context.cpp`          | Receives raw SDL events for ImGui                            |

## Concepts

### How SDL is hidden

Headers never include SDL. `window.h` forward-declares `struct SDL_Window;`, and the two parsers forward-declare `union SDL_Event;`. The raw event callback takes a `const void*` instead of an `SDL_Event`. Code that includes the platform headers does not need SDL on its include path, except code that wants to read the raw event, which casts the pointer back to `SDL_Event` itself (the editor's debug UI does this).

`window_t` exposes its `SDL_Window*` and SDL window id as public fields, because the renderer and the debug UI need them to create GL contexts and initialize ImGui.

### Event flow

One call to `platform_t::poll_events()` drains the SDL queue. Each SDL event goes through three steps, in this order:

```
SDL_PollEvent
  └─► native_event_callback(&sdl_event)          every event, untranslated
  └─► SDL_EVENT_QUIT?  request_close(), return
  └─► window_event_parser_t::parse ─► window_event_callback
  └─► input_event_parser_t::parse  ─► input_event_callback
```

The platform does not store any event. It only translates and forwards them. Keeping state (which keys are down, the mouse position) is the job of `input_state_t`, which the owner feeds from the input callback.

### Window ids

A window is identified by the `u32` id SDL assigns to it (`SDL_GetWindowID`). Window events and input events carry the id of the window they belong to in `window_id`. `window_manager_t::get_window()` and `destroy_window()` take this id, and the renderer keys its GL contexts by it.

### Key codes are physical keys

The input parser reads `sdl_event.key.scancode`, not the key symbol. SDL scancodes describe the physical position of a key, named after the US layout. So `e_key_codes::A` means the key in the position of A on a US keyboard, whatever character the user's layout prints on it.

### Pressed, down and released

`input_state_t` keeps three views of each key and mouse button:

| View       | True when                                                       |
| ---------- | --------------------------------------------------------------- |
| `down`     | The key went down and has not gone up yet                       |
| `pressed`  | The key went down during the current frame                      |
| `released` | The key went up during the current frame                        |

`pressed`, `released` and the wheel delta are reset by `begin_frame()`, so they describe only the events polled since the last `begin_frame()`.

## `platform_t`

```cpp
class BF_PLATFORM_API platform_t
{
public:
    display_descriptor_t    display_descriptor;
    window_manager_t        window_manager;
    bool*                   should_close_handler;
    native_event_callback_t native_event_callback;

    platform_t(display_descriptor_t display_descriptor, bool* should_close_handler);

    bool initialize();
    void request_close();
    void poll_events(const input_event_callback_t& input_event_callback, const window_event_callback_t& window_event_callback);
    void dispose();

private:
    bool initialized;
};
```

There is no destructor. SDL is shut down and the windows are freed only in `dispose()`.

### State

| Field                   | Meaning                                                              |
| ----------------------- | -------------------------------------------------------------------- |
| `display_descriptor`    | Graphics backend, passed to the window manager on every window creation |
| `window_manager`        | Owns every window                                                    |
| `should_close_handler`  | Pointer to a flag owned by someone else. `request_close()` sets it to `true` |
| `native_event_callback` | Called with every raw SDL event. Empty by default                    |
| `initialized`           | `true` between a successful `initialize()` and `dispose()`           |

### `initialize()`

Calls `SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS)`. On failure it logs `Could not initialize SDL: <error>` at fatal level, with SDL's error text, and returns `false`. On success it sets `initialized` and returns `true`.

### `request_close()`

Writes `true` through `should_close_handler`, if the pointer is not null. It does not close any window or stop SDL; it only raises the flag, and the owner's loop decides what to do with it.

### `poll_events(input_event_callback, window_event_callback)`

1. Calls `SDL_PollEvent` until the queue is empty.
2. For each event, calls `native_event_callback` with a pointer to the `SDL_Event`.
3. If the event is `SDL_EVENT_QUIT`, calls `request_close()` and returns immediately. Events still in the queue stay there for the next call.
4. Builds a `window_event_t`. If `window_event_parser_t::parse()` accepts the event, calls `window_event_callback`.
5. Builds an `input_event_t`. If `input_event_parser_t::parse()` accepts the event, calls `input_event_callback`.

Both parsers see every event. An SDL event is translated by at most one of them in practice, since they handle different SDL event types.

### `dispose()`

1. Calls `window_manager.dispose()`, which destroys every window. This happens even if `initialize()` was never called or failed.
2. If the platform was initialized, quits the events and video subsystems, then calls `SDL_Quit()`, and clears `initialized`.

Calling it twice is safe: the window list is already empty and `initialized` is `false`.

## `display_descriptor_t`

```cpp
struct display_descriptor_t
{
public:
    graphics::e_graphic_backends graphic_backend;

    explicit display_descriptor_t(const graphics::e_graphic_backends graphic_backend);
};
```

Describes the display side of the platform. Today it holds only the graphics backend. The window manager reads it when it builds SDL window flags: `OpenGL` adds `SDL_WINDOW_OPENGL`, and every other value of `e_graphic_backends` adds nothing.

## `window_descriptor_t`

```cpp
constexpr usize WINDOW_TITLE_MAX_LENGTH = 256;

struct window_descriptor_t
{
public:
    char           title[WINDOW_TITLE_MAX_LENGTH];
    math::recti_t  bounds;
    e_window_flags flags;
    bool           use_vsync;

    window_descriptor_t(const char* window_title, const math::recti_t& window_bounds, const e_window_flags window_flags, bool use_vsync);
};
```

The settings a window is created with. A copy is kept inside the `window_t`.

### State

| Field       | Meaning                                                                 |
| ----------- | ----------------------------------------------------------------------- |
| `title`     | Window title. Copied with `snprintf`, so it is cut at 255 characters plus the terminator |
| `bounds`    | Position and size. The size becomes the window size; the position is applied only when it is not `(0, 0)` |
| `flags`     | `e_window_flags` to apply                                               |
| `use_vsync` | Not read by the platform. The renderer reads it when it attaches a GL context to the window |

## `e_window_flags`

```cpp
enum class e_window_flags : u8
{
    None       = 0,
    Resizable  = (1 << 0),
    Borderless = (1 << 1),
    Fullscreen = (1 << 2),
    Hidden     = (1 << 3),
    Maximized  = (1 << 4),
    Minimized  = (1 << 5),
};

constexpr e_window_flags  operator| (e_window_flags left_flag, e_window_flags right_flag);
constexpr e_window_flags  operator& (e_window_flags left_flag, e_window_flags right_flag);
constexpr e_window_flags& operator|=(e_window_flags& left_flag, e_window_flags right_flag);
constexpr bool            has_flag(e_window_flags flags, e_window_flags flag);
```

A bit set. The operators combine and mask flags without casts at the call site, and `has_flag()` returns `true` when every bit of `flag` is set in `flags`. Each flag maps to the SDL flag of the same name (`SDL_WINDOW_RESIZABLE`, `SDL_WINDOW_BORDERLESS` and so on). `SDL_WINDOW_HIGH_PIXEL_DENSITY` is always added, whatever the flags.

## `window_t`

```cpp
struct BF_PLATFORM_API window_t
{
public:
    SDL_Window*         sdl_window;
    u32                 sdl_window_id;
    window_descriptor_t descriptor;

    window_t(const window_descriptor_t& descriptor, SDL_Window* sdl_window, u32 sdl_window_id);

    void          set_title(const char* new_title) const;
    bool          is_valid() const;
    math::vec2i_t get_size() const;
    void          destroy();
};
```

One OS window. It is created and deleted by `window_manager_t`; other code receives a `window_t*` or a reference to it.

### State

| Field           | Meaning                                                         |
| --------------- | --------------------------------------------------------------- |
| `sdl_window`    | The SDL window. `nullptr` after `destroy()`                     |
| `sdl_window_id` | SDL's id for the window. `0` after `destroy()`                  |
| `descriptor`    | Copy of the descriptor the window was created with              |

### `set_title(new_title)`

Calls `SDL_SetWindowTitle`. The new title is not copied into `descriptor.title`, which keeps the original one.

### `is_valid()`

`true` when `sdl_window` is not null and `sdl_window_id` is not `0`.

### `get_size()`

Returns the drawable size in pixels from `SDL_GetWindowSizeInPixels`. Because windows are created with `SDL_WINDOW_HIGH_PIXEL_DENSITY`, on a scaled display this can be larger than the size given in the descriptor. If SDL fails, it returns `(0, 0)`.

### `destroy()`

Destroys the SDL window if there is one, sets `sdl_window` to `nullptr` and `sdl_window_id` to `0`. Calling it again does nothing. It does not free the `window_t` itself; the window manager does that.

## `window_manager_t`

```cpp
class BF_PLATFORM_API window_manager_t
{
public:
    window_t* create_window(const window_descriptor_t& descriptor, const display_descriptor_t& display_descriptor);
    window_t* get_window(const u32 window_id);
    window_t* get_window_at(const usize index);
    usize     get_window_count() const;
    void      destroy_window(const u32 window_id);
    void      dispose();

private:
    std::vector<window_t*> windows;
};
```

Owns every window. Each `window_t` is allocated with `new`, so its address stays the same while other windows are added or removed.

### State

| Field     | Meaning                                         |
| --------- | ----------------------------------------------- |
| `windows` | Every live window, in creation order            |

### `create_window(descriptor, display_descriptor)`

1. Returns `nullptr` if `descriptor.bounds` is empty (width or height not positive).
2. Builds the SDL flags: `SDL_WINDOW_HIGH_PIXEL_DENSITY`, one SDL flag per `e_window_flags` bit, and `SDL_WINDOW_OPENGL` when the backend is `OpenGL`.
3. Calls `SDL_CreateWindow` with the title, width and height. Returns `nullptr` if SDL fails.
4. If `descriptor.bounds.position` is not `(0, 0)`, moves the window there with `SDL_SetWindowPosition`. Otherwise the position is left to SDL.
5. Allocates a `window_t` with the descriptor, the SDL window and its id, adds it to `windows` and returns it.

Neither failure is logged here. `runtime_t::create_window()` wraps this call and logs nothing either; the caller (the editor) logs `Could not create the editor window.`.

### Lookup

| Member               | Returns                                                         |
| -------------------- | --------------------------------------------------------------- |
| `get_window(id)`     | The window with that SDL id, or `nullptr`. Linear search        |
| `get_window_at(i)`   | The window at position `i` in creation order, or `nullptr` if out of range |
| `get_window_count()` | Number of live windows                                          |

### `destroy_window(window_id)`

Finds the window with that id, calls `destroy()`, deletes it and removes it from the list. An unknown id does nothing. Any `window_t*` kept elsewhere for that window is left dangling, so callers must drop it.

### `dispose()`

Destroys and deletes every window, then clears the list. Calling it twice is safe.

## `window_event_t`

```cpp
enum class e_window_event_types : u8
{
    None,
    Resize,
    CloseRequested
};

struct window_event_t
{
public:
    e_window_event_types type;
    u32                  window_id;

    union
    {
        struct
        {
            math::vec2i_t new_size;
        } resize;
    };
};
```

A window event. `type` says which union member is valid; only `Resize` has data. The default constructor sets `type` to `None`, `window_id` to `0` and zeroes `resize`.

## `window_event_parser_t`

```cpp
class BF_PLATFORM_API window_event_parser_t
{
public:
    static bool parse(const SDL_Event& sdl_event, window_event_t& event);
};
```

### `parse(sdl_event, event)`

1. Copies `sdl_event.window.windowID` into `event.window_id`. This is done before checking the type, so the field is written even when the function then returns `false`.
2. `SDL_EVENT_WINDOW_RESIZED`: sets `type` to `Resize`, asks SDL for the current size in pixels of that window (`SDL_GetWindowSizeInPixels`) and stores it in `resize.new_size`. The size in the SDL event itself is not used. Returns `true`.
3. `SDL_EVENT_WINDOW_CLOSE_REQUESTED`: sets `type` to `CloseRequested` and returns `true`.
4. Any other event returns `false`.

## Event callbacks

```cpp
struct native_event_callback_t
{
    std::function<void(const void*)> callback;
    void operator()(const void* native_event) const;
};

struct window_event_callback_t
{
    std::function<void(const window_event_t&)> callback;
    void operator()(const window_event_t& event) const;
};

struct input_event_callback_t
{
    std::function<void(const input_event_t&)> callback;
    void operator()(const input_event_t& event) const;
};
```

Three small wrappers with the same shape. Each holds a `std::function` and calls it from `operator()` only if it is set, so an empty callback is a valid "ignore these events" value. The native one receives a pointer to the `SDL_Event`; it is only valid during the call.

## `input_event_t`

```cpp
enum class e_input_event_types : u8 { None, Key, MouseButton, MouseMove, MouseWheel };
enum class e_input_states      : u8 { Pressed = 0, Released = 1, Held = 2 };

struct input_event_t
{
public:
    e_input_event_types type;
    u32                 window_id;

    union
    {
        struct { e_key_codes code; e_input_states state; }                      key;
        struct { e_mouse_buttons button; e_input_states state; f32 x, y; }      mouse_button;
        struct { f32 x, y; f32 delta_x, delta_y; }                              mouse_move;
        struct { f32 delta_x, delta_y; }                                        mouse_wheel;
    };
};
```

One keyboard or mouse event, already translated. `type` says which union member is valid. The default constructor sets `type` to `None`, `window_id` to `0` and zeroes `key`.

| Type          | Member         | Data                                                          |
| ------------- | -------------- | ------------------------------------------------------------- |
| `Key`         | `key`          | Engine key code and `Pressed`, `Released` or `Held`           |
| `MouseButton` | `mouse_button` | Engine button, `Pressed` or `Released`, cursor position       |
| `MouseMove`   | `mouse_move`   | Cursor position and movement since the previous motion event  |
| `MouseWheel`  | `mouse_wheel`  | Horizontal and vertical scroll amount                         |

`Held` is used only for key auto-repeat events. Mouse buttons are never `Held`.

## `e_key_codes`

```cpp
constexpr u32 KEYCODES_TABLE_SIZE = 512;

enum class e_key_codes : u16
{
    Unknown   = 0,
    Backspace = 8, Tab = 9, Return = 13, Escape = 27, Space = 32, Delete = 127,
    Insert    = 256, Home, End, PageUp, PageDown, ArrowLeft, ArrowRight, ArrowUp, ArrowDown,
    // CapsLock ... Menu, F1 ... F24, Numpad0 ... NumpadEqual, A ... Z,
    // Num0 ... Num9, Minus ... Slash, MediaPlay ... MediaMute
};
```

The engine's own key codes. The first few keys reuse their ASCII values; the rest count up from `256`, and the last value (`MediaMute`) is well below `512`. `KEYCODES_TABLE_SIZE` is the size of every per-key array: the scancode table in the parser and the three key arrays in `input_state_t`. It is also SDL's scancode count, which lets the parser index its table directly by scancode.

## `e_mouse_buttons`

```cpp
constexpr u8 MOUSE_BUTTONS_TABLE_SIZE = 10;

enum class e_mouse_buttons : u8
{
    None = 0, Left = 1, Right = 2, Middle = 3,
    WheelUp = 4, WheelDown = 5, WheelLeft = 6, WheelRight = 7,
    XButton1 = 8, XButton2 = 9,
};
```

The engine's mouse buttons. `MOUSE_BUTTONS_TABLE_SIZE` is the size of the button table in the parser and of the three button arrays in `input_state_t`. Scrolling does not produce the `Wheel*` buttons; it produces `MouseWheel` events (see the parser below).

## `input_event_parser_t`

```cpp
class BF_PLATFORM_API input_event_parser_t
{
public:
    static const std::array<e_key_codes, KEYCODES_TABLE_SIZE>          keycodes_mapper;
    static const std::array<e_mouse_buttons, MOUSE_BUTTONS_TABLE_SIZE> mouse_buttons_mapper;

    static bool parse(const SDL_Event& sdl_event, input_event_t& event);

private:
    static std::array<e_key_codes, KEYCODES_TABLE_SIZE> create_keycodes_mapper();
};
```

### State

| Field                  | Meaning                                                              |
| ---------------------- | -------------------------------------------------------------------- |
| `keycodes_mapper`      | Indexed by SDL scancode. Every entry starts as `Unknown`, then `create_keycodes_mapper()` fills one entry per supported key |
| `mouse_buttons_mapper` | Indexed by SDL button number. Entry `i` is the `e_mouse_buttons` value `i`, from `None` to `XButton2` |

Both tables are static and built once, when the engine library is loaded.

### `parse(sdl_event, event)`

| SDL event                                            | Result                                                      |
| ---------------------------------------------------- | ----------------------------------------------------------- |
| `SDL_EVENT_KEY_DOWN`, `SDL_EVENT_KEY_UP`             | `Key`. The code comes from `keycodes_mapper[scancode]`. The state is `Held` if SDL marks the event as a repeat, otherwise `Pressed` for key down and `Released` for key up |
| `SDL_EVENT_MOUSE_BUTTON_DOWN`, `SDL_EVENT_MOUSE_BUTTON_UP` | `MouseButton`. The button comes from `mouse_buttons_mapper[button]`, the state is `Pressed` or `Released`, and `x`, `y` are the cursor position |
| `SDL_EVENT_MOUSE_MOTION`                             | `MouseMove` with `x`, `y` and the relative motion `xrel`, `yrel` |
| `SDL_EVENT_MOUSE_WHEEL`                              | `MouseWheel` with SDL's `x` and `y` scroll amounts          |
| Anything else                                        | Returns `false`                                             |

A key whose scancode is `512` or more, or a mouse button whose number is `10` or more, is rejected with `false`. A scancode that is in range but not in the table becomes `e_key_codes::Unknown`, and the event is still returned. Every accepted event also copies SDL's `windowID` into `event.window_id`.

## `input_names_t`

```cpp
struct BF_PLATFORM_API input_names_t
{
public:
    static bool get_key_code(const char* name, e_key_codes& out_key_code);
    static bool get_mouse_button(const char* name, e_mouse_buttons& out_mouse_button);
};
```

Translates names to codes, for Lua scripts. Each function walks a fixed table with `strcmp`. On a match it writes the code to the output parameter and returns `true`; otherwise it returns `false` and leaves the output untouched.

Key names are the `e_key_codes` enumerator names, written exactly the same way (`"A"`, `"Num1"`, `"ArrowLeft"`, `"F12"`, `"NumpadEnter"`, `"MediaMute"`, ...). `Unknown` has no name. The match is case sensitive, so `"a"` and `"space"` are not found.

Mouse button names are `"Left"`, `"Right"`, `"Middle"`, `"XButton1"` and `"XButton2"`. The `None` and `Wheel*` values have no name.

## `input_state_t`

```cpp
struct BF_PLATFORM_API input_state_t
{
public:
    bool         enabled;
    bool         keys_down[KEYCODES_TABLE_SIZE];
    bool         keys_pressed[KEYCODES_TABLE_SIZE];
    bool         keys_released[KEYCODES_TABLE_SIZE];
    bool         mouse_buttons_down[MOUSE_BUTTONS_TABLE_SIZE];
    bool         mouse_buttons_pressed[MOUSE_BUTTONS_TABLE_SIZE];
    bool         mouse_buttons_released[MOUSE_BUTTONS_TABLE_SIZE];
    math::vec2_t mouse_position;
    math::vec2_t mouse_wheel;

    input_state_t();

    void begin_frame();
    void process(const input_event_t& event);
    void clear();

    bool is_key_down(const e_key_codes key_code) const;
    bool is_key_pressed(const e_key_codes key_code) const;
    bool is_key_released(const e_key_codes key_code) const;
    bool is_mouse_button_down(const e_mouse_buttons mouse_button) const;
    bool is_mouse_button_pressed(const e_mouse_buttons mouse_button) const;
    bool is_mouse_button_released(const e_mouse_buttons mouse_button) const;
};
```

The keyboard and mouse state for one frame, built from `input_event_t` values. The arrays are indexed by the numeric value of `e_key_codes` and `e_mouse_buttons`.

### State

| Field                       | Meaning                                                        |
| --------------------------- | -------------------------------------------------------------- |
| `enabled`                   | When `false`, key, button and wheel events are ignored. Starts `true` |
| `keys_down` and `mouse_buttons_down` | Held right now                                        |
| `keys_pressed` and `mouse_buttons_pressed` | Went down since the last `begin_frame()`        |
| `keys_released` and `mouse_buttons_released` | Went up since the last `begin_frame()`        |
| `mouse_position`            | Last cursor position from a `MouseMove` event                  |
| `mouse_wheel`               | Sum of wheel deltas since the last `begin_frame()`             |

### Constructor

Sets `enabled` to `true`, zeroes the two vectors and calls `clear()`.

### `begin_frame()`

Clears the `pressed` and `released` arrays and resets `mouse_wheel` to zero. The `down` arrays and `mouse_position` are kept. Call it once per frame, before polling events.

### `process(event)`

1. A `MouseMove` event updates `mouse_position`. This happens even when `enabled` is `false`.
2. If `enabled` is `false`, stops here.
3. `Key`: on `Pressed`, sets `down` and `pressed` for that key; on `Released`, clears `down` and sets `released`. `Held` changes nothing, since the key is already down.
4. `MouseButton`: the same as keys, using the button arrays. The `x`, `y` in the event are not used.
5. `MouseWheel`: adds the deltas to `mouse_wheel`.

Codes outside the array range are ignored.

### `clear()`

Clears the `down` arrays, then calls `begin_frame()`. After it, nothing is down, pressed or released. `mouse_position` and `enabled` are not changed.

### Queries

| Member                             | Returns                                         |
| ---------------------------------- | ----------------------------------------------- |
| `is_key_down(code)`                | `keys_down[code]`                               |
| `is_key_pressed(code)`             | `keys_pressed[code]`                            |
| `is_key_released(code)`            | `keys_released[code]`                           |
| `is_mouse_button_down(button)`     | `mouse_buttons_down[button]`                    |
| `is_mouse_button_pressed(button)`  | `mouse_buttons_pressed[button]`                 |
| `is_mouse_button_released(button)` | `mouse_buttons_released[button]`                |

Each one returns `false` for a value outside the array range instead of reading past it.

## Usage

### Who owns it

`runtime_t` has a `platform::platform_t platform` member and a `bool should_close` member. Its constructor passes `&should_close` as the platform's `should_close_handler`, so `request_close()` ends up setting `runtime.should_close`.

- `runtime_t::initialize()` initializes the platform first, then scripting and audio. If any of them fails, it returns `false`.
- `runtime_t::create_window()` calls `window_manager.create_window()` with the platform's display descriptor, then `renderer.attach_window()`. If the renderer cannot attach a GL context, the window is destroyed again and `nullptr` is returned.
- `runtime_t::destroy_window()` detaches the window from the renderer, then destroys it.
- `runtime_t::render()` walks the windows with `get_window_count()` and `get_window_at()`.
- `runtime_t::dispose()` disposes the platform **last**, after audio, scripting and the renderer.

`input_state_t` is not owned by the platform or the runtime. The editor has its own `platform::input_state_t input_state` member and passes it by reference to `scene_player_t::initialize()`, which hands it to the script system.

### Editor setup

The editor creates the runtime with `display_descriptor_t(graphics::e_graphic_backends::OpenGL)` and one window:

```cpp
platform::window_descriptor_t descriptor(window_title, window_bounds, platform::e_window_flags::Resizable | platform::e_window_flags::Maximized, true);
window = runtime.create_window(descriptor);
```

`window_title` is `"ByteForge Editor"` and `window_bounds` is `(0, 0, 1600, 900)`, so the position is left to SDL. It then sets the native callback to forward every SDL event to ImGui (`ImGui_ImplSDL3_ProcessEvent`), and builds the two other callbacks in `run()`:

```cpp
input_callback.callback = [this](const platform::input_event_t& event)
{
    input_state.process(event);
};

window_callback.callback = [this](const platform::window_event_t& event)
{
    if (event.type == platform::e_window_event_types::CloseRequested)
    {
        runtime.platform.request_close();
    }
};
```

The editor does not react to `Resize` events. It reads `window->get_size()` every frame when it renders the scene.

The status panel shows `window_manager.get_window_count()`.

### Frame order in the editor

```
while (!runtime.should_close)
    frame_manager.start_frame()
    input_state.begin_frame()             -- clears pressed, released, wheel
    platform.poll_events(...)             -- fills input_state, may request close
    scene_player.update(delta_time)       -- Lua reads input_state
    audio.update()
    render
    frame_manager.end_frame()
```

### Scene playback

`scene_player_t` switches the input state on and off:

- `start()` calls `clear()` and sets `enabled` to `true`.
- `stop()` sets `enabled` to `false` and calls `clear()`.

While the scene is stopped, key, button and wheel events are dropped, and only the mouse position keeps updating.

### Lua API

Scripts get an `input` table:

| Function                          | Returns          | Description                                     |
| --------------------------------- | ---------------- | ----------------------------------------------- |
| `input.is_key_down(name)`         | boolean          | `is_key_down` for the named key                 |
| `input.is_key_pressed(name)`      | boolean          | `is_key_pressed` for the named key              |
| `input.is_key_released(name)`     | boolean          | `is_key_released` for the named key             |
| `input.is_mouse_down(name)`       | boolean          | `is_mouse_button_down` for the named button     |
| `input.is_mouse_pressed(name)`    | boolean          | `is_mouse_button_pressed` for the named button  |
| `input.is_mouse_released(name)`   | boolean          | `is_mouse_button_released` for the named button |
| `input.get_mouse_position()`      | `x`, `y`         | `mouse_position` as two numbers                 |
| `input.get_mouse_wheel()`         | `x`, `y`         | `mouse_wheel` as two numbers                    |

Names go through `input_names_t`. An unknown key name raises the Lua error `unknown key name '<name>'`, and an unknown button name raises `unknown mouse button name '<name>'`. The functions reach the `input_state_t` through a light userdata stored as upvalue `3`.

From `applications/editor/assets/scripts/demo.lua`:

```lua
if input.is_key_down("ArrowLeft")  or input.is_key_down("A") then direction_x = direction_x - 1 end
if input.is_key_down("ArrowRight") or input.is_key_down("D") then direction_x = direction_x + 1 end

if input.is_key_pressed("Space") then
    transform.set_position(entity, 0, 0)
    log.info("Space pressed: position reset")
end
```

## Notes

- `poll_events()` returns as soon as it sees `SDL_EVENT_QUIT`. The quit event still reaches the native callback first, and the events behind it in the queue are left for a later call.
- The mouse button table maps SDL button numbers straight to `e_mouse_buttons` values. SDL3 numbers its buttons left `1`, middle `2`, right `3`, X1 `4` and X2 `5`, so with the current table the physical middle button arrives as `Right`, the physical right button as `Middle`, and the two side buttons as `WheelUp` and `WheelDown`. `XButton1` and `XButton2` only match SDL button numbers `8` and `9`.
- Keys that are not in the scancode table arrive as `Unknown`, whose index is `0`, so `is_key_down(e_key_codes::Unknown)` becomes `true` while any such key is held. Lua cannot ask for it, since `Unknown` has no name.
- The editor feeds every input event to `input_state_t`, including events ImGui also handles. Typing in an editor text field while the scene runs is also seen by scripts.
- `mouse_position` comes from SDL mouse coordinates, while `window_t::get_size()` returns pixels. On a scaled display the two can use different units.
- `create_window()` cannot place a window at exactly `(0, 0)`: that position is treated as "let SDL decide".
- `window_t::set_title()` changes only the OS title; `descriptor.title` keeps the old one.
- `platform_t`, `window_manager_t` and `window_t` have no destructors and their copy operations are not deleted. Windows are freed only by `destroy_window()` or `dispose()`, and a copy of `window_manager_t` would hold the same window pointers.
