# ECS module

> Language: **English** | [Português (Brasil)](../../../../pt-br/engine/modules/ecs/README.md)

The ECS module is the engine's entity component system. Entities are plain numbers, components are raw bytes stored in one packed buffer per component type, and systems are lists of the entities that have a given set of components. Everything is owned by one `registry_t`. The module lives in the `byteforge::ecs` namespace and has no third-party dependencies. Inside the engine it only depends on the logging module: it does not include rendering, platform or scene, so it can be used (and later built) on its own.

`entity.h` is header-only. The other three types are declared in headers and implemented in `engine/source/modules/ecs/`, with `registry_t` split across several source files by topic.

## Files

| File                                                            | Contents                                                        |
| --------------------------------------------------------------- | --------------------------------------------------------------- |
| `engine/include/byte-forge-engine/modules/ecs/entity.h`         | `entity_t`, a wrapper around a `u64` id                         |
| `engine/include/byte-forge-engine/modules/ecs/component_type.h` | `component_type_t` and `component_free_function_t`              |
| `engine/include/byte-forge-engine/modules/ecs/system.h`         | `system_t`, `system_update_function_t`, `system_free_function_t` |
| `engine/include/byte-forge-engine/modules/ecs/registry.h`       | `registry_t` and `MAX_COMPONENT_TYPES`                          |
| `engine/source/modules/ecs/component_type.cpp`                  | Component storage: add, replace, remove, debug print            |
| `engine/source/modules/ecs/system.cpp`                          | System matching and entity list                                 |
| `engine/source/modules/ecs/registry.cpp`                        | `update()`, `print_info()`, `dispose()`                         |
| `engine/source/modules/ecs/registry_entity.cpp`                 | Creating, spawning and killing entities                         |
| `engine/source/modules/ecs/registry_component.cpp`              | Registering component types, adding and removing components     |
| `engine/source/modules/ecs/registry_system.cpp`                 | `create_system()`                                               |
| `engine/source/modules/ecs/registry_validation.cpp`             | `validate()` and `get_memory_bytes()`                           |

Related pieces in other modules:

| File                                                      | Role                                                           |
| --------------------------------------------------------- | -------------------------------------------------------------- |
| `modules/components/*.h`                                  | The component structs the scene stores in the registry         |
| `modules/scene/scene.cpp`                                 | Owns the registry, registers the types and creates the systems |
| `modules/scene/scene_player.cpp`                          | Walks the physics, collision and audio system lists            |
| `modules/scene/scene_script_system.cpp`                   | Walks the script system list, passes entity ids to Lua         |
| `applications/editor/.../ecs_observer_component.cpp`      | Editor window that shows the registry and runs `validate()`    |

## Concepts

### Entities are ids

An entity is only a `u64`. The registry hands them out in order, starting at `0`, from `entities_current_id`. An id is never reused: killing an entity does not give its id back.

The id is also an index. `entities_components_bitsets[id]` holds the entity's component bitset, so that vector has one slot per id ever created, including ids of entities that were killed.

### Component bitsets

Each registered component type gets an id equal to its position in `component_types` (`0`, `1`, `2`, ...). That id is a bit position. An entity's bitset has bit `n` set when it has a component of type `n`.

Because the bitset is a `u64`, there can be at most 64 component types (`MAX_COMPONENT_TYPES`).

### Packed component storage

A `component_type_t` stores every component of its type back to back in one `std::vector<u8>`. Element `i` occupies bytes `[i * size, (i + 1) * size)`. A map, `entity_id_to_index`, says which element belongs to which entity.

The ECS does not know the C++ type of a component. It copies bytes in with `std::memcpy` and hands back `void*`. The caller casts the pointer to the right struct. This only works for types that are safe to copy byte by byte. For a component that owns memory, the type can be registered with a free function, which the ECS calls before the bytes are overwritten or dropped.

### Systems are entity lists

