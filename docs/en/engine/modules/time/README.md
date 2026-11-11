# Time module

> Language: **English** | [Português (Brasil)](../../../../pt-br/engine/modules/time/README.md)

The time module measures frames and code sections. It has two parts: a frame manager that computes `delta_time` and caps the frame rate using SDL's millisecond clock, and a small profiler that times named sections with `std::chrono::steady_clock` and keeps a running average. It lives in the `byteforge::time` namespace. `frame_settings_t`, `time_converter_t`, `profiler_sample_t` and `scoped_profile_t` are header-only; `frame_manager_t` and `profiler_t` have source files.

## Files

| File                                                               | Contents                                          |
| ------------------------------------------------------------------ | ------------------------------------------------- |
| `engine/include/byte-forge-engine/modules/time/frame_settings.h`   | `frame_settings_t`, the target frame rate         |
| `engine/include/byte-forge-engine/modules/time/frame_manager.h`    | `frame_manager_t` declaration                     |
| `engine/include/byte-forge-engine/modules/time/time_converter.h`   | `time_converter_t`, seconds and milliseconds      |
| `engine/include/byte-forge-engine/modules/time/profiler.h`         | `profiler_sample_t`, `profiler_t`, `scoped_profile_t` |
| `engine/source/modules/time/frame_manager.cpp`                     | Delta time, frame cap, elapsed time               |
| `engine/source/modules/time/profiler.cpp`                          | Sample lookup and averaging                       |

Related pieces in other modules:

| File                                                                  | Role                                                |
| --------------------------------------------------------------------- | --------------------------------------------------- |
| `modules/runtime/runtime.h`                                           | Owns `frame_settings` and `frame_manager`           |
| `modules/scene/scene_player.cpp`                                      | Profiles `physics`, `collisions` and `scripts`      |
| `applications/editor/source/editor.cpp`                               | Owns the profiler, drives the frame manager         |
| `applications/editor/source/ui/components/status_panel_component.cpp` | Shows FPS, frame time and the profiler phases       |

## Concepts

### Two clocks

The frame manager reads `SDL_GetTicks()`, which returns whole milliseconds since SDL was initialized. Every value it computes is therefore a multiple of one millisecond before conversion. The profiler reads `std::chrono::steady_clock`, which has sub-millisecond resolution, and reports milliseconds as `f64`.

### What `delta_time` measures

`delta_time` is the time between the ends of two consecutive frames, in seconds:

```
frame N-1                       frame N
start ... end | delay |         start ... end | delay |
              ^------------------------------^
                     delta_time after frame N
```

`end_frame()` takes the tick count before it sleeps, so the measured span covers the previous frame's sleep and the current frame's work. The value is written at the end of a frame and read by the next frame's update, so the update always uses the duration of the frame before it.

### Frame cap

The frame manager limits the frame rate by sleeping. If a frame's own work (from `start_frame()` to `end_frame()`) took less than the target frame time, `end_frame()` calls `SDL_Delay` for the difference. A frame that took longer does not sleep and is not compensated later.

## `frame_settings_t`

```cpp
struct frame_settings_t
{
public:
    u8 target_fps;

    explicit frame_settings_t(u8 target_fps) : target_fps(target_fps) {}
};
```

### State

| Field        | Meaning                                                                  |
| ------------ | ------------------------------------------------------------------------ |
| `target_fps` | Frames per second the frame manager aims for. A `u8`, so at most `255`   |

## `frame_manager_t`

```cpp
struct BF_TIME_API frame_manager_t
{
public:
    f64 delta_time;

    void reset(const frame_settings_t& frame_settings);
    void start_frame();
    void end_frame();
    f64 get_elapsed_time() const;

private:
    u64 first_frame_tick_count;
    u64 last_frame_tick_count;
    u64 current_frame_start_tick_count;
    u64 current_frame_end_tick_count;
    f64 target_frame_time_in_ms;
};
```

