# Scene module

> Language: **English** | [Português (Brasil)](../../../../pt-br/engine/modules/scene/README.md)

The scene module holds the entities of a game world and runs them. It has three classes. `scene_t` owns an ECS registry with the engine's built-in components and draws the scene. `scene_player_t` runs the scene in play mode: it moves entities, checks collisions, plays audio sources and drives scripts, with pause, single step and time scale. `scene_script_system_t` loads one Lua script per entity, calls its callbacks and exposes the Lua API. Everything lives in the `byteforge::scene` namespace. The module is not header-only.

## Files

| File                                                                     | Contents                                                  |
| ------------------------------------------------------------------------ | --------------------------------------------------------- |
| `engine/include/byte-forge-engine/modules/scene/scene.h`                 | `scene_t` declaration                                     |
| `engine/include/byte-forge-engine/modules/scene/scene_player.h`          | `scene_player_t` declaration                              |
| `engine/include/byte-forge-engine/modules/scene/scene_script_system.h`   | `scene_script_system_t` declaration                       |
| `engine/source/modules/scene/scene.cpp`                                  | Component and system registration, defaults, rendering    |
| `engine/source/modules/scene/scene_player.cpp`                           | Play, pause, step, physics and collision passes           |
| `engine/source/modules/scene/scene_script_system.cpp`                    | Script loading, callbacks and every Lua API function      |

Related pieces in other modules:

| File                                                                      | Role                                                      |
| ------------------------------------------------------------------------- | --------------------------------------------------------- |
| `modules/ecs/registry.h`                                                  | Storage for entities, components and systems              |
| `modules/components/*.h`                                                  | The plain data structs the scene registers                |
| `modules/physics/physics_2D.h`                                            | Movement and AABB test used by `scene_player_t`           |
| `modules/audio/audio_engine.h`                                            | Plays audio sources and the Lua `audio` functions         |
| `modules/platform/input/input_state.h`                                    | Keyboard and mouse state read by the Lua `input` functions |
| `modules/rendering/sprite/sprite_renderer.h`                                     | Draws sprites and labels for `scene_t::render()`          |
| `applications/editor/source/editor.cpp`                                   | Owns the scene and the player, runs the frame loop        |

## Concepts

### Scene, registry and entities

`scene_t` is a thin layer over an `ecs::registry_t`. The registry stores the entities, one packed array per component type, and the systems. An entity is an `ecs::entity_t`, which is only a `u64` id. Ids start at `0` and are never reused while the registry lives.

Every entity created through `scene_t::create_entity()` gets a name and a 2D transform. `scene_t::create_empty_entity()` gives only the name. The transform is a normal component: it can be removed, and added back with `add_transform_2D()`. Systems that need it (sprite, label, camera, physics, collision) simply skip entities without one. The other components are optional and are added one by one with the `add_*` functions.

### Components and systems

`scene_t::initialize()` registers nine component types and creates seven systems. A system here is only a list of the entities that have a given set of components. The scene does not attach update functions to them; the code that needs a list reads `system->entities` directly.

| System         | Required components                | Read by                                          |
| -------------- | ---------------------------------- | ------------------------------------------------ |
| `sprite_2D`    | `transform_2D` + `sprite_2D`       | `scene_t::render()`                              |
| `script`       | `script`                           | `scene_script_system_t`                          |
| `physics_2D`   | `transform_2D` + `velocity_2D`     | `scene_player_t` physics pass                    |
| `collision_2D` | `transform_2D` + `box_collider_2D` | `scene_player_t` collision pass                  |
| `audio`        | `audio_source`                     | `scene_player_t::start()`                        |
| `label_2D`     | `transform_2D` + `label_2D`        | `scene_t::render()`                              |
| `camera_2D`    | `transform_2D` + `camera_2D`       | `scene_t::render()`                              |
| `light_2D`     | `transform_2D` + `point_light_2D`  | Render stage: draws the light map (see components) |

