# Logging module

> Language: **English** | [Português (Brasil)](../../../../pt-br/engine/modules/logging/README.md)

The logging module formats log messages, keeps the most recent ones in memory, and prints them to the terminal in color. It is a static `logger_t` with a set of `LOG_*` macros on top, a ring buffer (`log_buffer_t`) the editor's console reads from, and an optional sink callback. It lives in the `byteforge::logging` namespace and uses only the C and C++ standard libraries.

## Files

| File                                                                 | Contents                                              |
| -------------------------------------------------------------------- | ----------------------------------------------------- |
| `engine/include/byte-forge-engine/modules/logging/e_log_levels.h`    | `e_log_levels`, the seven levels                      |
| `engine/include/byte-forge-engine/modules/logging/log_entry.h`       | `log_entry_t`, one formatted line and its level       |
| `engine/include/byte-forge-engine/modules/logging/log_sink.h`        | `log_sink_t`, the callback type                       |
| `engine/include/byte-forge-engine/modules/logging/log_buffer.h`      | `log_buffer_t` declaration                            |
| `engine/include/byte-forge-engine/modules/logging/logger.h`          | `logger_t` declaration                                |
| `engine/include/byte-forge-engine/modules/logging/log_macros.h`      | `LOG_*` macros and `extract_file_name`                |
| `engine/source/modules/logging/logger.cpp`                           | Global logger state, formatting and output            |
| `engine/source/modules/logging/log_buffer.cpp`                       | Ring buffer implementation                            |
| `engine/source/modules/logging/log_level_info.h` / `.cpp`            | Private: level names and terminal colors              |
| `engine/source/modules/logging/log_styles.h`                         | Private: ANSI escape codes                            |

The last three files are under `engine/source`, so applications can't include them.

Related pieces in other modules:

| File                                                                    | Role                                                 |
| ----------------------------------------------------------------------- | ---------------------------------------------------- |
| `applications/editor/source/ui/components/console_output_component.cpp` | Shows the buffer, changes the minimum level        |
| `modules/scene/scene_script_system.cpp`                                 | Lua `log.info`, `log.warning` and `log.error`        |

## Concepts

### One global logger

All logger state lives in an anonymous namespace inside `logger.cpp`, which is compiled into the engine DLL: the buffer, the sink, the minimum level and the console flag. `logger_t` only has static functions that read and change it. The engine and every application that links it share the same state, so a message logged by the engine shows up in the editor's console.

### Levels

```cpp
enum class e_log_levels : u8
{
    Info    = 0,
    Debug   = 1,
    Trace   = 2,
    Success = 3,
    Warning = 4,
    Error   = 5,
    Fatal   = 6
};
```

A message is kept only if its level is greater than or equal to the minimum level, which starts at `Info`. Because `Info` is `0`, the default lets every message through. Raising the minimum hides the levels below it in this order, so setting it to `Debug` hides `Info` but keeps `Trace` and `Success`.

### Line format

Every message becomes one line:

```
[24-10-2026 14:03:27] [ERROR] [scripting.cpp:50:load_file] Could not run script 'a.lua': ...
[24-10-2026 14:03:27] [INFO] [lua] hello from on_start
```

The date is local time as `day-month-year hour:minute:second`. The bracketed location (file name, line, function) is present only for messages logged through the detailed path, which the `LOG_FATAL`, `LOG_ERROR`, `LOG_WARNING` and `LOG_TRACE` macros use. If the local time can't be read, the date is replaced by `unknown-datetime`.

### Where a line goes

For each message that passes the level check, `write()` in `logger.cpp` does three things in order:

1. Adds a `log_entry_t` with the level and the line to the buffer.
2. Calls the sink, if one is set, with that entry.
3. If console output is enabled, prints the line to `stderr` for `Warning`, `Error` and `Fatal`, or to `stdout` for the others, wrapped in an ANSI color code and a reset code.

Everything runs synchronously on the calling thread, and there is no locking.

## `log_entry_t`

```cpp
struct log_entry_t
{
public:
    e_log_levels level;
    std::string line;
};
```

### State