The constructor sets every field to zero.

### State

| Field                            | Meaning                                                         |
| -------------------------------- | --------------------------------------------------------------- |
| `delta_time`                     | Seconds between the ends of the last two frames, capped at `0.05` |
| `first_frame_tick_count`         | Tick count when `reset()` ran. Origin of the elapsed time       |
| `last_frame_tick_count`          | Tick count at the end of the previous frame                     |
| `current_frame_start_tick_count` | Tick count when the current frame started                       |
| `current_frame_end_tick_count`   | Tick count at the end of the last completed frame               |
| `target_frame_time_in_ms`        | `1000 / target_fps`, the frame budget in milliseconds           |

### `reset(frame_settings)`

Reads the current tick count into `first_frame_tick_count` and `last_frame_tick_count`, zeroes the start and end counts and `delta_time`, and sets `target_frame_time_in_ms` to `sec_to_ms(1.0 / target_fps)`. `runtime_t::initialize()` calls it as its last step.

### `start_frame()`

Stores the current tick count as the start of the frame. Call it at the top of the loop, before any work that should count toward the frame budget.

### `end_frame()`

1. Reads the current tick count as the end of the frame.
2. Sets `delta_time` to the time since the previous frame's end, converted to seconds and capped at `0.05` (the `max_delta_time` constant).
3. Stores the end as the previous frame's end for the next call.
4. Computes the frame's own duration (end minus start) in milliseconds. If it is below `target_frame_time_in_ms`, sleeps with `SDL_Delay` for the difference, cast to `u32`, which drops the fractional part.

### `get_elapsed_time()`

Returns the seconds between `reset()` and the end of the last completed frame. It does not read the clock, so it only advances when `end_frame()` runs.

## `time_converter_t`

```cpp
struct time_converter_t
{
public:
    constexpr static f64 MS_PER_SEC = 1000.0;

    constexpr static f64 sec_to_ms(const f64 secs);
    constexpr static f64 ms_to_sec(const f64 ms);
};
```

| Member         | Returns                     |
| -------------- | --------------------------- |
| `MS_PER_SEC`   | `1000.0`                    |
| `sec_to_ms(s)` | `s * 1000.0`                |
| `ms_to_sec(m)` | `m / 1000.0`                |

Both functions are `constexpr` and can be used in constant expressions.

## `profiler_sample_t`

```cpp
struct profiler_sample_t
{
public:
    std::string name;
    std::chrono::steady_clock::time_point start;
    f64 last_ms;
    f64 average_ms;
};
```

### State

| Field        | Meaning                                                                |
| ------------ | ---------------------------------------------------------------------- |
| `name`       | The section's name, as passed to `begin()` or `end()`                  |
| `start`      | When the last `begin()` for this name ran                              |
| `last_ms`    | Duration of the last `begin()` to `end()` span, in milliseconds        |
| `average_ms` | Exponential moving average of `last_ms`                                |

## `profiler_t`

```cpp
class BF_TIME_API profiler_t
{
public:
    void begin(const char* name);
    void end(const char* name);
    usize get_sample_count() const;
    const profiler_sample_t& get_sample(const usize index) const;
    const profiler_sample_t* find(const char* name) const;

private:
    profiler_sample_t* find_or_create(const char* name);

    std::vector<profiler_sample_t> samples;
};
```

### State

| Field     | Meaning                                                                 |
| --------- | ----------------------------------------------------------------------- |
| `samples` | One sample per distinct name, in the order each name was first seen    |

### `begin(name)`

Finds the sample called `name`, creating it if it does not exist, and sets its `start` to now.

### `end(name)`

1. Finds the sample called `name`, creating it if it does not exist.
2. Sets `last_ms` to the time since its `start`, in milliseconds.
3. Updates `average_ms`. If it is still `0.0` (the first measurement), it becomes `last_ms`. Otherwise it becomes `average_ms * 0.9 + last_ms * 0.1`, so each new value moves the average by 10 percent.

