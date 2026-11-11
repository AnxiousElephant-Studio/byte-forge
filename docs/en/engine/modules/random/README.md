# Random module

> Language: **English** | [Português (Brasil)](../../../../pt-br/engine/modules/random/README.md)

The random module produces pseudo-random numbers. It is one header-only struct, `random_t`, that wraps a standard library Mersenne Twister (`std::mt19937`) and offers ranges for integers, floats and 2D vectors. It lives in the `byteforge::random` namespace and has no source file.

## Files

| File                                                         | Contents                                 |
| ------------------------------------------------------------ | ---------------------------------------- |
| `engine/include/byte-forge-engine/modules/random/random.h`   | `random_t` declaration and implementation |

## Concepts

### Generator and distributions

Each `random_t` owns its own generator. The constructor seeds it once with a value from `std::random_device`, so two instances, or two runs of the program, produce different sequences. There is no way to pass a seed, so a sequence cannot be reproduced.

Every call creates a fresh standard distribution (`std::uniform_int_distribution` or `std::uniform_real_distribution`) with the requested bounds, draws one value from the generator and throws the distribution away.

### Bounds

| Function family | Range                                         |
| --------------- | --------------------------------------------- |
| Integer         | Closed: both `min` and `max` can be returned  |
| Float           | Half-open: `min` can be returned, `max` cannot |

These are the rules of the standard distributions the struct uses.

## `random_t`

```cpp
struct random_t
{
public:
    random_t() : generator(std::random_device{}()) {}

    i32 range_int(i32 min, i32 max);
    f32 range_float(f32 min, f32 max);
    math::vec2_t range_vec2(const math::vec2_t& min, const math::vec2_t& max);

    f32 next_float();
    i32 next_int();
    math::vec2_t next_vec2();

private:
    std::mt19937 generator;
};
```

All members are defined inline in the header. The struct is not marked `BF_RANDOM_API`, so every application that includes it compiles its own copy.

### State

| Field       | Meaning                                                        |
| ----------- | -------------------------------------------------------------- |
| `generator` | 32-bit Mersenne Twister. Advanced by every call                |

### `range_int(min, max)`

Returns a uniformly distributed integer in `[min, max]`. `min` must not be greater than `max`.

### `range_float(min, max)`

Returns a uniformly distributed float in `[min, max)`. `min` must not be greater than `max`.

### `range_vec2(min, max)`

Returns a `math::vec2_t` whose `x` is `range_float(min.x, max.x)` and whose `y` is `range_float(min.y, max.y)`. The two components are drawn independently, so the result lies inside the rectangle spanned by `min` and `max`.

### `next_float()`, `next_int()`, `next_vec2()`

Shortcuts for the unit ranges:

| Function      | Same as                                          | Result                         |
| ------------- | ------------------------------------------------ | ------------------------------ |
| `next_float()`| `range_float(0.0f, 1.0f)`                        | Float in `[0, 1)`              |
| `next_int()`  | `range_int(0, 1)`                                | `0` or `1`                     |
| `next_vec2()` | `range_vec2(vec2_t::zero(), vec2_t::one())`      | Both components in `[0, 1)`    |

## Usage

No code in the engine or the editor uses `random_t` yet. A caller creates one instance and keeps it, so the generator is seeded only once:

```cpp
#include <byte-forge-engine/modules/random/random.h>

byteforge::random::random_t random;

const i32 die = random.range_int(1, 6);
const math::vec2_t spawn = random.range_vec2(math::vec2_t(0.0f, 0.0f), math::vec2_t(800.0f, 600.0f));
```

## Notes

- `next_int()` returns only `0` or `1`, like a coin flip, not an arbitrary integer.
- Passing `min` greater than `max` breaks the precondition of the standard distributions, and the result is undefined.
- `random_t` holds a `std::mt19937`, which is about 5 KB. Copying a `random_t` copies the generator's state, so the copy produces the same sequence as the original from that point on.
- The struct has no locking. Using one instance from several threads at once is a data race.
- Lua scripts do not reach this module. Scripts can use Lua's own `math.random`.
