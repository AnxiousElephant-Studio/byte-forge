# Scripting module

> Language: **English** | [Português (Brasil)](../../../../pt-br/engine/modules/scripting/README.md)

The scripting module owns the engine's Lua interpreter. It is one class, `scripting_t`, that creates a single `lua_State`, opens the standard libraries, and offers a few helpers to run a file, call a global function and read fields from a global table. It lives in the `byteforge::scripting` namespace and wraps the [Lua](https://www.lua.org) C API.

The functions that scripts call (`transform`, `velocity`, `label`, `audio`, `log`, `input`) and the per-entity script environments are not part of this module. They are registered by the scene's script system on top of the state created here and are documented in the [scene module](../scene/README.md).

## Files

| File                                                              | Contents                                   |
| ----------------------------------------------------------------- | ------------------------------------------ |
| `engine/include/byte-forge-engine/modules/scripting/scripting.h`  | `scripting_t` declaration                  |
| `engine/source/modules/scripting/scripting.cpp`                   | Implementation over the Lua C API          |

Related pieces in other modules:

| File                                                              | Role                                                      |
| ----------------------------------------------------------------- | --------------------------------------------------------- |
| `modules/runtime/runtime.h`                                       | Owns the `scripting_t` instance and its lifetime          |
| `modules/scene/scene_script_system.cpp`                           | Registers the Lua API and loads entity scripts            |
| `applications/editor/source/ui/components/status_panel_component.cpp` | Shows `get_memory_bytes()` as "Lua memory"            |

## Concepts

### How Lua is hidden

The header only forward-declares the Lua state type:

```cpp
struct lua_State;
```

`scripting_t` stores a pointer to it, so `scripting.h` does not include any Lua header. Only `scripting.cpp` includes `<lua.hpp>`. Lua itself comes from the xmake package `lua`, which `engine/xmake.lua` requires and links into the engine.

### One shared state

There is exactly one `lua_State` per `scripting_t`, and the runtime owns exactly one `scripting_t`. Every script in the engine runs in that state. `get_lua_state()` hands out the raw pointer, and the scene's script system uses it to register its global tables and to give each entity script its own environment table. Since the state is shared, globals set by `load_file()` and the tables registered by the scene live in the same global table (`_G`).

### The Lua stack

All helpers work on the Lua stack. `stack_top` (`-1`) is the index of the value on top. Each helper leaves the stack as it found it, except `push_table()`, which on success leaves the table on top until `pop_table()` removes it.

## `scripting_t`

```cpp
class BF_SCRIPTING_API scripting_t
{
public:
    scripting_t() : lua_state(nullptr) {}

    bool initialize();
    bool load_file(const char* file_path);
    bool call(const char* function_name);

    bool push_table(const char* table_name);
    void pop_table();
    bool get_integer(const char* field, i32& out_integer);
    bool get_number(const char* field, f64& out_number);
    bool get_string(const char* field, char* out_buffer, usize buffer_size);

    void dispose();

    lua_State* get_lua_state() const { return lua_state; }
    usize get_memory_bytes() const;

private:
    lua_State* lua_state;
};
```

There is no destructor. The state is closed only in `dispose()`, which the owner has to call.

### State

| Field       | Meaning                                                                 |
| ----------- | ----------------------------------------------------------------------- |
| `lua_state` | The Lua interpreter. `nullptr` before `initialize()` and after `dispose()` |

### `initialize()`

1. Creates the state with `luaL_newstate()`, which uses Lua's default allocator.
2. If that returns `nullptr`, logs `Could not create the Lua state.` at the `Fatal` level and returns `false`.
3. Opens every standard library with `luaL_openlibs()`. Scripts therefore have every standard library, such as `string`, `table`, `math`, `coroutine`, `io`, `os`, `debug` and `package`.
4. Returns `true`.

### `load_file(file_path)`

Runs a Lua file in the global environment with `luaL_dofile`, which loads the chunk and calls it right away. Top-level code in the file executes, and any global functions or variables it defines become visible to `call()` and `push_table()`.

If loading or running fails (missing file, syntax error, runtime error), it logs `Could not run script '<path>': <message>` at the `Error` level, pops the error message and returns `false`. `<message>` is the text Lua put on the stack, or `unknown error` if it is not a string.