### `find(name)`

Returns a pointer to the sample called `name`, or `nullptr`. It never creates one. The pointer points into the vector and is invalidated when a new name is added.

### `get_sample_count()` and `get_sample(index)`

The number of samples and the sample at `index`. `get_sample` does not check the index.

### `find_or_create(name)` (private)

Searches the vector by comparing each sample's `name` with `name`. If none matches, appends a sample with `start` set to now and both times at `0.0`. Lookup is linear, which is fine for the handful of names the editor uses.

## `scoped_profile_t`

```cpp
struct scoped_profile_t
{
public:
    scoped_profile_t(profiler_t* profiler, const char* name);
    ~scoped_profile_t();

    scoped_profile_t(const scoped_profile_t&)            = delete;
    scoped_profile_t& operator=(const scoped_profile_t&) = delete;

private:
    profiler_t* profiler;
    const char* name;
};
```

Calls `profiler->begin(name)` in its constructor and `profiler->end(name)` in its destructor, so it measures the scope it lives in. If `profiler` is `nullptr`, both do nothing. This lets engine code such as the scene player profile its sections only when an application has handed it a profiler. It keeps the `name` pointer, so the string must outlive the object.

## Usage

`runtime_t` owns `frame_settings` and `frame_manager`, and resets the frame manager at the end of `initialize()`. The editor drives the frame manager and owns a `profiler_t`:

```cpp
while (!runtime.should_close)
{
    profiler.begin(frame_profile_name);
    runtime.frame_manager.start_frame();

    {
        time::scoped_profile_t update_profile(&profiler, update_profile_name);
        scene_player.update(runtime.frame_manager.delta_time);
        runtime.audio.update();
    }

    // events, scene render and ui are profiled the same way

    runtime.frame_manager.end_frame();
    profiler.end(frame_profile_name);
}
```

The editor sets `scene_player.profiler = &profiler`, and the scene player then adds `physics`, `collisions` and `scripts` samples while a scene is playing. The editor's own samples are `frame`, `events`, `update`, `scene render` and `ui`.

The status panel reads the profiler:

- FPS is `1000 / average_ms` of the `frame` sample, shown next to `frame_settings.target_fps`.
- "Frame time" is the `frame` sample's `last_ms`, and the last 120 values are plotted.
- "Work" is the sum of `last_ms` of every sample except `frame`, and "waiting" is the frame time minus that sum.
- The "Phases" table lists every sample except `frame` with its last and average times.

## Notes

- The editor declares its target as `const u16 target_fps = 500` and passes it to `frame_settings_t`, which takes a `u8`. The conversion keeps the low 8 bits, so the stored target is `244`, and that is the value the status panel shows.
- A `target_fps` of `0` makes `1.0 / target_fps` infinite, and the delay in `end_frame()` is then cast from an infinite value.
- Because ticks are whole milliseconds and the sleep drops fractions, at high targets the actual frame time rounds to whole milliseconds. With the editor's `244`, the budget is about `4.1` ms and the sleep is at most `4` ms.
- `delta_time` is `0.0` during the first frame after `reset()`. The first non-zero value includes everything that ran between `reset()` and the first `end_frame()`, capped at `0.05` seconds.
- The cap means that a slow frame (over 50 ms) makes the simulation advance less than real time.
- `get_elapsed_time()` subtracts `first_frame_tick_count` from `current_frame_end_tick_count` as unsigned integers. Before the first `end_frame()`, the end count is `0`, so the subtraction wraps and the result is a very large number.
- Profiler samples are never removed. A name that stops being measured keeps its last values; for example the scene player's samples remain in the status panel's "Work" sum after Stop.
- The scene player's samples run inside the editor's `update` sample, so the status panel's "Work" sum counts that time twice while a scene is playing.
- The profiler is not thread-safe and keeps one `start` per name, so the same name cannot be nested or overlapped.
