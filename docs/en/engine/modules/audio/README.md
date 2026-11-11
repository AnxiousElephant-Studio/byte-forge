# Audio module

> Language: **English** | [Português (Brasil)](../../../../pt-br/engine/modules/audio/README.md)

The audio module plays sound files. It is a thin layer over [miniaudio](https://miniaud.io): one class, `audio_engine_t`, that loads a file, starts it and hands back a number (a handle) to stop it later. It lives in the `byteforge::audio` namespace.

## Files

| File                                                              | Contents                                         |
| ----------------------------------------------------------------- | ------------------------------------------------ |
| `engine/include/byte-forge-engine/modules/audio/audio_engine.h`   | `audio_engine_t` declaration                     |
| `engine/source/modules/audio/audio_engine.cpp`                    | Implementation and the miniaudio implementation  |

Related pieces in other modules:

| File                                                                      | Role                                              |
| ------------------------------------------------------------------------- | ------------------------------------------------- |
| `modules/components/audio_source_component.h`                             | Data an entity needs to play a clip               |
| `modules/scene/scene_player.cpp`                                          | Plays "play on start" sources, stops everything   |
| `modules/scene/scene_script_system.cpp`                                   | Lua `audio.play` and `audio.stop`                 |
| `modules/runtime/runtime.h`                                               | Owns the engine instance and its lifetime         |

## Concepts

### How miniaudio is hidden

The header only forward-declares the two miniaudio types it stores:

```cpp
struct ma_engine;
struct ma_sound;
```

`audio_engine_t` keeps pointers to them, so the header does not include `miniaudio.h`. Applications that include `audio_engine.h` never see miniaudio and don't need it on their include path.

miniaudio is a single-header library. `audio_engine.cpp` defines `MINIAUDIO_IMPLEMENTATION` before including it, which makes that file compile the whole library into the engine DLL. No other file may define it, or the library would be compiled twice.

### Handles

A handle is a `u32` that identifies one playback. Handle `0` means "invalid" and is what `play()` returns when it fails.

Handles start at `1` and only go up. A handle is never reused, so a stale handle from a sound that already ended can't stop a newer sound by accident. `stop()` and `is_playing()` simply don't find it.

Each `play()` call creates a separate playback with its own handle, even for the same file. Playing a clip twice gives two overlapping sounds.

### Lifecycle

```
initialize() ──► play() / stop() / update() ... ──► dispose()
```

## `audio_engine_t`

```cpp
class BF_AUDIO_API audio_engine_t
{
public:
    bool  initialize();
    u32   play(const char* path, const f32 volume, const bool loop);
    void  stop(const u32 handle);
    void  stop_all();
    bool  is_playing(const u32 handle) const;
    void  update();
    usize get_active_count() const;
    void  dispose();

private:
    ma_engine*                         engine;
    std::unordered_map<u32, ma_sound*> sounds;
    u32                                next_handle;
};
```

The copy constructor and copy assignment are deleted. A copy would share the same `ma_engine` and `ma_sound` pointers, and whichever copy was disposed first would free them under the other.

There is no destructor. Cleanup happens only in `dispose()`, which the owner has to call.

### State

| Field         | Meaning                                                             |
| ------------- | ------------------------------------------------------------------- |
| `engine`      | The miniaudio engine. `nullptr` before `initialize()` and after `dispose()` |
| `sounds`      | Every sound started and not yet removed, keyed by its handle        |
| `next_handle` | The handle the next `play()` will return. Starts at `1`             |

### `initialize()`

Creates the `ma_engine` with `ma_engine_init(nullptr, engine)`. The `nullptr` config means miniaudio's defaults: the system's default playback device, its native sample rate and channel count, and miniaudio's own mixing thread.

If initialization fails (for example, no audio device), it logs `Could not initialize the audio engine (<code>).`, frees the engine and returns `false`. `<code>` is miniaudio's `ma_result`.

### `play(path, volume, loop)`

1. Returns `0` right away if the engine was not initialized, or `path` is null or empty.
2. Creates a new `ma_sound` and loads the file with `ma_sound_init_from_file`. Formats are whatever miniaudio decodes out of the box: WAV, FLAC and MP3.
3. If loading fails, logs `Could not load audio '<path>' (<code>).` and returns `0`.
4. Sets the volume and looping, starts the sound, stores it under a new handle and returns the handle.

`path` is opened as given. A relative path is relative to the process working directory, which for the editor is `applications/editor` (`set_rundir` in its `xmake.lua`).

`volume` is a linear multiplier: `1.0` is the file's original level, `0.5` is half the amplitude, `0.0` is silent. Values above `1.0` amplify and can clip.

The file is loaded on the calling thread, so `play()` blocks until the file is open and ready.

### `stop(handle)`

Looks the handle up. If it exists, stops and frees the sound (`ma_sound_uninit`) and removes it from the map. An unknown or already removed handle does nothing.

### `stop_all()`

Frees every sound in the map and clears it. The engine itself stays alive, so `play()` keeps working afterwards.

### `update()`

Removes sounds that reached their end. It collects the handles whose `ma_sound_at_end` is true, then calls `stop()` on each. The two passes are there because `stop()` erases from the map, and erasing while iterating over the same map would invalidate the loop.

A looping sound never reaches its end, so `update()` never removes it. It plays until `stop()`, `stop_all()` or `dispose()`.

`update()` has to be called regularly, once per frame. Without it, sounds that finished stay in memory, and `get_active_count()` keeps counting them. The editor calls it every frame right after `scene_player.update()`.

### `is_playing(handle)`

`true` only if the handle is in the map **and** miniaudio reports it as playing. Unknown handles, removed handles and sounds that ended but haven't been cleaned by `update()` yet all return `false`.

### `get_active_count()`

The number of sounds in the map. Between two `update()` calls this can include sounds that just ended. The editor's status panel shows this value.

### `dispose()`

Calls `stop_all()`, then shuts down and frees the `ma_engine`. Calling it twice is safe, since the second call finds an empty map and a null engine.

## Usage

### Who owns it

`runtime_t` has an `audio_engine_t audio` member.

- `runtime_t::initialize()` initializes the platform, then scripting, then audio. If any of them fails, it returns `false`.
- `runtime_t::dispose()` disposes audio **first**, before scripting, the renderer and the platform.

Other code receives a reference to `runtime.audio` and never creates its own.

### The audio source component

An entity plays sound through `components::audio_source_component_t`:

```cpp
constexpr usize AUDIO_CLIP_PATH_MAX_LENGTH = 256;

struct audio_source_component_t
{
    char clip_path[AUDIO_CLIP_PATH_MAX_LENGTH];
    f32  volume;
    bool loop;
    bool play_on_start;
};
```

| Field           | Default   | Meaning                                              |
| --------------- | --------- | ---------------------------------------------------- |
| `clip_path`     | empty     | File to play, up to 255 characters plus the terminator |
| `volume`        | `1.0`     | Passed to `play()` as is                             |
| `loop`          | `false`   | Passed to `play()` as is                             |
| `play_on_start` | `false`   | Play automatically when the scene starts             |

The defaults come from `scene_t::add_audio_source()`. The component only holds settings. It does not store a handle, so the engine doesn't know which sounds belong to which entity.

The `audio` system in `scene_t` matches every entity with this component. In the editor the fields are edited in the object inspector.

### Scene playback

`scene_player_t` uses the engine in two places:

- `start()` first calls `stop()`, then goes through every entity in the audio system and calls `play()` for each one with `play_on_start` set. The handles it gets back are not kept.
- `stop()` calls `stop_all()`. This stops **every** sound in the engine, including ones started from Lua.

### Lua API

Scripts get an `audio` table:

| Function             | Returns  | Description                                            |
| -------------------- | -------- | ------------------------------------------------------ |
| `audio.play(entity)` | handle   | Plays the entity's audio source with its clip, volume and loop |
| `audio.stop(handle)` | nothing  | Stops one sound                                        |

`audio.play` raises a Lua error if the entity has no audio source component. If the file can't be loaded it does not raise an error; it returns `0`.

The script has to keep the handle if it wants to stop the sound later:

```lua
function update_audio(entity)
    if input.is_key_pressed("P") then
        sound_handle = audio.play(entity)
    end

    if input.is_key_pressed("O") and sound_handle ~= 0 then
        audio.stop(sound_handle)
        sound_handle = 0
    end
end
```

Lua reaches the engine through an upvalue: when the tables are registered, a light userdata pointing to the `audio_engine_t` is stored as upvalue `2` of each function, and `get_audio_engine()` reads it back.

### Frame order in the editor

```
poll events
scene_player.update(delta_time)   -- scripts may call audio.play / audio.stop
runtime.audio.update()            -- removes sounds that ended
render
```

Mixing does not follow this loop. miniaudio mixes on its own thread, so a started sound keeps playing at the same pace even if a frame takes long.

## Notes

- `update()` has to run every frame. Without it, finished sounds stay in memory and `get_active_count()` doesn't go down.
- `play()` loads the file on the calling thread, so a large file holds that frame until it is ready.
- Handles of "play on start" sounds are not kept by anyone, so only `stop_all()` can stop them.
- `scene_player_t::stop()` stops every sound in the engine, including ones started from Lua.
- There is no destructor. If `dispose()` is never called, the sounds and the miniaudio engine are never freed.