`file_path` is opened as given, so a relative path is relative to the process working directory.

### `call(function_name)`

1. Pushes the global named `function_name`.
2. If it is not a function, logs `Lua function '<name>' does not exist.`, pops it and returns `false`.
3. Calls it with `lua_pcall` with no arguments, no results and no message handler.
4. If the call raises an error, logs `Lua function '<name>' failed: <message>`, pops the message and returns `false`.

Without a message handler the logged message has no traceback, only the error text.

### `push_table(table_name)`

Pushes the global named `table_name`. If it is a table, it stays on top of the stack and the function returns `true`. If it is anything else (including `nil`), it is popped and the function returns `false`. Nothing is logged.

### `pop_table()`

Pops the top of the stack, but only if it is a table. Calling it after a failed `push_table()` does nothing.

### `get_integer(field, out_integer)`, `get_number(field, out_number)`, `get_string(field, out_buffer, buffer_size)`

The three getters read `field` from the table on top of the stack, so they are meant to be called between `push_table()` and `pop_table()`. Each one pushes the field, checks its type, writes the output only if the type matches, pops the field and returns whether it matched. On a mismatch the output is left untouched.

| Getter        | Accepts                                           | Conversion                                    |
| ------------- | ------------------------------------------------- | --------------------------------------------- |
| `get_integer` | Values with Lua's integer subtype (`lua_isinteger`) | Cast to `i32`                               |
| `get_number`  | Numbers and strings that convert to a number (`lua_isnumber`) | `lua_tonumber`                    |
| `get_string`  | Only real strings (`LUA_TSTRING`), not numbers    | Copied with `snprintf` into `out_buffer`      |

`get_integer` rejects floats, even `3.0`. `get_string` truncates to `buffer_size - 1` characters and always writes the terminator when `buffer_size` is not zero.

### `get_memory_bytes()`

Returns the memory Lua's collector currently tracks, in bytes. It combines `lua_gc(LUA_GCCOUNT)` (whole kilobytes) times `1024` with `lua_gc(LUA_GCCOUNTB)` (the remaining bytes). Returns `0` when there is no state.

### `dispose()`

Closes the state with `lua_close` and sets the pointer to `nullptr`. Calling it twice, or before `initialize()`, does nothing.

### `get_lua_state()`

Returns the raw `lua_State*`, or `nullptr` if the state does not exist. This is how other modules reach Lua directly.

## Usage

`runtime_t` has a `scripting::scripting_t scripting` member. `runtime_t::initialize()` initializes it right after the platform and before audio, and `runtime_t::dispose()` disposes it right after audio. See the [runtime module](../runtime/README.md).

The editor never calls `load_file()` or `call()`. It passes the state to the scene player, which forwards it to the scene's script system:

```cpp
if (!scene.initialize() || !scene_player.initialize(runtime.scripting.get_lua_state(), scene, runtime.audio, input_state))
{
    LOG_FATAL(scene_error_format);
    return false;
}
```

From there, the scene module registers its tables and runs each entity's script in its own environment. The status panel shows the state's size every frame:

```cpp
ImGui::Text(lua_memory_format, static_cast<f32>(context.runtime->scripting.get_memory_bytes()) / bytes_per_kilobyte);
```

A direct use of the helpers would look like this:

```cpp
scripting.load_file("assets/scripts/config.lua");

i32 width = 0;

if (scripting.push_table("config"))
{
    scripting.get_integer("width", width);
    scripting.pop_table();
}
```

## Notes

- `initialize()` must succeed before any other member is called. `load_file()`, `call()` and the table helpers pass `lua_state` to Lua without checking it for `nullptr`.
- Calling `initialize()` twice creates a second state and overwrites the pointer to the first one, which is never closed.
- `load_file()` uses `luaL_dofile`, which keeps every value the chunk returns. Those values stay on the stack after a successful call.
- The getters read from whatever is on top of the stack. If the value there cannot be indexed, `lua_getfield` raises a Lua error outside of any protected call.
- `get_integer` narrows Lua's 64-bit integer to `i32` with a plain cast, so large values wrap.
- `scripting_t` does not delete its copy operations. A copy would share the same `lua_State` pointer.
- All standard libraries are opened, including `io` and `os`, so scripts can read and write files and run system commands.
