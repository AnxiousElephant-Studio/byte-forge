# Components module

> Language: **English** | [Português (Brasil)](../../../../pt-br/engine/modules/components/README.md)

The components module holds the data structs that entities carry: a name, a 2D transform, a sprite, a label, a velocity, a box collider, a camera, an audio source and a script. Each one is a plain struct with no functions. The module is header-only, lives in the `byteforge::components` namespace and depends only on the engine's basic types and the math vectors.

The structs don't do anything by themselves. The scene registers each one as an ECS component type, gives new components their default values, and the scene player, the renderer, the Lua API and the editor read and write the fields.

## Files

| File                                                                         | Contents                                         |
| ---------------------------------------------------------------------------- | ------------------------------------------------ |
| `engine/include/byte-forge-engine/modules/components/name_component.h`       | `name_component_t`, `NAME_MAX_LENGTH`            |
| `engine/include/byte-forge-engine/modules/components/transform_2D_component.h` | `transform_2D_component_t`                     |
| `engine/include/byte-forge-engine/modules/components/sprite_2D_component.h`  | `sprite_2D_component_t`, `TEXTURE_PATH_MAX_LENGTH` |
| `engine/include/byte-forge-engine/modules/components/label_2D_component.h`   | `label_2D_component_t`, `LABEL_TEXT_MAX_LENGTH`, `FONT_PATH_MAX_LENGTH` |
| `engine/include/byte-forge-engine/modules/components/velocity_2D_component.h` | `velocity_2D_component_t`                       |
| `engine/include/byte-forge-engine/modules/components/box_collider_2D_component.h` | `box_collider_2D_component_t`               |
| `engine/include/byte-forge-engine/modules/components/camera_2D_component.h`  | `camera_2D_component_t`                          |
| `engine/include/byte-forge-engine/modules/components/point_light_2D_component.h` | `point_light_2D_component_t`                |
| `engine/include/byte-forge-engine/modules/components/audio_source_component.h` | `audio_source_component_t`, `AUDIO_CLIP_PATH_MAX_LENGTH` |
| `engine/include/byte-forge-engine/modules/components/script_component.h`     | `script_component_t`, `SCRIPT_PATH_MAX_LENGTH`   |

Related pieces in other modules:

| File                                                     | Role                                                            |
| -------------------------------------------------------- | --------------------------------------------------------------- |
| `modules/ecs/`                                           | Stores the components as raw bytes                              |
| `modules/scene/scene.cpp`                                | Registers the types, sets defaults, draws sprites and labels    |
| `modules/scene/scene_player.cpp`                         | Moves entities, checks collisions, plays audio on start         |
| `modules/scene/scene_script_system.cpp`                  | Runs scripts and exposes transform, velocity, label and audio to Lua |
| `applications/editor/.../object_inspector_component.cpp` | Edits the fields, adds and removes components                   |

## Concepts

### Plain data, copied as bytes

The ECS stores components by copying their bytes with `std::memcpy`, and the scene registers every type in this module without a free function. All the structs are made only of numbers, `bool`s, math vectors and fixed-size `char` arrays, so a byte copy is a complete copy and there is nothing to free.

Text fields are `char` arrays with a fixed capacity instead of `std::string`. Each capacity is a named constant next to the struct. The longest string a field can hold is the capacity minus one, for the terminator. The code that writes them (`std::snprintf` in the scene and in Lua, `ImGui::InputText` in the editor) is given the capacity and cuts longer text short.

### Defaults come from the scene

The structs have no constructors and no default member values. The defaults are set by `scene_t` when it adds a component: it starts from a zeroed struct (`= {}`) and assigns the fields that should not be zero. The tables below list those defaults.

### Every entity has a name

Every entity has a `name_component_t`. `scene_t::create_entity()` also adds a `transform_2D_component_t`, and `create_empty_entity()` does not. Every other component, the transform included, is optional and is added one at a time, from the editor's Add Component menu, through `scene_t::add_transform_2D()`, `add_sprite_2D()` and the other `add_*` functions. The editor does not offer a Remove Component button for the name.

### Units and coordinates

Positions and sizes are in world units. Without a camera, one unit is one pixel and the origin is the top-left corner of the viewport. With a primary camera, the camera's position is the center of the viewport and its zoom scales the visible area. Rotation is in degrees. Colors are RGBA with each channel from `0` to `1`.