A `system_t` has a required bitset and a list of entities. An entity belongs in the system when its bitset contains every bit of the required bitset. The registry keeps these lists up to date only when asked: adding or removing a component changes the bitset, but the system lists change only on `spawn_entity()`, `refresh_entity_systems()` or `kill_entity()`.

A system can also carry an update function and a data pointer. `registry_t::update()` calls the update functions. In the current engine no system uses one: code that needs a system reads its `entities` list directly.

### Immediate and deferred operations

Entity creation and destruction exist in two forms:

| Immediate              | Deferred                                                          |
| ---------------------- | ----------------------------------------------------------------- |
| `spawn_entity(entity)` | `publish_entity(entity)`, then `spawn_entities()`                 |
| `kill_entity(entity)`  | `add_entity_to_kill_queue(entity)`, then `kill_entities()`        |

The deferred form puts the entity in a queue (`entities_to_add` or `entities_to_kill`) and applies the whole queue later. This makes it possible to request changes while iterating over a system list without changing the list under the loop. The scene and the editor currently use only the immediate form.

## `entity_t`

```cpp
struct entity_t
{
public:
    u64 id;

    constexpr entity_t()       : id(0) {}
    constexpr entity_t(u64 id) : id(id) {}

    constexpr bool operator==(const entity_t& other) const { return id == other.id; }
};
```

A value type that wraps the id. Two entities are equal when their ids are equal. There is no `operator!=` and no hash, so maps in the ECS are keyed by `entity.id` instead.

The default constructor gives id `0`, which is also the id of the first entity the registry creates. A default-constructed `entity_t` is therefore not an "empty" value. The editor keeps a separate `has_selection` flag next to its `selected_entity` for this reason.

## `component_type_t`

```cpp
using component_free_function_t = void (*)(void* component_data);

struct BF_ECS_API component_type_t
{
public:
    u64                            id;
    usize                          size;
    std::string                    name;
    std::vector<u8>                data;
    std::unordered_map<u64, usize> entity_id_to_index;
    component_free_function_t      free_function;

    component_type_t(u64 id, usize size, const char* name, component_free_function_t free_function);

    usize get_count() const;
    usize get_memory_bytes() const;
    void* get(const entity_t entity);
    bool  has(const entity_t entity) const;
    void  add_or_replace(const entity_t entity, const void* const component);
    void  remove(const entity_t entity);
    void  print_info(const usize registry_index) const;
    void  dispose();
};
```

Instances are created with `new` by `registry_t::register_component_type()` and deleted by `registry_t::dispose()`. Code outside the registry gets a pointer and does not own it.

### State

| Field                | Meaning                                                                  |
| -------------------- | ------------------------------------------------------------------------ |
| `id`                 | Index in the registry's `component_types`, and the bit used in bitsets   |
| `size`               | Size of one component in bytes, usually `sizeof` of the struct           |
| `name`               | Name given at registration. Used for lookup and in debug output          |
| `data`               | All components of this type, packed back to back                         |
| `entity_id_to_index` | Entity id to element index in `data`                                     |
| `free_function`      | Called on a component's bytes before they are overwritten or dropped. May be `nullptr` |

### `get_count()` and `get_memory_bytes()`

| Member               | Returns                                                                     |
| -------------------- | --------------------------------------------------------------------------- |
| `get_count()`        | `data.size() / size`, or `0` when `size` is `0`                             |
| `get_memory_bytes()` | `data.capacity()` plus `(sizeof(u64) + sizeof(usize))` per map entry. An estimate: it does not include the map's own node and bucket overhead |

### `get(entity)`

Looks the entity id up in `entity_id_to_index`. Returns `nullptr` if the entity has no component of this type, otherwise a pointer to its element in `data`.

The pointer points into the vector. Any later `add_or_replace()` that adds a new element can reallocate `data`, and any `remove()` shifts the elements after the removed one, so a pointer from `get()` should not be kept across those calls.

### `has(entity)`