| Field   | Meaning                                                             |
| ------- | ------------------------------------------------------------------- |
| `level` | The level the message was logged at                                |
| `line`  | The full formatted line, with date, level name and location, without color codes or newline |

## `log_sink_t`

```cpp
using log_sink_t = void (*)(const log_entry_t& entry);
```

A plain function pointer. It receives the same entry that was just added to the buffer. The reference is only valid during the call, so a sink that wants to keep the line has to copy it. Since it is a function pointer and not a `std::function`, it can't capture state.

## `log_buffer_t`

```cpp
class BF_LOGGING_API log_buffer_t
{
public:
    void add(const log_entry_t& entry);
    void clear();
    usize get_count() const;
    const log_entry_t& get(const usize index) const;
    u64 get_total_count() const;

private:
    static constexpr usize DEFAULT_CAPACITY = 2000;

    std::vector<log_entry_t> entries;
    usize capacity;
    usize oldest_index;
    u64 total_count;
};
```

A fixed-capacity ring buffer. It holds up to 2000 entries; after that, each new entry overwrites the oldest one.

### State

| Field          | Meaning                                                                  |
| -------------- | ------------------------------------------------------------------------ |
| `entries`      | Stored entries. Grows to `capacity`, then is reused in place             |
| `capacity`     | Maximum number of entries. Always `DEFAULT_CAPACITY` (`2000`)            |
| `oldest_index` | Position of the oldest entry inside `entries` once the buffer is full    |
| `total_count`  | Number of entries ever added, including those already overwritten       |

### `add(entry)`

Increments `total_count`. While `entries` is below capacity, appends a copy of `entry`. Once full, writes the copy over `entries[oldest_index]` and moves `oldest_index` forward by one, wrapping to `0` at the end.

### `get(index)`

Returns the entry at `index` counted from the oldest: `0` is the oldest kept entry and `get_count() - 1` is the newest. Internally it reads `entries[(oldest_index + index) % entries.size()]`. The index is not checked.

### `get_count()` and `get_total_count()`

`get_count()` is the number of entries currently kept, at most 2000. `get_total_count()` is the number ever added.

### `clear()`

Removes every entry and resets `oldest_index` to `0`. `total_count` is not reset.

## `logger_t`

```cpp
struct BF_LOGGING_API logger_t
{
    static void set_sink(log_sink_t sink);
    static void set_min_level(e_log_levels level);
    static e_log_levels get_min_level();
    static void set_console_output(bool enabled);
    static log_buffer_t& get_buffer();

    static void standard_log(e_log_levels level, const char* format, ...);
    static void detailed_log(e_log_levels level, const char* file_name, i32 code_line, const char* function_name, const char* format, ...);
};
```

### State

The state is not part of the struct; it is file-level in `logger.cpp`:

| Variable                 | Initial value | Meaning                                    |
| ------------------------ | ------------- | ------------------------------------------ |
| `buffer`                 | empty         | The single `log_buffer_t`                  |
| `current_sink`           | `nullptr`     | The sink, or none                          |
| `min_level`              | `Info`        | Messages below this level are dropped      |
| `console_output_enabled` | `true`        | Whether lines are printed to the terminal  |

### `set_sink(sink)`

Replaces the sink. Only one sink exists at a time; passing `nullptr` removes it.

### `set_min_level(level)` and `get_min_level()`

Set and read the minimum level. A message below it is dropped before any formatting, so it reaches neither the buffer, nor the sink, nor the terminal.

### `set_console_output(enabled)`

Turns terminal printing on or off. The buffer and the sink keep receiving entries either way.

### `get_buffer()`

Returns a reference to the global buffer. The editor's console reads it every frame and clears it with its Clear button.

### `standard_log(level, format, ...)`

1. Returns if `level` is below the minimum.
2. Formats `format` and the arguments with `vsnprintf` into a 1024-byte buffer. Longer messages are cut.
3. Writes the line with an empty location.

### `detailed_log(level, file_name, code_line, function_name, format, ...)`

Same as `standard_log`, but also formats ` [file_name:code_line:function_name]` into a 256-byte buffer and puts it between the level name and the message. The final line is built in a 1400-byte buffer.