The `name` component has no system. Lists are kept up to date when components are added or removed through `scene_t`, because those functions call `registry.refresh_entity_systems()`.

### Coordinates

A transform position is the center of the entity. Sprites are drawn as a quad centered on it, and the collision test treats the collider as a box centered on position plus offset.

Without a camera, one unit is one pixel, the origin is the top-left corner of the viewport and y grows downward. With a primary camera, the camera position becomes the center of the viewport and zoom scales the visible area. Rotation is in degrees.

### Edit mode and play mode

The scene exists all the time. While the player is stopped, the editor edits component data directly and nothing moves. `scene_player_t::start()` switches to play mode: it plays audio sources, enables input and loads the scripts. `update()` then advances physics, collisions and scripts every frame until `stop()`.

`scene_player_t` itself does not copy the scene. The editor does: Play saves the level as text before `start()`, and Stop restores that text, so whatever physics and scripts change (positions, spawned or destroyed entities) is undone. The standalone player never restores.

### Script environments

Each entity with a script runs it in its own Lua table, called its environment. The environment has a metatable whose `__index` is `_G`, so a script can read the standard library and the engine API tables, but every global it assigns lands in its own environment. Two entities using the same file therefore have separate copies of every variable and function. The environments are stored in the Lua registry and referenced by entity id.

## `scene_t`

```cpp
class BF_SCENE_API scene_t
{
public:
    ecs::registry_t registry;

    ecs::component_type_t* name_type;
    ecs::component_type_t* transform_2D_type;
    ecs::component_type_t* sprite_2D_type;
    ecs::component_type_t* script_type;
    ecs::component_type_t* velocity_2D_type;
    ecs::component_type_t* box_collider_2D_type;
    ecs::component_type_t* audio_source_type;
    ecs::component_type_t* label_2D_type;
    ecs::component_type_t* camera_2D_type;

    ecs::system_t* sprite_2D_system;
    ecs::system_t* script_system;
    ecs::system_t* physics_2D_system;
    ecs::system_t* collision_2D_system;
    ecs::system_t* audio_system;
    ecs::system_t* label_2D_system;
    ecs::system_t* camera_2D_system;

    scene_t();

    bool initialize();
    void dispose();

    ecs::entity_t create_entity(const char* name);
    ecs::entity_t create_empty_entity(const char* name);
    void destroy_entity(const ecs::entity_t entity);

    components::name_component_t* get_name(const ecs::entity_t entity);
    // ... one get_* per component type

    void add_transform_2D(const ecs::entity_t entity);
    void add_sprite_2D(const ecs::entity_t entity);
    // ... one add_* per optional component type
    void remove_component(ecs::component_type_t& component_type, const ecs::entity_t entity);

    void render(rendering::sprite_renderer_t& sprite_renderer, rendering::texture_cache_t& texture_cache, rendering::font_cache_t& font_cache, const math::vec2i_t& viewport_size);
};
```

`scene_t` cannot be copied, because `registry_t` deletes its copy operations. There is no destructor; the owner has to call `dispose()`.

### State

| Field          | Meaning                                                                     |
| -------------- | --------------------------------------------------------------------------- |
| `registry`     | The ECS registry with every entity, component and system of the scene      |
| `*_type`       | One pointer per registered component type. `nullptr` before `initialize()` and after `dispose()` |
| `*_system`     | One pointer per system, as listed in Concepts. Same lifetime as the types   |

All fields are public. The editor reads the registry directly for its status panel and ECS observer, and passes the `*_type` pointers to `remove_component()`.

### `initialize()`

1. Registers the nine component types with `registry.register_component_type()`, using the size of each component struct and the names `name`, `transform_2D`, `sprite_2D`, `script`, `velocity_2D`, `box_collider_2D`, `audio_source`, `label_2D` and `camera_2D`. No free function is given, so the components are plain bytes.
2. Returns `false` if any registration returned `nullptr`. The registry logs that error itself when the limit of 64 component types is reached.
3. Builds each system's required bitset from the type ids (`1 << id`) and creates the seven systems.
4. Returns `true`.

