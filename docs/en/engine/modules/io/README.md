# IO module

> Language: **English** | [Português (Brasil)](../../../../pt-br/engine/modules/io/README.md)

The io module reads and writes files and works with paths and directories. It has two parts: `file_t`, a header-only struct that reads a whole file into a heap buffer or writes a buffer to a file using C `stdio`, and `file_system_t`, a set of static functions over `std::filesystem` for checking, creating, listing and removing paths and for reading and writing text. It lives in the `byteforge::io` namespace.

## Files

| File                                                                | Contents                                            |
| ------------------------------------------------------------------- | --------------------------------------------------- |
| `engine/include/byte-forge-engine/modules/io/e_file_access_modes.h` | `e_file_access_modes`: `Read`, `Write`              |
| `engine/include/byte-forge-engine/modules/io/e_file_data_types.h`   | `e_file_data_types`: `Text`, `Binary`               |
| `engine/include/byte-forge-engine/modules/io/file.h`                | `file_t`, declared and implemented in the header    |
| `engine/include/byte-forge-engine/modules/io/file_system.h`         | `directory_entry_t` and `file_system_t` declaration |
| `engine/source/modules/io/file_system.cpp`                          | `file_system_t` implementation                      |

Related pieces in other modules:

| File                                                                     | Role                                                      |
| ------------------------------------------------------------------------ | --------------------------------------------------------- |
| `modules/rendering/opengl/shader/opengl_shader.cpp`                      | Reads shader sources with `file_t` (`Text`)               |
| `modules/rendering/font/font.cpp`                                        | Reads font files with `file_t` (`Binary`)                 |
| `applications/editor/source/editor.cpp`                                  | Creates the `assets` directory at startup                 |
| `applications/editor/source/ui/components/content_browser_component.cpp` | Lists directories and creates new scripts                 |
| `applications/editor/source/ui/components/script_editor_component.cpp`   | Opens and saves scripts with `read_text` and `write_text` |

## Concepts

### Errors are return values

Nothing in this module logs or throws on purpose. Every operation that can fail returns `false` (or an empty result), and every `std::filesystem` call uses the overload that takes a `std::error_code`. Callers decide what to log; for example the script editor logs `Could not open script '<path>'.` when `read_text` fails.

### Text and binary

Files are always opened in binary mode (`"rb"` or `"wb"`), so there is no newline translation on any platform. The only difference between `e_file_data_types::Text` and `Binary` is that `Text` writes a `'\0'` after the last byte read, so the buffer can be used as a C string.

### Path strings

Paths go in as `const char*` and come out as `std::string`. Functions that return paths (`get_parent_path`, `join`, and the `path` field of directory entries) use `generic_string()`, so the separator is always `/`, even on Windows. File names and extensions use `string()`.

## `e_file_access_modes` and `e_file_data_types`

```cpp
enum class e_file_access_modes : u8 { Read, Write };
enum class e_file_data_types   : u8 { Text, Binary };
```

`e_file_access_modes` is only used inside `file_t` to choose between `"rb"` and `"wb"`. `e_file_data_types` is passed to `file_t::read()`.

## `file_t`

```cpp
struct file_t
{
public:
    char* content;
    usize size;

    file_t();
    ~file_t();

    file_t(const file_t&)            = delete;
    file_t& operator=(const file_t&) = delete;

    bool read(const char* path, const e_file_data_types data_type);
    bool write(const char* path, const void* data, usize data_size);
    void dispose();

private:
    constexpr static const char* read_mode  = "rb";
    constexpr static const char* write_mode = "wb";

    constexpr static const char* get_file_mode(const e_file_access_modes access_mode);
};
```

`file_t` owns the buffer it reads. The destructor calls `dispose()`, so a `file_t` on the stack frees its content when it goes out of scope. Copying is deleted, and since the struct declares a destructor and deleted copies, it has no move operations either.

### State

| Field     | Meaning                                                           |
| --------- | ----------------------------------------------------------------- |
| `content` | Heap buffer from `malloc` with the file's bytes, or `nullptr`     |
| `size`    | Number of bytes read, not counting the terminator. `0` when empty |

### `read(path, data_type)`

1. Calls `dispose()`, so any previous content is freed first.
2. Gets the file size with `std::filesystem::file_size`. Returns `false` if that fails (for example, the file does not exist).
3. Opens the file with `fopen(path, "rb")`. Returns `false` if that fails.
4. Allocates `size + 1` bytes with `malloc`. On failure resets `size` to `0`, closes the file and returns `false`.
5. Reads the whole file with one `fread` and closes it.
6. If fewer bytes than expected were read, calls `dispose()` and returns `false`.
7. For `Text`, writes `'\0'` at `content[size]`.
8. Returns `true`.

The buffer always has one extra byte. For `Binary` that byte is left uninitialized.

### `write(path, data, data_size)`

Opens `path` with `fopen(path, "wb")`, which creates the file or truncates an existing one, writes `data_size` bytes from `data` with one `fwrite`, closes the file and returns whether every byte was written. The parent directory must already exist. `write()` does not use or change `content` and `size`.

### `dispose()`

Frees `content` with `free` if it is not `nullptr`, sets it to `nullptr` and sets `size` to `0`. Calling it more than once is safe.