## Macros

```cpp
#define LOG_FATAL(...)   ... detailed_log(e_log_levels::Fatal,   BF_LOG_FILE_NAME, __LINE__, __func__, __VA_ARGS__)
#define LOG_ERROR(...)   ... detailed_log(e_log_levels::Error,   ...)
#define LOG_WARNING(...) ... detailed_log(e_log_levels::Warning, ...)
#define LOG_TRACE(...)   ... detailed_log(e_log_levels::Trace,   ...)

#define LOG_SUCCESS(...) ... standard_log(e_log_levels::Success, __VA_ARGS__)
#define LOG_DEBUG(...)   ... standard_log(e_log_levels::Debug,   __VA_ARGS__)
#define LOG_INFO(...)    ... standard_log(e_log_levels::Info,    __VA_ARGS__)
```

The arguments are a `printf` format string followed by its values. `BF_LOG_FILE_NAME` calls `extract_file_name(__FILE__)`, a `constexpr` function that returns a pointer to the part after the last `/` or `\`, so the location shows `scripting.cpp` instead of the full path. `__func__` is the bare function name, without class or namespace.

## Private helpers

`log_level_info_t` maps each level to its name and terminal color, and `log_style_t` holds the escape codes:

| Level     | Name      | Terminal style       | Stream   |
| --------- | --------- | -------------------- | -------- |
| `Info`    | `INFO`    | bright white text    | `stdout` |
| `Debug`   | `DEBUG`   | cyan text            | `stdout` |
| `Trace`   | `TRACE`   | blue text            | `stdout` |
| `Success` | `SUCCESS` | green text           | `stdout` |
| `Warning` | `WARNING` | yellow text          | `stderr` |
| `Error`   | `ERROR`   | red text             | `stderr` |
| `Fatal`   | `FATAL`   | red background       | `stderr` |

A value outside the enum gets the name `UNKNOWN` and the reset code.

## Usage

Engine and editor code include `log_macros.h` and log with format strings kept as named constants:

```cpp
const char* const file_error_format = "Could not run script '%s': %s";

LOG_ERROR(file_error_format, file_path, get_error_message(lua_state));
```

The editor's Console Output panel reads `logger_t::get_buffer()` directly, so it needs no sink and also shows entries logged before the editor window existed. Its controls map to the logger like this:

| Control     | Effect                                                                   |
| ----------- | ------------------------------------------------------------------------ |
| Capture     | Calls `set_min_level()`. Changes what the logger keeps, globally         |
| Show        | Filters the displayed lines by level. The buffer is unchanged            |
| Search      | Case-insensitive substring filter on `line`                              |
| Clear       | Calls `clear()` on the buffer                                            |
| Auto-scroll | Scrolls to the bottom when `get_total_count()` changed since last frame  |

The summary line shows the shown, kept and total counts, plus the number of warnings and of errors (`Error` and `Fatal` together) among the kept entries.

Lua scripts log through the `log` table, which the scene module registers (see the [scene module](../scene/README.md)). `log.info`, `log.warning` and `log.error` call `LOG_INFO`, `LOG_WARNING` and `LOG_ERROR` with the format `[lua] %s`.

## Notes

- `LOG_FATAL` only logs. It does not abort or exit; the caller decides what happens next.
- The level order is not a verbosity order. `Info` is the lowest, so there is no minimum level that shows `Info` while hiding `Debug` and `Trace`.
- Messages from Lua's `log.warning` and `log.error` go through the detailed path, so their location is the C++ binding in `scene_script_system.cpp`, not the line in the Lua script.
- Messages are cut at 1023 characters, and the whole line, including date and location, at 1399.
- The format functions are variadic C functions without format checking, so a mismatch between the format string and the arguments is not caught at compile time.
- The ANSI color codes are always written when console output is on. Whether they appear as colors depends on the terminal.
- No code sets a sink or turns console output off today. The editor's console works only through the buffer.
- `log_buffer_t::get()` on an empty buffer divides by zero. Callers loop up to `get_count()` first.