### `dispose()`

Disposes the registry, which frees every component type and system and clears all entities, then sets every type and system pointer back to `nullptr`. The id counter of the registry goes back to `0`.

### `create_empty_entity(name)` and `create_entity(name)`

`create_empty_entity(name)`:

1. Creates a new entity in the registry.
2. Builds a name component by copying `name` with `snprintf`, so it is cut to 63 characters plus the terminator.
3. Adds it and calls `registry.spawn_entity()`, which puts the entity into every system that accepts it right away. It does not go through the registry's spawn queue.
4. Returns the entity.

`create_entity(name)` calls `create_empty_entity(name)` and then `add_transform_2D()`. Level files use `create_empty_entity()` and add the transform only when the saved entity has one.

### `destroy_entity(entity)`

Calls `registry.kill_entity()` immediately: the entity is removed from every system, every component it had is removed, its component bitset is cleared and it is erased from the entity list.

### Getters

| Member                       | Returns                                                     |
| ---------------------------- | ----------------------------------------------------------- |
| `get_name(entity)`           | `name_component_t*`                                         |
| `get_transform_2D(entity)`   | `transform_2D_component_t*`                                 |
| `get_sprite_2D(entity)`      | `sprite_2D_component_t*`                                    |
| `get_script(entity)`         | `script_component_t*`                                       |
| `get_velocity_2D(entity)`    | `velocity_2D_component_t*`                                  |
| `get_box_collider_2D(entity)`| `box_collider_2D_component_t*`                              |
| `get_audio_source(entity)`   | `audio_source_component_t*`                                 |
| `get_label_2D(entity)`       | `label_2D_component_t*`                                     |
| `get_camera_2D(entity)`      | `camera_2D_component_t*`                                    |

Each one asks the component type for the entity's data and casts it. The result is `nullptr` when the entity does not have that component. The pointer points into the type's packed array, so it stays valid only until a component of the same type is added or removed.

### `add_*(entity)`

Each `add_*` builds a zero-initialized component, fills in its defaults and passes it to the private `add_component()`, which adds it to the registry and calls `refresh_entity_systems()` so the entity joins the systems that now accept it.

| Member                        | Defaults                                                     |
| ----------------------------- | ------------------------------------------------------------ |
| `add_sprite_2D(entity)`       | No texture, size `(100, 100)`, tint white `(1, 1, 1, 1)`     |
| `add_script(entity)`          | Empty script path                                            |
| `add_velocity_2D(entity)`     | Velocity `(0, 0)`, speed `100`                               |
| `add_box_collider_2D(entity)` | Offset `(0, 0)`, size `(100, 100)`                           |
| `add_audio_source(entity)`    | No clip, volume `1.0`, no loop, no play on start             |
| `add_label_2D(entity)`        | Text `Label`, no font, font size `32`, color white           |
| `add_camera_2D(entity)`       | Zoom `1.0`, `is_primary` set to `true`                       |

If the entity already has that component, the registry replaces it with the defaults. There is no `add_*` for the name, since every entity gets one. `add_transform_2D()` adds a transform with position `(0, 0)`, rotation `0` and scale `(1, 1)`.

### `remove_component(component_type, entity)`

Removes the component from the registry and calls `refresh_entity_systems()`, so the entity leaves every system that no longer accepts it. It works for any type, including name and transform; the editor just does not offer removal for the transform.

### `render(sprite_renderer, texture_cache, font_cache, viewport_size)`