## `name_component_t`

```cpp
constexpr usize NAME_MAX_LENGTH = 64;

struct name_component_t
{
    char name[NAME_MAX_LENGTH];
};
```

### State

| Field  | Default                          | Meaning                                  |
| ------ | -------------------------------- | ---------------------------------------- |
| `name` | The name passed to `create_entity()` | Display name, up to 63 characters    |

### Who reads it

The name is only for people. The scene hierarchy and the ECS Observer list entities by it, showing `<unnamed>` if an entity has no name component. The object inspector shows it as a text field at the top, outside any component header. Nothing looks entities up by name, and two entities can share one.

## `transform_2D_component_t`

```cpp
struct transform_2D_component_t
{
    math::vec2_t position;
    math::vec2_t scale;
    f32          rotation;
};
```

### State

| Field      | Default      | Meaning                                         |
| ---------- | ------------ | ----------------------------------------------- |
| `position` | `(0, 0)`     | Where the entity is, in world units             |
| `scale`    | `(1, 1)`     | Multiplies the sprite size                      |
| `rotation` | `0`          | Rotation in degrees                             |

### Who reads it

Every system except `script` and `audio` requires it. How each one uses it:

| Reader                    | Uses                                                          |
| ------------------------- | ------------------------------------------------------------- |
| Sprite rendering          | `position` as the sprite center, `rotation`, `scale` times the sprite size |
| Label rendering           | `position` only                                               |
| Physics                   | Writes `position` every update                                |
| Collisions                | `position` plus the collider offset                           |
| Camera                    | `position` as the center of the view                          |

Lua reads and writes it through the `transform` table: `get_position`, `set_position`, `get_rotation`, `set_rotation`, `get_scale` and `set_scale`. Vectors are passed as two numbers, for example `transform.set_position(entity, x, y)`.

## `sprite_2D_component_t`

```cpp
constexpr usize TEXTURE_PATH_MAX_LENGTH  = 256;
constexpr usize MATERIAL_PATH_MAX_LENGTH = 256;

struct sprite_2D_component_t
{
    char         texture_path[TEXTURE_PATH_MAX_LENGTH];
    math::vec2_t size;
    math::vec4_t tint;
    char         material_path[MATERIAL_PATH_MAX_LENGTH];
    bool         is_lit;
};
```

### State

| Field          | Default        | Meaning                                              |
| -------------- | -------------- | ---------------------------------------------------- |
| `texture_path` | empty          | Image file to draw, up to 255 characters             |
| `size`         | `(100, 100)`   | Size in world units before the transform's scale     |
| `tint`         | `(1, 1, 1, 1)` | Multiplies the texture, or fills the quad without one |
| `material_path` | empty         | Material file (`surface()`); empty uses the sprite shader |
| `is_lit`       | `false`        | Receives point lights: uses the `Lit` shading model, with the default sprite material when `material_path` is empty |

### Who reads it

`scene_t::render()` draws every entity in the `sprite_2D` system, which requires a transform and a sprite. For each one it multiplies `size` by the transform's `scale` component by component, then draws a quad centered on the transform's `position`, rotated by its `rotation`, with `tint`.

The texture comes from `texture_cache_t::get(texture_path)`. With an empty path the cache returns `nullptr`, and the sprite shader then fills the quad with the `tint` color alone. With a texture, the texture color is multiplied by `tint`.

With a `material_path`, the sprite is drawn with that material instead (`sprite_renderer_t::draw_with_material()`), taken from `material_cache_t` with the shading model chosen by `is_lit`. If the material fails to load, the sprite falls back to the sprite shader. See the [rendering module](../rendering/README.md) for what a material receives.

Sprites are drawn in the order of the `sprite_2D` system list, which is the order the components were added. Lua has no access to sprites.

## `label_2D_component_t`

```cpp
constexpr usize LABEL_TEXT_MAX_LENGTH = 256;
constexpr usize FONT_PATH_MAX_LENGTH  = 256;

struct label_2D_component_t
{
    char         text[LABEL_TEXT_MAX_LENGTH];
    char         font_path[FONT_PATH_MAX_LENGTH];
    f32          font_size;
    math::vec4_t color;
};
```

### State

