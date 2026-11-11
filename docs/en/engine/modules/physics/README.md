# Physics module

> Language: **English** | [Português (Brasil)](../../../../pt-br/engine/modules/physics/README.md)

The physics module holds the 2D math that the scene uses to move entities and to detect overlaps. It is one struct, `physics_2D_t`, with two static functions and no state: one moves a position by a velocity, the other tests two axis-aligned boxes. It does not wrap any physics library. It lives in the `byteforge::physics` namespace and is not header-only.

## Files

| File                                                              | Contents                                         |
| ----------------------------------------------------------------- | ------------------------------------------------ |
| `engine/include/byte-forge-engine/modules/physics/physics_2D.h`   | `physics_2D_t` declaration                       |
| `engine/source/modules/physics/physics_2D.cpp`                    | Implementation of `move` and `check_aabb_collision` |

Related pieces in other modules:

| File                                                                      | Role                                              |
| ------------------------------------------------------------------------- | ------------------------------------------------- |
| `modules/components/velocity_2D_component.h`                              | Velocity vector and speed of an entity            |
| `modules/components/box_collider_2D_component.h`                          | Offset and size of an entity's collision box      |
| `modules/scene/scene_player.cpp`                                          | Calls both functions every frame while playing    |
| `modules/scene/scene_script_system.cpp`                                   | Receives the collisions as `on_collision` calls   |

## Concepts

### No state, no world

There is no physics world, body or step function. `physics_2D_t` only computes values from the arguments it receives. The scene keeps the data in its components, and `scene_player_t` decides when to call the functions and what to do with the results.

### Positions are centers

Both functions treat a position as the center of the object. A box with position `(10, 10)` and size `(4, 2)` covers x from `8` to `12` and y from `9` to `11`. This matches the sprite renderer, which draws a quad centered on the transform position.

### Units

The functions have no unit of their own. In the scene, positions are in world units, which are pixels when no camera is active, and delta time is in seconds. Velocity times speed is then the displacement per second.

## `physics_2D_t`

```cpp
struct BF_PHYSICS_API physics_2D_t
{
public:
    static math::vec2_t move(const math::vec2_t& position, const math::vec2_t& velocity, const f32 speed, const f32 delta_time);
    static bool check_aabb_collision(const math::vec2_t& a_position, const math::vec2_t& a_size, const math::vec2_t& b_position, const math::vec2_t& b_size);
};
```

The struct has no fields and is never instantiated. Both members are called as `physics::physics_2D_t::move(...)` and `physics::physics_2D_t::check_aabb_collision(...)`.

### `move(position, velocity, speed, delta_time)`

Returns `position + velocity * speed * delta_time`. It is one explicit Euler step: the result depends only on the current position and velocity, and nothing is accumulated between calls.

`velocity` is a direction and `speed` scales it. The function does not normalize `velocity`, so its length also affects the distance. A velocity of `(1, 1)` with speed `100` moves about `141` units per second along the diagonal, while `(1, 0)` moves `100`.

### `check_aabb_collision(a_position, a_size, b_position, b_size)`

Tests whether two axis-aligned boxes overlap.

1. Computes the half sizes by multiplying each size by `0.5`.
2. On the x axis, the boxes overlap when the left edge of `a` is less than or equal to the right edge of `b` and the right edge of `a` is greater than or equal to the left edge of `b`.
3. The y axis is checked the same way with the top and bottom edges.
4. Returns `true` only when both axes overlap.

The comparisons use `<=` and `>=`, so two boxes that only share an edge or a corner count as colliding. The function only answers yes or no. It does not return the overlap depth, the contact normal or which side was hit.

## Usage

Only `scene_player_t` calls the module, from `update()` while the scene is playing and not paused (or during a single step):

- The physics pass goes through every entity with a transform and a velocity and stores `move(transform.position, velocity.velocity, velocity.speed, delta_time)` back into the transform. `delta_time` is already multiplied by the player's `time_scale`.
- The collision pass goes through every pair of entities with a transform and a box collider. For each pair it calls `check_aabb_collision()` with `transform.position + collider.offset` and `collider.size` for both sides. On overlap it notifies both scripts through `on_collision`.

From `scene_player.cpp`:

```cpp
transform->position = physics::physics_2D_t::move(transform->position, velocity->velocity, velocity->speed, delta_time);
```

Scripts never call the module directly. They change its inputs with `velocity.set`, `velocity.set_speed` and `transform.set_position`, and react to its output in `on_collision`. See the [scene module](../scene/README.md) for the full frame order.

## Notes

- Collisions are detected but not resolved. Overlapping entities keep moving through each other unless a script changes their position or velocity.
- Movement and collision are separate passes. An entity that moves far enough in one frame can pass through a thin collider without any frame where the boxes overlap.
- The collision box ignores the transform's scale and rotation. Only the collider's own offset and size are used.
- The collision pass tests every pair, so its cost grows with the square of the number of colliders.
- A box with a size of `(0, 0)` is a point. It still collides when it lies inside or on the edge of another box.
