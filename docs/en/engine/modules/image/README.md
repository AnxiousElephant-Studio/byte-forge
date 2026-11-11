# Image module

> Language: **English** | [Português (Brasil)](../../../../pt-br/engine/modules/image/README.md)

The image module decodes image files into pixels in memory. It is a thin layer over [stb_image](https://github.com/nothings/stb): one struct, `image_t`, that loads a file as 8-bit RGBA and frees the pixels again. It lives in the `byteforge::image` namespace and does not touch the GPU; uploading the pixels is done by the [rendering module](../rendering/README.md).

## Files

| File                                                | Contents                                        |
| --------------------------------------------------- | ----------------------------------------------- |
| `engine/include/byte-forge-engine/modules/image/image.h` | `image_t` declaration                      |
| `engine/source/modules/image/image.cpp`             | Implementation and the stb_image implementation |

Related pieces in other modules:

| File                                                          | Role                                       |
| ------------------------------------------------------------- | ------------------------------------------ |
| `modules/rendering/opengl/texture/opengl_texture.cpp`         | `opengl_texture_t::load()` is the only caller |

## Concepts

### How stb_image is hidden

The header includes nothing from stb. `image.cpp` defines `STB_IMAGE_IMPLEMENTATION` before including `<stb_image.h>`, which compiles the library into the engine DLL. No other file may define it.

### Pixel layout

Pixels are always 4 channels, red, green, blue and alpha, one byte each, whatever the file stores. A grayscale or RGB file is expanded and gets an opaque alpha. Rows are tightly packed, so the buffer is `size.x * size.y * 4` bytes and the first row is the **top** of the image. This matches the texture coordinates of the sprite renderer's quad.

## `image_t`

```cpp
struct BF_IMAGE_API image_t
{
public:
    u8* pixels;
    math::vec2i_t size;

    image_t();
    ~image_t();

    image_t(const image_t&)            = delete;
    image_t& operator=(const image_t&) = delete;

    bool load(const char* path);
    void dispose();
};
```

The copy constructor and copy assignment are deleted. A copy would point to the same pixel buffer, and both would free it. Since a destructor and deleted copies are declared, no move operations are generated either, so an `image_t` can't be moved.

The destructor calls `dispose()`, so an `image_t` on the stack frees its pixels when it goes out of scope.

### State

| Field    | Meaning                                                              |
| -------- | -------------------------------------------------------------------- |
| `pixels` | RGBA bytes allocated by stb_image, `nullptr` when nothing is loaded  |
| `size`   | Width (`x`) and height (`y`) in pixels, `(0, 0)` when nothing is loaded |

### `load(path)`

1. Calls `dispose()`, so a previous image is freed first.
2. Calls `stbi_load` asking for 4 channels. stb_image fills `size` and returns the pixels.
3. If it fails, logs `Could not load image '<path>': <reason>`, where `<reason>` is `stbi_failure_reason()`, sets `size` to zero and returns `false`.
4. Returns `true` on success.

Formats are those stb_image decodes in its default build: PNG, JPEG, BMP, TGA, GIF (first frame), PSD, HDR, PIC and PNM. HDR files are converted to 8 bits per channel.

`path` is opened as given. A relative path is relative to the process working directory, which for the editor is `applications/editor`.

### `dispose()`

Frees the pixels with `stbi_image_free` if there are any, sets `pixels` to `nullptr` and `size` to zero. Calling it twice is safe.

## Usage

The only user is `opengl_texture_t::load()`, which keeps the image on the stack just long enough to upload it:

```cpp
bool opengl_texture_t::load(const char* path)
{
    image::image_t image;

    if (!image.load(path))
    {
        return false;
    }

    return upload(image.pixels, image.size);
}
```

The pixels are freed by the destructor when `load()` returns. The texture cache calls this the first time a sprite asks for a path.

## Notes

- Decoding happens on the calling thread and blocks until the whole file is decoded.
- `pixels` and `size` are public and can be changed by the caller. `dispose()` only frees `pixels` if it is not null, and it frees it with `stbi_image_free`, so the pointer must still be the one stb_image returned.