| Field       | Default        | Meaning                                         |
| ----------- | -------------- | ----------------------------------------------- |
| `text`      | `"Label"`      | Text to draw, up to 255 characters              |
| `font_path` | empty          | Font file, up to 255 characters                 |
| `font_size` | `32`           | Size the font is baked at                       |
| `color`     | `(1, 1, 1, 1)` | Text color                                      |

### Who reads it

`scene_t::render()` draws every entity in the `label_2D` system after all the sprites. It asks `font_cache_t::get(font_path, font_size)` for a font and skips the label if it gets `nullptr`, which happens when the path is empty, the size is `0` or less, or the font failed to load. A new label with the default empty font path is therefore not drawn until a font is set.

The text starts at the transform's `position.x`, with its baseline `font_size` units below `position.y`. The transform's scale and rotation are not applied to labels.

Lua reads and writes the text through `label.get_text(entity)` and `label.set_text(entity, text)`. The editor edits the text, the font path, the size (a slider from `4` to `256`) and the color.

## `velocity_2D_component_t`

```cpp
struct velocity_2D_component_t
{
    math::vec2_t velocity;
    f32          speed;
};
```

### State

| Field      | Default  | Meaning                                       |
| ---------- | -------- | --------------------------------------------- |
| `velocity` | `(0, 0)` | Direction and relative amount of movement     |
| `speed`    | `100`    | Multiplier applied to `velocity`              |

### Who reads it

While the scene is playing, `scene_player_t` moves every entity in the `physics_2D` system (transform and velocity) each update:

```
position = position + velocity * speed * delta_time
```

`delta_time` is the frame time, or a fixed 1/60 of a second for a single step while paused, multiplied by the player's time scale. With the defaults, an entity whose `velocity` is `(1, 0)` moves 100 units per second to the right. Nothing normalizes `velocity`, so its length also scales the movement.

Lua reads and writes both fields through the `velocity` table: `get`, `set`, `get_speed` and `set_speed`.

## `box_collider_2D_component_t`

```cpp
struct box_collider_2D_component_t
{
    math::vec2_t offset;
    math::vec2_t size;
};
```

### State

| Field    | Default      | Meaning                                          |
| -------- | ------------ | ------------------------------------------------ |
| `offset` | `(0, 0)`     | Box center relative to the transform's position  |
| `size`   | `(100, 100)` | Full width and height of the box                 |

### Who reads it

While the scene is playing, `scene_player_t` tests every pair of entities in the `collision_2D` system (transform and box collider) each update. Each box is centered on `position + offset` and is `size` wide and tall. The transform's scale and rotation are not applied to it. Boxes that touch at an edge count as overlapping.

For each overlapping pair, the script system calls `on_collision(entity, other)` on both entities' scripts, once in each direction. This repeats every update while the boxes overlap. Colliders do not push entities apart.

Lua has no direct access to colliders.

## `camera_2D_component_t`

```cpp
struct camera_2D_component_t
{
    f32  zoom;
    bool is_primary;
};
```

### State

| Field        | Default | Meaning                                                   |
| ------------ | ------- | --------------------------------------------------------- |
| `zoom`       | `1`     | Scale of the view. `2` shows half the width and height    |
| `is_primary` | `true`  | Whether this camera may be used for rendering             |

### Who reads it

At the start of `scene_t::render()`, the scene walks the `camera_2D` system and takes the first entity whose camera has `is_primary` set. Its transform's `position` becomes the center of the view and its `zoom` scales the visible area: the view is the viewport size divided by `zoom`. If no camera is primary, or the chosen camera's `zoom` is `0` or less, the renderer falls back to pixel coordinates with the origin at the top-left.

The camera's transform rotation and scale are not used. The editor clamps `zoom` between `0.01` and `100` in its drag field. Lua has no access to cameras.

## `point_light_2D_component_t`

```cpp
struct point_light_2D_component_t
{
    math::vec4_t color;
    f32          radius;
    f32          intensity;
};
```

### State

| Field       | Default             | Meaning                                         |
| ----------- | ------------------- | ----------------------------------------------- |
| `color`     | `(1, 0.85, 0.6, 1)` | Color the light adds                            |
| `radius`    | `250`               | Distance in world units where the light reaches zero |
| `intensity` | `1`                 | Multiplies the color                            |

### Who reads it