`true` if the entity id is in `entity_id_to_index`.

### `add_or_replace(entity, component)`

1. If the entity already has a component of this type, calls `free_function` on the existing bytes (when set), copies `size` bytes from `component` over them and returns. The element keeps its index.
2. Otherwise, grows `data` by `size` bytes, copies the component into the new last element and records its index in the map.

It does not touch any bitset. The registry's `add_component_to_entity()` sets the bit and then calls this.

### `remove(entity)`

1. Returns if the entity has no component of this type.
2. Calls `free_function` on the element's bytes, when set.
3. Erases the element's bytes from `data`. Every element after it moves down by one slot.
4. Erases the entity from the map, then walks the whole map and decrements every index greater than the removed one.

Removal keeps the remaining elements in their original order. Its cost grows with both the number of bytes after the removed element and the number of entries in the map.

### `print_info(registry_index)`

Logs, at debug level, a block with the index passed in, the type's id, size, name and element count, followed by one `Entity ID: <id> -> Array Index: <index>` line per map entry.

### `dispose()`

Calls `free_function` on every element (when set), then clears `data` and the map. The `component_type_t` object itself stays alive and can be reused.

## `system_t`

```cpp
class registry_t;
struct system_t;

using system_update_function_t = void (*)(registry_t& registry, system_t& system, const f64 delta_time);
using system_free_function_t   = void (*)(void* system_data);

struct BF_ECS_API system_t
{
public:
    u64                      id;
    std::string              name;
    void*                    data;
    u64                      required_entity_components_bitset;
    system_update_function_t update_function;
    system_free_function_t   free_function;
    std::vector<entity_t>    entities;

    system_t(u64 id, const char* name, void* data, u64 required_entity_components_bitset,
             system_update_function_t update_function, system_free_function_t free_function);

    bool accepts(const u64 entity_components_bitset) const;
    bool contains_entity(const entity_t entity) const;
    void add_entity(const entity_t entity);
    void remove_entity(const entity_t entity);
    void print_info(const usize registry_index) const;
    void dispose();
};
```

Like component types, systems are created with `new` by `registry_t::create_system()` and deleted by `registry_t::dispose()`.

### State

| Field                               | Meaning                                                        |
| ----------------------------------- | -------------------------------------------------------------- |
| `id`                                | Index in the registry's `systems`                              |
| `name`                              | Name given at creation, used in debug output and validation    |
| `data`                              | Optional user data, passed to `free_function` on dispose       |
| `required_entity_components_bitset` | Bits an entity must have to belong to this system              |
| `update_function`                   | Called by `registry_t::update()`. May be `nullptr`             |
| `free_function`                     | Frees `data` on dispose. May be `nullptr`                      |
| `entities`                          | The entities currently in the system, in the order they were added |

### `accepts(bitset)`

`true` when `bitset` contains every bit of `required_entity_components_bitset`, that is `(required & bitset) == required`. Extra bits don't matter. A system with a required bitset of `0` accepts every entity.

### `contains_entity(entity)`

A linear search through `entities`.

### `add_entity(entity)` and `remove_entity(entity)`

`add_entity()` appends the entity without checking whether it is already there. `remove_entity()` searches for the first occurrence and erases it, keeping the order of the rest. If the entity is not in the list, it does nothing.

### `print_info(registry_index)`

Logs, at debug level, the index passed in, the id, the name, the required bitset as 16 hex digits and the entity count.

### `dispose()`

If both `free_function` and `data` are set, calls `free_function(data)`. Then sets `data` to `nullptr` and clears `entities`.

## `registry_t`