1. Looks for a camera. It walks the `camera_2D` system in order and takes the first entity whose `is_primary` is `true`, reading its transform position and its zoom. If none is found, no camera is used.
2. Calls `sprite_renderer.begin()` with the viewport size, the camera position, the zoom and whether a camera was found.
3. Draws every entity of the `sprite_2D` system. The drawn size is the sprite size multiplied by the transform scale, and the texture comes from `texture_cache.get(texture_path)`. An empty path gives no texture, and the quad is drawn as a flat tinted color.
4. Draws every entity of the `label_2D` system with `sprite_renderer.draw_text()`, at the transform position with the label color. The font comes from `font_cache.get(font_path, font_size)`. When it returns `nullptr` (no font path, size not positive, or a font that failed to load), the label is skipped.

Labels are always drawn after all sprites, so text appears on top. Within each group, the order is the system's entity order.

## `scene_player_t`

```cpp
class BF_SCENE_API scene_player_t
{
public:
    scene_script_system_t script_system;
    time::profiler_t* profiler;
    f32 time_scale;

    bool initialize(lua_State* lua_state, scene_t& scene, audio::audio_engine_t& audio_engine, platform::input_state_t& input_state);
    void start();
    void update(const f64 delta_time);
    void stop();
    void set_paused(const bool value);
    void request_step();
    bool is_running() const;
    bool is_paused() const;
    void dispose();

private:
    void update_physics(const f32 delta_time);
    void update_collisions();

    scene_t* scene;
    audio::audio_engine_t* audio_engine;
    platform::input_state_t* input_state;
    bool running;
    bool paused;
    bool step_requested;
};
```

### State

| Field            | Meaning                                                                 |
| ---------------- | ----------------------------------------------------------------------- |
| `script_system`  | The script system it drives. Public so the editor can read its environment count |
| `profiler`       | Optional profiler for the `physics`, `collisions` and `scripts` sections. `nullptr` by default |
| `time_scale`     | Multiplier applied to every delta time. `1.0` by default                |
| `scene`, `audio_engine`, `input_state` | The objects passed to `initialize()`. Not owned           |
| `running`        | `true` between `start()` and `stop()`                                   |
| `paused`         | `true` while paused. Only meaningful while running                      |
| `step_requested` | `true` when one fixed step should run on the next `update()`            |

### States

```
stopped ──start()──► running ──set_paused(true)──► paused
   ▲                    │  ◄──set_paused(false)──     │
   └──────stop()────────┴──────────stop()─────────────┘
```

While paused, `request_step()` lets exactly one frame advance on the next `update()`.

### `initialize(lua_state, scene, audio_engine, input_state)`

Stores the scene, the audio engine and the input state, then returns the result of `script_system.initialize()` with the same arguments. It does not start anything.

### `start()`

1. Calls `stop()`, so starting while already running restarts from a clean state.
2. Goes through the `audio` system and calls `audio_engine->play()` for every audio source with `play_on_start` set. The returned handles are not kept.
3. Clears the input state and sets `input_state->enabled` to `true`, so input events are recorded again.
4. Calls `script_system.start()`, which loads every script and runs `on_start`.
5. Sets `running` to `true`.

### `update(delta_time)`

1. Returns if not running.
2. Reads `step_requested` into a local and clears it.
3. Returns if paused and no step was requested.
4. Picks the delta time: the fixed `1/60` second when stepping, `delta_time` otherwise. Then multiplies it by `time_scale`.
5. Runs the physics pass with that delta time.
6. Runs the collision pass.
7. Calls `script_system.update()` with the same delta time.

The physics pass goes through the `physics_2D` system and sets every transform position to `physics_2D_t::move(position, velocity, speed, delta_time)`.

The collision pass goes through every pair of entities in the `collision_2D` system, each pair once. For each pair it calls `physics_2D_t::check_aabb_collision()` with the transform position plus the collider offset and the collider size. When they overlap, it calls `script_system.notify_collision()` twice: once with the first entity and the other as argument, then the other way round. Nothing is pushed apart; the pass only reports.

Each pass runs inside a `time::scoped_profile_t`, which does nothing when `profiler` is `nullptr`.

### `stop()`