The `light_2D` render system walks every entity with a transform and a point light, after the camera set the projection. It clears the light map to the ambient color of `scene_render_context_t` and adds each light with `point_light_renderer_t`: the light is brightest at the transform's `position` and fades linearly to zero at `radius`. Only sprites with `is_lit` set read the light map; everything else is not affected by lights.

## `tags_component_t` and `parent_component_t`

```cpp
struct tags_component_t
{
    char tags[TAGS_MAX_LENGTH];
    bool has_tag(const char* tag) const;
};

struct parent_component_t
{
    ecs::entity_t parent;
};
```

`tags` is a comma-separated list such as `"enemy, flying"`; Lua finds tagged entities with `scene.find_with_tag(tag)`. `parent_component_t` makes the entity's transform local to `parent`. Both are explained in the [scene module](../scene/README.md#hierarchy). The editor edits tags in the inspector and parents by dragging entities in the Scene Hierarchy.

## `audio_source_component_t`

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

### State

| Field           | Default | Meaning                                                 |
| --------------- | ------- | ------------------------------------------------------- |
| `clip_path`     | empty   | Sound file to play, up to 255 characters                |
| `volume`        | `1`     | Linear volume passed to the audio engine                |
| `loop`          | `false` | Whether the sound repeats                               |
| `play_on_start` | `false` | Play automatically when the scene starts                |

### Who reads it

When the scene player starts, it walks the `audio` system and calls `audio_engine_t::play(clip_path, volume, loop)` for every source with `play_on_start` set. Lua calls the same function through `audio.play(entity)`, which returns a handle for `audio.stop(handle)`.

The component only holds settings. It does not keep the handle of a sound it started. The editor's volume slider goes from `0` to `1`. See the [audio module](../audio/README.md) for how playback works.

## `script_component_t`

```cpp
constexpr usize SCRIPT_PATH_MAX_LENGTH = 256;

struct script_component_t
{
    char script_path[SCRIPT_PATH_MAX_LENGTH];
};
```

### State

| Field         | Default | Meaning                                  |
| ------------- | ------- | ---------------------------------------- |
| `script_path` | empty   | Lua file to run, up to 255 characters    |

### Who reads it

When the scene player starts, the script system walks the `script` system, which requires only this component. For each entity with a non-empty path, it loads the file into an environment of its own and calls `on_start(entity)` if the script defines it. Each update it calls `on_update(entity, delta_time)`, and on collisions `on_collision(entity, other)`. Entities with an empty path, or whose script fails to load, are skipped.

The path is opened as given, so a relative path is relative to the process working directory. In the editor, the Edit Script button opens the file in the script editor and is disabled while the path is empty.

## Usage

`scene_t::initialize()` registers each struct with its size and a name, and keeps the returned `component_type_t*` in a matching member (`name_type`, `transform_2D_type`, and so on):

```cpp
transform_2D_type = registry.register_component_type(sizeof(components::transform_2D_component_t), transform_2D_type_name, nullptr);
```

Adding a component builds the struct on the stack with its defaults and copies it into the registry:

```cpp
void scene_t::add_velocity_2D(const ecs::entity_t entity)
{
    components::velocity_2D_component_t velocity = {};
    velocity.speed = default_speed;

    add_component(*velocity_2D_type, &velocity, entity);
}
```

Reading one goes through a typed getter that casts the pointer from the ECS, and returns `nullptr` if the entity doesn't have the component:

```cpp
components::transform_2D_component_t* transform = scene->get_transform_2D(entity);
```

The editor's object inspector calls these getters for the selected entity and binds ImGui widgets directly to the fields, so edits change the stored component in place. The Lua functions do the same: they look up the component for the entity id they receive and raise a Lua error such as `entity 3 has no velocity_2D` if it is missing.

## Notes

- The headers set no defaults. A struct created outside `scene_t` starts with whatever its initializer gives it, and a zeroed sprite has size `(0, 0)` and a fully transparent tint.
- New cameras are primary by default. With several primary cameras, the one that was added first is used.
- Labels need a font path before they are drawn. Sprites without a texture are still drawn, as solid rectangles.
- Labels and colliders ignore the transform's scale and rotation. Only sprites use all three transform fields.
- Physics and collisions run only while the scene is playing. Sprites and labels are drawn every frame, playing or not.
- Pointers returned by the scene's getters point into the ECS buffer, so they stop being valid after a component of the same type is added or removed.
