# Math module

> Language: **English** | [Português (Brasil)](../../../../pt-br/engine/modules/math/README.md)

The math module holds the small value types the engine uses for positions, sizes, colors, rectangles and transformations. Everything is header-only, lives in the `byteforge::math` namespace and is built on the aliases from `byte-forge-engine/api/types.h` (`f32`, `i32`, `usize`).

## Files

| Header                       | Type                 | Purpose                                      |
| ---------------------------- | -------------------- | -------------------------------------------- |
| `vectors/vector2.h`          | `vec2_t`             | 2D float vector (positions, sizes, UVs)      |
| `vectors/vector2i.h`         | `vec2i_t`            | 2D integer vector (pixel positions, sizes)   |
| `vectors/vector3.h`          | `vec3_t`             | 3D float vector                              |
| `vectors/vector4.h`          | `vec4_t`             | 4D float vector (tints, UV rectangles)       |
| `colors/color.h`             | `color_t`            | RGBA float color                             |
| `rects/recti.h`              | `recti_t`            | Integer rectangle (viewports, window bounds) |
| `matrices/matrix4.h`         | `mat4_t`             | 4x4 float matrix (projection and model)      |
| `utils/math_constants.h`     | `math_constants_t`   | `PI`, `HALF_PI`, `TAU`                       |
| `utils/math_converter.h`     | `math_converter_t`   | Degree and radian conversion                 |

All paths are relative to `engine/include/byte-forge-engine/modules/math/`. Include them from the include root:

```cpp
#include <byte-forge-engine/modules/math/vectors/vector2.h>
```

## Concepts

- Types are plain structs with public fields, so they can be copied, compared and passed to the GPU as they are.
- Almost everything is `constexpr`. The exceptions are functions that call `<cmath>`: `length()`, `normalized()`, `distance()` and `mat4_t::rotate_z()`.
- Operations never modify the object they are called on, except the compound operators (`+=`, `-=`, `*=`, `/=`). `normalized()`, `translate()`, `scale()` and the rest return a new value.
- Equality uses exact float comparison (`==`), with no epsilon.
- Angles in the public API are in **degrees**. Use `math_converter_t` when radians are needed.
- Matrices are stored in **column-major** order and transform **column vectors**, matching what OpenGL expects.
- Screen space has **Y pointing down**: `recti_t::top()` is the smaller Y, and the sprite renderer builds its projection with `top = 0` and `bottom = height`.

## Vectors

`vec2_t`, `vec3_t` and `vec4_t` share the same shape, with one, two or three more components.

| Member                                  | vec2 | vec3 | vec4 | Notes                                          |
| --------------------------------------- | :--: | :--: | :--: | ---------------------------------------------- |
| Default constructor                     |  ✓   |  ✓   |  ✓   | All components are `0`                         |
| `operator[](usize)`                     |  ✓   |  ✓   |  ✓   | Read or write a component by index             |
| `+`, `-` (vector)                       |  ✓   |  ✓   |  ✓   | Component-wise                                 |
| `*`, `/` (scalar)                       |  ✓   |  ✓   |  ✓   | `scalar * vector` also works                   |
| `+=`, `-=`, `*=`, `/=`                  |  ✓   |  ✓   |  ✓   |                                                |
| `==`                                    |  ✓   |  ✓   |  ✓   | Exact comparison                               |
| `length()`, `length_squared()`          |  ✓   |  ✓   |  ✓   |                                                |
| `dot()`                                 |  ✓   |  ✓   |  ✓   |                                                |
| `cross()`                               |      |  ✓   |      |                                                |
| `normalized()`                          |  ✓   |  ✓   |  ✓   | Returns `zero()` for a zero-length vector      |
| `distance()`, `distance_squared()`      |  ✓   |  ✓   |  ✓   |                                                |
| `zero()`, `one()`                       |  ✓   |  ✓   |  ✓   |                                                |
| `up()`, `down()`, `left()`, `right()`   |  ✓   |  ✓   |      | `up()` is `+Y`                                 |

Accessor aliases make intent clearer at the call site:

- `vec2_t`: `width()` / `height()`, `r()` / `g()`, `u()` / `v()`
- `vec3_t`: `r()` / `g()` / `b()`
- `vec4_t`: `r()` / `g()` / `b()` / `a()`

### Formulas

```
length(a)          = sqrt(a · a)
length_squared(a)  = a · a
a · b              = ax*bx + ay*by (+ az*bz) (+ aw*bw)
a × b              = (ay*bz - az*by, az*bx - ax*bz, ax*by - ay*bx)
normalized(a)      = a / length(a)
distance(a, b)     = length(a - b)
```

The sign of the dot product tells the angle between two vectors: positive means less than 90°, zero means perpendicular, negative means more than 90°. The cross product is perpendicular to both inputs, and its length is `|a| |b| sin(theta)`.

Prefer `length_squared()` and `distance_squared()` when only comparing lengths, since they skip the square root.

### `vec2i_t`

The integer vector is smaller on purpose. It has `x`, `y`, `width()`, `height()`, `operator[]`, `+`, `-`, `* i32`, `+=`, `-=`, `==`, `!=`, `zero()`, `one()` and `is_positive()` (true when both components are greater than zero). It has no division, length or normalization.

