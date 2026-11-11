# API

> Language: **English** | [Português (Brasil)](../../../pt-br/engine/api/README.md)

The API folder holds the two headers every other engine header builds on: the primitive type aliases and the export macros. Neither has a source file.

## Files

| Header    | Contents                        | Purpose                                         |
| --------- | ------------------------------- | ----------------------------------------------- |
| `types.h` | `u8`..`u64`, `i8`..`i64`, `f32`, `f64`, `usize`, `isize` | Fixed-size primitive type aliases |
| `api.h`   | `BF_EXPORT`, `BF_IMPORT`        | Building blocks of the per-module export macros |

All paths are relative to `engine/include/byte-forge-engine/api/`. Include them from the include root:

```cpp
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/api/api.h>
```

## `types.h`

Short aliases for the standard fixed-width types, declared in the `byteforge` namespace. Engine code uses them instead of `int`, `float`, `std::uint32_t` and the like, so every size is explicit.

| Alias   | Type             | Size    |
| ------- | ---------------- | ------- |
| `u8`    | `std::uint8_t`   | 8 bits  |
| `u16`   | `std::uint16_t`  | 16 bits |
| `u32`   | `std::uint32_t`  | 32 bits |
| `u64`   | `std::uint64_t`  | 64 bits |
| `i8`    | `std::int8_t`    | 8 bits  |
| `i16`   | `std::int16_t`   | 16 bits |
| `i32`   | `std::int32_t`   | 32 bits |
| `i64`   | `std::int64_t`   | 64 bits |
| `f32`   | `float`          | 32 bits |
| `f64`   | `double`         | 64 bits |
| `usize` | `std::size_t`    | Pointer-sized, unsigned |
| `isize` | `std::ptrdiff_t` | Pointer-sized, signed   |

`float` and `double` have no size guaranteed by the standard, so the header checks them with `static_assert`. The build fails on a platform where `f32` is not 32 bits or `f64` is not 64 bits.

Code inside a `byteforge::<module>` namespace sees the aliases without a prefix. Code outside it, such as the editor, writes `byteforge::u32` or adds a `using` declaration.

## `api.h`

The engine is built as a shared library (`set_kind("shared")` in `engine/xmake.lua`). Classes and functions that applications call have to be exported from it. `api.h` defines the two building blocks:

| Macro       | Windows                 | Other platforms                          | With `BF_STATIC` |
| ----------- | ----------------------- | ---------------------------------------- | ---------------- |
| `BF_EXPORT` | `__declspec(dllexport)` | `__attribute__((visibility("default")))` | empty            |
| `BF_IMPORT` | `__declspec(dllimport)` | `__attribute__((visibility("default")))` | empty            |

On other platforms both are the same, and they only hide anything when the library is compiled with `-fvisibility=hidden`, which the build does not do yet.

### One macro per module

Each module that exports symbols has its own header, `modules/<module>/<module>_api.h`, with its own macro:

```cpp
// modules/ecs/ecs_api.h
#if defined(BF_ECS_BUILD) || defined(BF_ENGINE_BUILD)
    #define BF_ECS_API BF_EXPORT
#else
    #define BF_ECS_API BF_IMPORT
#endif
```

| Module  | Macro            | Module    | Macro              |
| ------- | ---------------- | --------- | ------------------ |
| assets  | `BF_ASSETS_API`  | platform  | `BF_PLATFORM_API`  |
| audio   | `BF_AUDIO_API`   | project   | `BF_PROJECT_API`   |
| ecs     | `BF_ECS_API`     | rendering | `BF_RENDERING_API` |
| image   | `BF_IMAGE_API`   | runtime   | `BF_RUNTIME_API`   |
| io      | `BF_IO_API`      | scene     | `BF_SCENE_API`     |
| level   | `BF_LEVEL_API`   | scripting | `BF_SCRIPTING_API` |
| logging | `BF_LOGGING_API` | time      | `BF_TIME_API`      |
| physics | `BF_PHYSICS_API` |           |                    |

Why one per module: once a module is its own library, it must export its own symbols and import those of the modules it uses. With a single macro, the rendering library would also mark the ECS symbols it uses as exported. With `BF_<MODULE>_BUILD` defined only in that module's target, each library exports exactly what it owns.

Today every module is still compiled into the one `byte-forge-engine` library, which defines `BF_ENGINE_BUILD`, so every module macro exports. Splitting a module later only needs a target that defines its `BF_<MODULE>_BUILD`; no header changes.

Modules with only header-only code (math, components, graphics, random) have no macro, because there is nothing to export.

### `BF_STATIC`

Defining `BF_STATIC` (in the engine and in every application that uses it) leaves `BF_EXPORT` and `BF_IMPORT` empty, for building the engine as a static library.

## Usage

Include the module's API header and put its macro between `class` or `struct` and the type name, or before a free function:

```cpp
#include <byte-forge-engine/modules/audio/audio_api.h>

namespace byteforge::audio
{
    class BF_AUDIO_API audio_engine_t
    {
        ...
    };
}

namespace byteforge::scene
{
    BF_SCENE_API bool add_default_2D_systems(scene_context_t& scene_context);
}
```

A new module adds its own `<module>_api.h`, copying one of the existing ones.

## Notes

- Forgetting the macro on a class that has a `.cpp` makes the application fail to link with an unresolved symbol for that class.
- Defining `BF_ENGINE_BUILD` or a `BF_<MODULE>_BUILD` in an application target makes it try to export the engine symbols itself, which also fails at link time.
- Applications return `EXIT_SUCCESS` or `EXIT_FAILURE` from `main`; the engine has no result code type of its own.