1. Calls `script_system.stop()`, which drops every script environment.
2. Calls `audio_engine->stop_all()`, which stops every sound in the engine, including those started from Lua.
3. Sets `input_state->enabled` to `false` and clears the input state.
4. Sets `running`, `paused` and `step_requested` to `false`.

### `set_paused(value)` and `request_step()`

`set_paused()` stores the value as is, whether or not the player is running. `request_step()` only sets `step_requested` when the player is running and paused; otherwise it does nothing.

### `is_running()` and `is_paused()`

Return `running` and `paused`. The editor uses them to choose the menu items and the label of the status panel.

### `dispose()`

Calls `stop()` if `initialize()` was called (the scene pointer is set), then disposes the script system and clears the three pointers. It must run while the Lua state and the audio engine are still alive.

## `scene_script_system_t`

```cpp
class BF_SCENE_API scene_script_system_t
{
public:
    bool initialize(lua_State* lua_state, scene_t& scene, audio::audio_engine_t& audio_engine, platform::input_state_t& input_state);
    void start();
    void update(const f64 delta_time);
    void notify_collision(const ecs::entity_t entity, const ecs::entity_t other);
    void stop();
    bool is_running() const;
    usize get_environment_count() const;
    void dispose();

private:
    bool load_script(const ecs::entity_t entity, const char* script_path);
    bool push_function(const ecs::entity_t entity, const char* function_name);
    void invoke(const ecs::entity_t entity, const char* function_name, const i32 argument_count);
    void register_api();

    lua_State* lua_state;
    scene_t* scene;
    audio::audio_engine_t* audio_engine;
    platform::input_state_t* input_state;
    bool running;
    std::unordered_map<u64, i32> entity_environments;
};
```

### State

| Field                 | Meaning                                                              |
| --------------------- | -------------------------------------------------------------------- |
| `lua_state`           | The Lua state, owned by the scripting module of the runtime          |
| `scene`, `audio_engine`, `input_state` | The objects the Lua API works on. Not owned         |
| `running`             | `true` between `start()` and `stop()`                                |
| `entity_environments` | Entity id to the Lua registry reference of that entity's environment |

### Callbacks

A script defines any of these functions. Missing ones are skipped without a message.

| Callback                       | When                                                            | Arguments                                   |
| ------------------------------ | --------------------------------------------------------------- | ------------------------------------------- |
| `on_start(entity)`             | Once, right after the entity's script is loaded in `start()`    | The entity id                               |
| `on_update(entity, delta_time)`| Every frame that `scene_player_t::update()` advances            | The entity id, the scaled delta in seconds  |
| `on_collision(entity, other)`  | Every advancing frame in which the two colliders overlap        | The entity id, the id of the other entity   |

Within one frame the order is: physics, then every `on_collision`, then every `on_update`. Entity ids are passed as Lua integers.

### `initialize(lua_state, scene, audio_engine, input_state)`

Stores the four pointers and calls `register_api()`, which creates the global API tables. It always returns `true`.

### `start()`

1. Calls `stop()` to drop any previous environments.
2. Goes through the `script` system. For each entity whose script path is not empty, it loads the script and, if loading worked and the script defines `on_start`, calls `on_start(entity)`. Each script is loaded and started before the next one is loaded.
3. Sets `running` to `true`.

### `update(delta_time)`

Returns if not running. Otherwise goes through the `script` system and calls `on_update(entity, delta_time)` for every entity that has an environment and defines the function.

### `notify_collision(entity, other)`

If running and the entity has an environment with `on_collision`, calls `on_collision(entity, other)`.

### `stop()`

Releases every environment reference with `luaL_unref`, clears the map and sets `running` to `false`. With no reference left, the environment tables can be collected by Lua's garbage collector.

### `get_environment_count()` and `is_running()`

`get_environment_count()` returns the number of loaded environments, which the editor's status panel shows. It goes back to `0` after `stop()`. `is_running()` returns `running`.