```cpp
constexpr usize MAX_COMPONENT_TYPES = 64;

class BF_ECS_API registry_t
{
public:
    std::vector<entity_t>          entities;
    std::vector<component_type_t*> component_types;
    std::vector<system_t*>         systems;
    std::vector<u64>               entities_components_bitsets;
    std::vector<entity_t>          entities_to_add;
    std::vector<entity_t>          entities_to_kill;
    u64                            entities_current_id;

    registry_t() : entities_current_id(0) {}

    void  update(const f64 delta_time);
    void  print_info() const;
    usize validate(std::vector<std::string>& out_issues) const;
    usize get_memory_bytes() const;
    void  dispose();

    entity_t create_entity();
    void     publish_entity(const entity_t entity);
    void     spawn_entities();
    void     spawn_entity(const entity_t entity);
    void     add_entity_to_kill_queue(const entity_t entity);
    void     kill_entities();
    void     kill_entity(const entity_t entity);
    void     refresh_entity_systems(const entity_t entity);

    component_type_t* register_component_type(const usize size, const char* name, component_free_function_t free_function);
    component_type_t* get_component_type_by_name(const char* name) const;
    void              add_component_to_entity(component_type_t& component_type, const void* const component, const entity_t entity);
    void              remove_component_from_entity(component_type_t& component_type, const entity_t entity);

    system_t* create_system(const u64 required_entity_components_bitset, const char* name,
                            system_update_function_t update_function, system_free_function_t free_function, void* data);
};
```

The copy constructor and copy assignment are deleted. The registry holds raw owning pointers to its component types and systems, and a copy would delete them twice.

There is no destructor. Component types and systems are freed only in `dispose()`, which the owner has to call.

All fields are public. The editor reads them directly to draw its panels.

### State

| Field                         | Meaning                                                                 |
| ----------------------------- | ----------------------------------------------------------------------- |
| `entities`                    | Entities that are alive, in creation order                              |
| `component_types`             | Registered component types, indexed by their id                         |
| `systems`                     | Created systems, indexed by their id                                    |
| `entities_components_bitsets` | One bitset per entity id ever created, indexed by id                    |
| `entities_to_add`             | Queue filled by `publish_entity()`, applied by `spawn_entities()`       |
| `entities_to_kill`            | Queue filled by `add_entity_to_kill_queue()`, applied by `kill_entities()` |
| `entities_current_id`         | The id the next `create_entity()` will return. Starts at `0`            |

### `create_entity()`

Takes the next id, appends the entity to `entities` and appends a zero bitset to `entities_components_bitsets`. Returns the entity.

The entity is alive from this point on, but it is in no system until `spawn_entity()` (or `publish_entity()` and `spawn_entities()`) is called.

### `register_component_type(size, name, free_function)`

If `MAX_COMPONENT_TYPES` types are already registered, logs `Could not register component type '<name>': the limit of 64 component types was reached.` and returns `nullptr`.

Otherwise creates a `component_type_t` whose id is the current number of types, appends it and returns it. It does not check whether the name is already taken or whether `size` is `0`.

### `get_component_type_by_name(name)`

A linear search comparing `name` with each type's name. Returns the first match or `nullptr`. Nothing in the engine or editor calls it today.

### `add_component_to_entity(component_type, component, entity)`

1. If `entity.id` is not below the size of `entities_components_bitsets`, logs `Could not add component '<type>' to entity <id>: the entity does not exist.` and returns.
2. Sets the type's bit in the entity's bitset.
3. Calls `component_type.add_or_replace()`, which copies the bytes.

System lists are not updated. Call `refresh_entity_systems()` afterwards, or `spawn_entity()` if the entity has not been spawned yet.

### `remove_component_from_entity(component_type, entity)`

If the id is in range, clears the type's bit and calls `component_type.remove()`. An out-of-range id does nothing and logs nothing. Like adding, it does not update system lists.

### `spawn_entity(entity)`

Reads the entity's bitset and calls `add_entity()` on every system that accepts it. Out-of-range ids are ignored.

It does not check whether the entity is already in a system, so it is meant to be called once per entity. Later changes go through `refresh_entity_systems()`.

### `refresh_entity_systems(entity)`

For every system, compares "accepts the current bitset" with "already contains the entity":