## `directory_entry_t`

```cpp
struct directory_entry_t
{
public:
    std::string name;
    std::string path;
    std::string extension;
    bool is_directory;
};
```

### State

| Field          | Meaning                                                         |
| -------------- | --------------------------------------------------------------- |
| `name`         | File or directory name with extension, for example `rotate.lua` |
| `path`         | Full path as listed, with `/` separators                        |
| `extension`    | Extension including the dot, for example `.lua`. Empty if none  |
| `is_directory` | `true` for directories. `false` if the check itself failed      |

## `file_system_t`

```cpp
struct BF_IO_API file_system_t
{
public:
    static bool exists(const char* path);
    static bool is_directory(const char* path);
    static bool create_directories(const char* path);
    static bool remove(const char* path);
    static bool rename(const char* path, const char* new_path);
    static bool list_directory(const char* path, std::vector<directory_entry_t>& out_entries);
    static std::string get_parent_path(const char* path);
    static std::string get_file_name(const char* path);
    static std::string get_extension(const char* path);
    static std::string join(const char* base_path, const char* relative_path);
    static bool read_text(const char* path, std::string& out_text);
    static bool write_text(const char* path, const std::string& text);
};
```

All members are static, and the struct has no state.

### `exists(path)` and `is_directory(path)`

Wrap `std::filesystem::exists` and `std::filesystem::is_directory`. Any error (for example, a permission problem) makes them return `false`.

### `create_directories(path)`

Creates `path` and every missing parent with `std::filesystem::create_directories`. Returns `true` if there was no error and `path` is a directory afterwards, so it also returns `true` when the directory already existed.

### `remove(path)`

Deletes `path` with `std::filesystem::remove_all`. For a directory this deletes everything inside it, recursively. Returns `true` only if at least one entry was removed and there was no error. A path that does not exist returns `false`.

### `rename(path, new_path)`

Moves or renames a file or folder with `std::filesystem::rename`. Returns `false` on any error, for example when the file is open in another program. It does not check whether `new_path` exists; the Content Browser checks that first.

### `list_directory(path, out_entries)`

1. Clears `out_entries`.
2. Opens a `std::filesystem::directory_iterator` on `path`. Returns `false` if that fails.
3. For each entry directly inside `path` (not recursive), fills a `directory_entry_t` and appends it.
4. Sorts the entries: directories first, then files, each group by `name` in byte order.
5. Returns `true`.

Byte order compares characters by their code, so uppercase names come before lowercase ones (`Zebra` before `apple`).

### `get_parent_path(path)`, `get_file_name(path)`, `get_extension(path)`

Thin wrappers over `std::filesystem::path`:

| Function          | `"assets/scripts/rotate.lua"` gives | Separator      |
| ----------------- | ----------------------------------- | -------------- |
| `get_parent_path` | `assets/scripts`                    | `/` (generic)  |
| `get_file_name`   | `rotate.lua`                        | not applicable |
| `get_extension`   | `.lua`                              | not applicable |

These only work on the string. They do not check that the path exists.

### `join(base_path, relative_path)`

Joins the two parts with `std::filesystem::path::operator/` and returns the result with `/` separators. `join("assets", "script_0.lua")` gives `assets/script_0.lua`. If `relative_path` is absolute, the result is `relative_path` alone, which is how `operator/` works.

### `read_text(path, out_text)`

Reads the file with a local `file_t` in `Text` mode and assigns its `size` bytes to `out_text`. Returns `false` if the read fails, leaving `out_text` untouched. The temporary buffer is freed when the function returns.

### `write_text(path, text)`

Writes `text.size()` bytes of `text` with a local `file_t::write()`. Returns `false` if the file can't be opened or not every byte was written.

## Usage

The editor creates its assets directory during startup and only logs if that fails:

```cpp
if (!io::file_system_t::create_directories(assets_directory))
{
    LOG_ERROR(assets_error_format, assets_directory);
}
```

The content browser lists the current directory into its `entries` vector, navigates up with `get_parent_path`, and creates new scripts by trying `script_0.lua`, `script_1.lua` and so on until `exists` returns `false`:

```cpp
const std::string path = io::file_system_t::join(current_directory.c_str(), file_name);

if (!io::file_system_t::exists(path.c_str()))
{
    io::file_system_t::write_text(path.c_str(), new_script_template);
}
```

The script editor loads a file with `read_text` and saves it with `write_text`, logging an error when either fails. In the renderer, shaders are read with `file_t` in `Text` mode, so `file.content` can be handed to OpenGL as a C string, and fonts are read in `Binary` mode.

## Notes

- Relative paths are relative to the process working directory. For the editor that is `applications/editor`.
- `read()` gets the size and opens the file in two separate steps. If the file changes size in between, the `fread` count no longer matches and `read()` fails.
- `file_t::write()` and `write_text()` replace the whole file. There is no append mode.
- `list_directory` uses the error-code overload to open the directory, but advances the iterator with the range-based `for`, which uses the throwing increment.
- `remove()` is recursive and permanent. The editor Content Browser calls it from Delete, after a confirmation.
- `read_text` and `file_t` read the whole file into memory at once; there is no streaming.