### `dispose()`

Calls `stop()` if a Lua state was set, then clears the four pointers.

### `load_script(entity, script_path)`

1. Compiles the file with `luaL_loadfile`. A relative path is relative to the process working directory. If it fails, logs `Could not load script '<path>' for entity <id>: <message>` and returns `false`.
2. Creates the environment table and a metatable with `__index` set to `_G`.
3. Stores a reference to the environment in the Lua registry.
4. Sets the environment as the first upvalue (`_ENV`) of the compiled chunk.
5. Runs the chunk, which defines the script's functions and variables inside the environment. If it raises an error, logs the same message format, releases the reference and returns `false`.
6. Saves the reference under the entity id and returns `true`.

### `push_function(entity, function_name)` and `invoke(entity, function_name, argument_count)`

`push_function()` looks up the entity's environment, pushes it and then pushes the field `function_name`. If there is no environment or the field is not a function, it pops what it pushed and returns `false`. On success the environment and the function stay on the stack and the caller pushes the arguments.

`invoke()` runs the function with `lua_pcall`. On error it logs `Script function '<name>' failed for entity <id>: <message>` and pops the message. In both cases it then pops the environment. An error does not remove the environment, so the callback is tried again on the next frame.

### `register_api()`

Creates six global tables: `transform`, `velocity`, `label`, `audio`, `log` and `input`. Every function in them gets three light userdata upvalues: the scene (`1`), the audio engine (`2`) and the input state (`3`). The C functions read them back with `lua_upvalueindex`.

### Lua API

Functions that take an entity read it from argument 1 as an integer. If the entity does not have the needed component, they raise the Lua error `entity <id> has no <component>`, where `<component>` is `transform_2D`, `velocity_2D`, `label_2D` or `audio_source`. Wrong argument types raise Lua's standard `bad argument` error. Inside a callback, any of these errors aborts the callback and is logged by `invoke()`.

`transform` table:

| Function                              | Returns    | Description                                       |
| ------------------------------------- | ---------- | ------------------------------------------------- |
| `transform.get_position(entity)`      | `x, y`     | Transform position                                |
| `transform.set_position(entity, x, y)`| nothing    | Sets the transform position                       |
| `transform.get_rotation(entity)`      | `degrees`  | Transform rotation                                |
| `transform.set_rotation(entity, deg)` | nothing    | Sets the transform rotation                       |
| `transform.get_scale(entity)`         | `x, y`     | Transform scale                                   |
| `transform.set_scale(entity, x, y)`   | nothing    | Sets the transform scale                          |

`velocity` table:

| Function                              | Returns    | Description                                       |
| ------------------------------------- | ---------- | ------------------------------------------------- |
| `velocity.get(entity)`                | `x, y`     | Velocity vector                                   |
| `velocity.set(entity, x, y)`          | nothing    | Sets the velocity vector                          |
| `velocity.get_speed(entity)`          | `speed`    | Speed multiplier                                  |
| `velocity.set_speed(entity, speed)`   | nothing    | Sets the speed multiplier                         |

`label` table:

| Function                              | Returns    | Description                                       |
| ------------------------------------- | ---------- | ------------------------------------------------- |
| `label.get_text(entity)`              | `string`   | Label text                                        |
| `label.set_text(entity, text)`        | nothing    | Copies `text`, cut to 255 characters              |