| Accepts | Contains | Action                 |
| ------- | -------- | ---------------------- |
| yes     | no       | `add_entity(entity)`    |
| no      | yes      | `remove_entity(entity)` |
| yes     | yes      | nothing                |
| no      | no       | nothing                |

Because it checks `contains_entity()` first, calling it more than once is safe.

### `publish_entity(entity)` and `spawn_entities()`

`publish_entity()` appends the entity to `entities_to_add`. `spawn_entities()` calls `spawn_entity()` on each queued entity in order, then clears the queue.

### `kill_entity(entity)`

1. Calls `remove_entity()` on every system.
2. Calls `remove()` on every component type, which runs the free functions.
3. If the id is in range, sets its bitset to `0`. The slot stays in the vector.
4. Finds the entity in `entities` and erases it, keeping the order of the rest.

It does not check whether the entity is alive. For an id that was never created or was already killed, the same steps run and normally find nothing to remove.

### `add_entity_to_kill_queue(entity)` and `kill_entities()`

`add_entity_to_kill_queue()` appends the entity to `entities_to_kill`. `kill_entities()` calls `kill_entity()` on each queued entity in order, then clears the queue.

### `create_system(required_bitset, name, update_function, free_function, data)`

Creates a `system_t` whose id is the current number of systems, appends it and returns it. There is no limit on the number of systems.

The new system starts empty. Entities that already exist are not added to it. Systems are meant to be created before entities are spawned.

### `update(delta_time)`

Calls each system's `update_function`, in creation order, with the registry, the system and `delta_time`. Systems without an update function are skipped.

### `print_info()`

Logs the number of alive entities and two titles at info level, then calls `print_info()` on every system and every component type, which log at debug level.

### `validate(out_issues)`

Checks that the registry's data structures agree with each other. It clears `out_issues`, appends one message per problem found and returns the number of messages. It does not change anything.

The checks, in order:

1. `entities_components_bitsets` has exactly `entities_current_id` entries.
2. For each component type:
   - `size` is not `0`. If it is, the other checks for that type are skipped.
   - `data` is a multiple of `size` bytes.
   - The map has as many entries as there are elements.
   - Every index in the map is in range, and no two entities share an index.
   - Every entity in the map is alive, and its bitset has the type's bit set.
3. For each alive entity, every set bit belongs to a registered type, and that type has a component for the entity.
4. For each system:
   - No entity appears twice.
   - Every entity in it is alive and is accepted by the system.
   - Every alive entity that the system accepts is in it. Entities waiting in `entities_to_add` are exempt from this check.

Each message is formatted into a 256-byte buffer, so a very long component or system name can cut it short.

### `get_memory_bytes()`

An estimate of the memory the ECS data uses: the capacities of `entities`, `entities_components_bitsets` and the two queues times their element size, plus `get_memory_bytes()` of every component type, plus the capacity of every system's entity list. It does not count the `component_types` and `systems` pointer vectors, the objects they point to, names or system `data`.

### `dispose()`

1. For each system, calls `dispose()` and deletes it.
2. For each component type, calls `dispose()` (which runs the free functions) and deletes it.
3. Clears every vector and resets `entities_current_id` to `0`.

After `dispose()` the registry is empty and can be used again. Pointers to component types and systems obtained earlier are dangling.

## Usage

`scene_t` owns the registry as a member (`ecs::registry_t registry`). In the editor, `scene.initialize()` runs at startup and `scene.dispose()` at shutdown, which calls `registry.dispose()`.

`scene_t::initialize()` registers nine component types, all with a `nullptr` free function, then creates seven systems with no update function. The ids follow registration order:

| Type id | Component type    | Struct                        |
| ------- | ----------------- | ----------------------------- |
| 0       | `name`            | `name_component_t`            |
| 1       | `transform_2D`    | `transform_2D_component_t`    |
| 2       | `sprite_2D`       | `sprite_2D_component_t`       |
| 3       | `script`          | `script_component_t`          |
| 4       | `velocity_2D`     | `velocity_2D_component_t`     |
| 5       | `box_collider_2D` | `box_collider_2D_component_t` |
| 6       | `audio_source`    | `audio_source_component_t`    |
| 7       | `label_2D`        | `label_2D_component_t`        |
| 8       | `camera_2D`       | `camera_2D_component_t`       |

| System id | System         | Required component types          |
| --------- | -------------- | --------------------------------- |
| 0         | `sprite_2D`    | `transform_2D`, `sprite_2D`       |
| 1         | `script`       | `script`                          |
| 2         | `physics_2D`   | `transform_2D`, `velocity_2D`     |
| 3         | `collision_2D` | `transform_2D`, `box_collider_2D` |
| 4         | `audio`        | `audio_source`                    |
| 5         | `label_2D`     | `transform_2D`, `label_2D`        |
| 6         | `camera_2D`    | `transform_2D`, `camera_2D`       |

The scene builds a required bitset by shifting `1` by each type's id:

```cpp
const u64 transform_2D_bit = get_component_bit(transform_2D_type);

sprite_2D_system = registry.create_system(transform_2D_bit | get_component_bit(sprite_2D_type), sprite_2D_system_name, nullptr, nullptr, nullptr);
```

Creating an entity adds the two components every entity has, then spawns it:

```cpp
const ecs::entity_t entity = registry.create_entity();

// ... fill name_component and transform ...

registry.add_component_to_entity(*name_type, &name_component, entity);
registry.add_component_to_entity(*transform_2D_type, &transform, entity);
registry.spawn_entity(entity);
```

Later additions and removals (from the editor's object inspector) go through `scene_t::add_component()` and `scene_t::remove_component()`, which call `add_component_to_entity()` or `remove_component_from_entity()` and then `refresh_entity_systems()`. Deleting an entity from the scene hierarchy calls `scene_t::destroy_entity()`, which calls `kill_entity()` right away.

Systems are read, not run. `registry.update()` is never called. Instead:

- `scene_t::render()` walks `camera_2D_system`, `sprite_2D_system` and `label_2D_system`.
- `scene_player_t` walks `physics_2D_system` and `collision_2D_system` every update and `audio_system` on start.
- `scene_script_system_t` walks `script_system` on start and every update.

Lua sees entities as integers. Script callbacks receive `entity.id`, and the Lua API functions turn the integer back into an `entity_t` with `luaL_checkinteger`.

The editor reads the registry in two panels. The ECS Observer window lists entities with their component names and bitset, component types with count and memory, systems with their required types and entity count, and has a Validation tab that calls `validate()` on demand or every frame. The status panel shows the entity and queue counts, the number of types out of `MAX_COMPONENT_TYPES`, the number of systems and `get_memory_bytes()` in kilobytes.

## Notes

- Ids are never reused and `entities_components_bitsets` never shrinks until `dispose()`. A killed entity keeps its slot with a bitset of `0`.
- `add_component_to_entity()` checks only that the id was created, not that the entity is alive. Adding a component to a killed entity stores it, and `validate()` then reports it as a component of an entity that is not alive.
- `kill_entity()` does not remove the entity from `entities_to_add`. If an entity is published and then killed before `spawn_entities()`, it is still added to systems afterwards.
- Adding or removing a component does not update system lists on its own. `refresh_entity_systems()` (or `spawn_entity()` for a new entity) has to follow.
- Calling `spawn_entity()` twice for the same entity adds it twice to every accepting system. `validate()` reports this as a duplicate.
- Components are copied with `std::memcpy` and returned as `void*`, so they must be safe to copy byte by byte, and the caller is responsible for casting to the right type.
- Pointers returned by `component_type_t::get()` become invalid after the same type adds a new component or removes one.
- Removal is linear: removing a component shifts the bytes after it and rewrites indices in the map, and removing an entity from a system or from `entities` is a search plus an erase.