## `color_t`

An RGBA color with `f32` channels, usually in the `[0, 1]` range. The default constructor gives opaque black, and the alpha argument defaults to `1`.

- Operators: `+`, `* f32`, `f32 * color`, `+=`, `*=`, `==`.
- Presets: `black()`, `white()`, `red()`, `green()`, `blue()`, `yellow()`, `cyan()`, `magenta()`, `transparent()`.
- Channels are not clamped. Adding two colors or scaling by more than `1` can go above `1`.
- Scaling also scales alpha. `color * 0.5f` halves the opacity too.

## `recti_t`

An integer rectangle made of a `position` (top-left corner) and a `size`, both `vec2i_t`.

```cpp
const math::recti_t bounds(0, 0, 1600, 900);
```

| Member        | Result                           |
| ------------- | -------------------------------- |
| `left()`      | `position.x`                     |
| `top()`       | `position.y`                     |
| `right()`     | `position.x + size.width()`      |
| `bottom()`    | `position.y + size.height()`     |
| `is_empty()`  | `true` unless both sizes are > 0 |
| `contains(p)` | Half-open test, see below        |
| `==`          | Same position and size           |
| `from_size(s)`| Static: rect at `(0, 0)` covering `s`, like a viewport that fills a window or framebuffer |

`contains()` includes the left and top edges and excludes the right and bottom ones (`left <= x < right`, `top <= y < bottom`). A 1600x900 rectangle at the origin contains `(0, 0)` and `(1599, 899)`, but not `(1600, 900)`.

## `mat4_t`

A 4x4 matrix stored as `f32 values[16]` in column-major order, where `index = column * 4 + row`. The default constructor gives an all-zero matrix, so start from `identity()`.

| Member                                  | Description                                          |
| --------------------------------------- | ---------------------------------------------------- |
| `identity()`                            | Diagonal of ones at indices 0, 5, 10 and 15          |
| `orthographic(l, r, b, t, near, far)`   | Maps the box to the `[-1, 1]` clip range             |
| `translate(x, y, z)` / `translate(vec3)`| Returns `this * T`                                   |
| `scale(x, y, z)`                        | Returns `this * S`                                   |
| `rotate_z(degrees)`                     | Returns `this * Rz`, rotating in the XY plane        |
| `multiply(right)`                       | Returns `this * right`                               |

### Composition order

Each transform call multiplies on the **right** of the current matrix. With column vectors the rightmost matrix is applied first, so the **last call is applied to the vertex first**. The sprite renderer builds its model matrix like this:

```cpp
const math::mat4_t model = math::mat4_t::identity()
    .translate(position.x, position.y, 0.0f)
    .rotate_z(rotation_degrees)
    .scale(size.x, size.y, 1.0f);
```

That is `T * R * S`: the quad is scaled first, then rotated around its own center, then moved to `position`.

### Orthographic projection

```
|  2/(r-l)        0               0               0 |
|  0              2/(t-b)         0               0 |
|  0              0              -2/(f-n)         0 |
| -(r+l)/(r-l)   -(t+b)/(t-b)    -(f+n)/(f-n)     1 |
```

The matrix above is shown in memory order (each printed row is one column). X and Y are scaled so the range has a size of `2`, then shifted so its center lands on the origin. The negative Z scale flips the Z axis. Passing `top = 0` and `bottom = height` gives Y-down screen coordinates.

### Rotation around Z

```
x' = x cos(theta) - y sin(theta)
y' = x sin(theta) + y cos(theta)
```

Positive angles rotate from +X toward +Y. With Y pointing down on screen, that looks clockwise.

## Constants and conversion

```cpp
math_constants_t::PI       // 3.14159265...
math_constants_t::HALF_PI  // PI / 2
math_constants_t::TAU      // PI * 2

math_converter_t::deg_to_rad(180.0f)                 // PI
math_converter_t::rad_to_deg(math_constants_t::PI)   // 180
```

## Usage

Every type is header-only, so using one is just including its header. Nothing has to be initialized or disposed.

| Type      | Main users                                                                  |
| --------- | --------------------------------------------------------------------------- |
| `vec2_t`  | 2D components (transform, velocity, collider, sprite), physics, input, Lua bindings |
| `vec2i_t` | Window sizes, viewports, `recti_t`                                          |
| `vec4_t`  | Sprite tints and UV rectangles, shader uniforms                             |
| `vec3_t`  | `mat4_t::translate(vec3_t)`                                                 |
| `recti_t` | Renderer viewport, the editor's window bounds                               |
| `mat4_t`  | Sprite renderer projection and model matrices, shader uniforms              |

## Notes

- `color_t` is not used anywhere in the engine yet. Tints are passed as `vec4_t`.
- `normalized()` of a zero-length vector returns `zero()` instead of dividing by zero.
- `recti_t::contains()` treats the right and bottom edges as outside the rectangle.
- `mat4_t()` is all zeros, not the identity.
- Float equality is exact, so two vectors computed in different ways may not compare equal.