`audio` table, described in detail in the [audio module](../audio/README.md#lua-api):

| Function                              | Returns    | Description                                       |
| ------------------------------------- | ---------- | ------------------------------------------------- |
| `audio.play(entity)`                  | `handle`   | Plays the entity's audio source, `0` on failure   |
| `audio.stop(handle)`                  | nothing    | Stops one sound                                   |

`input` table:

| Function                              | Returns    | Description                                       |
| ------------------------------------- | ---------- | ------------------------------------------------- |
| `input.is_key_down(name)`             | `boolean`  | Key is held                                       |
| `input.is_key_pressed(name)`          | `boolean`  | Key went down this frame                          |
| `input.is_key_released(name)`         | `boolean`  | Key went up this frame                            |
| `input.is_mouse_down(name)`           | `boolean`  | Mouse button is held                              |
| `input.is_mouse_pressed(name)`        | `boolean`  | Mouse button went down this frame                 |
| `input.is_mouse_released(name)`       | `boolean`  | Mouse button went up this frame                   |
| `input.get_mouse_position()`          | `x, y`     | Last position reported by a mouse move event      |
| `input.get_mouse_wheel()`             | `x, y`     | Wheel scroll accumulated this frame               |

Key and mouse button names are matched exactly, case included, against the tables in `platform::input_names_t` (for example `"A"`, `"Space"`, `"ArrowLeft"`, `"Left"`, `"XButton1"`). An unknown name raises `unknown key name '<name>'` or `unknown mouse button name '<name>'`.

`log` table:

| Function                              | Returns    | Description                                       |
| ------------------------------------- | ---------- | ------------------------------------------------- |
| `log.info(message)`                   | nothing    | Logs `[lua] <message>` as info                    |
| `log.warning(message)`                | nothing    | Logs `[lua] <message>` as a warning               |
| `log.error(message)`                  | nothing    | Logs `[lua] <message>` as an error                |

`message` must be a string or a number; a number is converted by `luaL_checkstring`.

`transform.get_world_position(entity)` returns the position after every parent transform is applied (see [Hierarchy](#hierarchy)).

`sprite` table:

| Function                                  | Returns       | Description                                 |
| ----------------------------------------- | ------------- | ------------------------------------------- |
| `sprite.get_texture(entity)`              | `path`        | Texture path                                |
| `sprite.set_texture(entity, path)`        | nothing       | Sets the texture path                       |
| `sprite.get_tint(entity)`                 | `r, g, b, a`  | Tint                                        |
| `sprite.set_tint(entity, r, g, b [, a])`  | nothing       | Sets the tint, alpha defaults to `1`        |
| `sprite.get_size(entity)`                 | `x, y`        | Size before the transform scale             |
| `sprite.set_size(entity, x, y)`           | nothing       | Sets the size                               |

`camera` table: `camera.get_zoom(entity)` and `camera.set_zoom(entity, zoom)`.

`time` table: `time.get_elapsed()` returns the seconds played since `start()` (scaled by the time scale), `time.get_delta()` the delta of the current update.

`scene` table:

| Function                                    | Returns          | Description                                                        |
| ------------------------------------------- | ---------------- | ------------------------------------------------------------------ |
| `scene.create_entity([name])`               | `entity`         | New entity with a name and a transform                             |
| `scene.destroy(entity)`                     | nothing          | Destroys the entity and its children after the current update      |
| `scene.exists(entity)`                      | `boolean`        | The entity was not destroyed                                       |
| `scene.find(name)`                          | `entity` or nil  | First entity with that exact name                                  |
| `scene.find_with_tag(tag)`                  | `{ entity... }`  | Every entity whose tags include `tag`                              |
| `scene.has_tag(entity, tag)`                | `boolean`        | The entity has the tag                                             |
| `scene.get_name(entity)`                    | `string`         | Entity name                                                        |
| `scene.get_parent(entity)`                  | `entity` or nil  | Parent entity                                                      |
| `scene.set_parent(entity, parent or nil)`   | `boolean`        | Sets or clears the parent, keeping the world transform             |
| `scene.get_children(entity)`                | `{ entity... }`  | Direct children                                                    |
| `scene.spawn_stamp(path [, x, y])`         | `entity` or nil  | Instantiates a `.bfstamp` and returns its root                    |
| `scene.load_level(path)`                    | nothing          | Asks the application to load another level after this frame        |

Entities created or spawned during play get their script started on the next update (or right away during `start()`). `destroy` is deferred: the entity is removed after the current stage, so loops over entities are never invalidated and `scene.exists` turns `false` on the next frame. `spawn_stamp` and `load_level` need the application to set `script_system.stamp_loader` and to handle `take_level_request()`; the editor and the player do both.

### Hierarchy

An entity can have a parent through `parent_component_t`. Its `transform_2D` is then **local**: relative to the parent's position, rotation and scale. `scene_t::get_world_transform_2D(entity)` composes the chain: the local position is scaled by the parent scale, rotated by the parent rotation and added to the parent position; rotations add up and scales multiply. A parent without a transform counts as the identity.

The render, light, camera and collision systems use world transforms. Physics moves the local position, so a moving child keeps following its parent. `set_parent` keeps the child where it is on screen by recomputing its local transform; `attach_to_parent` keeps the local values (used when loading levels and stamps). Destroying an entity destroys its whole subtree.

### Tags

`tags_component_t` holds comma-separated tags (`"enemy, flying"`), up to 127 characters. `has_tag` ignores spaces around each tag and compares exactly. `scene_t::find_with_tag` and `find_by_name` scan every entity.

## Usage

The editor (`editor_t`) owns one `scene_t` and one `scene_player_t` as members. In `initialize()` it calls `scene.initialize()` and then `scene_player.initialize()` with the runtime's Lua state, `runtime.audio` and its own `input_state`, and sets `scene_player.profiler`. In `dispose()` it disposes the player first, then the scene, and the runtime last.

Each frame:

```
input_state.begin_frame()
poll events                         -- fills input_state while enabled
scene_player.update(delta_time)     -- physics, collisions, scripts
runtime.audio.update()
scene.render(...)                   -- sprites, then labels
draw UI
```

The main menu bar shows `Play` while stopped, which calls `start()`. While running it shows `Stop`, `Pause` or `Resume` (`set_paused(!is_paused())`), and `Step` while paused (`request_step()`). The status panel offers the same pause and step buttons and a time scale slider from `0` to `4` that writes `time_scale` directly. The scene hierarchy calls `create_entity("Entity")` (Create Entity), `create_empty_entity("Entity")` (Create Empty) and `destroy_entity()`, and the object inspector calls the `add_*` functions and `remove_component()`.

A script from `applications/editor/assets/scripts/rotate.lua`:

```lua
local speed = 5

function on_start(entity)
    log.info("rotate.lua started for entity " .. entity)
end

function on_update(entity, delta_time)
    local rotation = transform.get_rotation(entity)
    transform.set_rotation(entity, rotation + speed * delta_time)
end
```

`speed` is local to the chunk and `on_start` and `on_update` are globals of the entity's environment, so every entity using this file has its own copies.

## Notes

- Scripts are read from disk on every `start()`. Changes to a file take effect the next time Play is pressed.
- Scripts are only loaded in `start()`. An entity that gets a script component, or is created, while the scene is running has no environment and receives no callbacks until the next `start()`.
- A script whose top-level code fails to load or run gets no environment at all. An error inside a callback is logged and the callback runs again on the next frame, so an error in `on_update` is logged every frame.
- `on_collision` fires on every advancing frame while two colliders overlap. There is no separate enter or exit event, and boxes that only touch at an edge count as overlapping.
- The collision box uses only the collider offset and size. Transform scale and rotation are not applied, while sprites are drawn with scale and rotation.
- When stepping, the fixed `1/60` second is also multiplied by `time_scale`. With `time_scale` at `0`, frames still run: scripts and collisions are called with a delta of `0` and nothing moves.
- `add_camera_2D()` marks every new camera as primary. With several primary cameras, `render()` uses the first one in the `camera_2D` system.
- The missing component error formats the entity id with `%d` after casting it to `i32`.
- `register_api()` sets the globals `transform`, `velocity`, `label`, `audio`, `log` and `input` in the shared Lua state, replacing any value they had. A script that assigns one of these names only shadows it in its own environment.
