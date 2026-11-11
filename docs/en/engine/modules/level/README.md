# Level module

> Language: **English** | [Português (Brasil)](../../../../pt-br/engine/modules/level/README.md)

The level module saves scenes to JSON files and loads them back. It depends on the scene module; the scene module does not depend on it. Everything lives in the `byteforge::level` namespace.

## Files

| File                                                                | Contents                                           |
| ------------------------------------------------------------------- | -------------------------------------------------- |
| `engine/include/byte-forge-engine/modules/level/level_file.h`       | `level_file_t`: whole scene, `.bflevel`            |
| `engine/include/byte-forge-engine/modules/level/stamp_file.h`      | `stamp_file_t`: one entity and its children, `.bfstamp` |
| `engine/source/modules/level/entity_json.h`, `.cpp`                 | Internal: entities to JSON and back                |

## Entity format

Both files store an `entities` array. Each entry has the entity `name` and one object per component it has (`transform_2D`, `sprite_2D`, `tags`, `point_light_2D`...). An entity with a parent has `"parent": <index>`, the position of the parent inside the same array. Indices are used instead of entity ids because ids change every time a scene is loaded. Missing keys keep their defaults, so older files still load.

## `level_file_t`

| Member                                  | Description                                                                  |
| --------------------------------------- | ---------------------------------------------------------------------------- |
| `serialize(scene_context)`              | Text of the whole scene, with `format_version` and `default_2D_systems`      |
| `save(scene_context, path, error)`      | Writes `serialize()` to a file                                               |
| `load(scene_context, path, error)`      | Reads a file and calls `deserialize()`                                       |
| `deserialize(scene_context, text, error)` | Clears the scene, adds or removes the default 2D systems, creates the entities |
| `clear(scene_context)`                  | Destroys every entity                                                        |

The editor uses `serialize()` and `deserialize()` to snapshot the scene on Play and restore it on Stop.

## `stamp_file_t`

A stamp is a reusable piece of a scene: an entity and all of its children, saved on its own.

| Member                                       | Description                                                          |
| -------------------------------------------- | -------------------------------------------------------------------- |
| `save(scene, root, path, error)`             | Saves `root` and its subtree; the root is saved without a parent     |
| `instantiate(scene, path, out_root, error)`  | Creates a copy of the saved entities and returns the new root        |

An instance is a plain copy: editing the stamp file later does not change instances already in a level. `instantiate` matches `scene::stamp_loader_t`, so applications pass it to `script_system.stamp_loader` to let Lua call `scene.spawn_stamp(path)`.

In the editor, "Save as Stamp" in an entity's context menu writes `<assets>/stamps/<name>.bfstamp`. Double-clicking a stamp in the Content Browser, or dropping it on the Scene Hierarchy, instantiates it.
