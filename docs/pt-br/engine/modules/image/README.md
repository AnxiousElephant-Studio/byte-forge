# Módulo de imagem

> Idioma: [English](../../../../en/engine/modules/image/README.md) | **Português (Brasil)**

O módulo de imagem decodifica arquivos de imagem em pixels na memória. Ele é uma camada fina sobre o [stb_image](https://github.com/nothings/stb): uma struct, `image_t`, que carrega um arquivo como RGBA de 8 bits e libera os pixels depois. Fica no namespace `byteforge::image` e não mexe com a GPU; o envio dos pixels é feito pelo [módulo de renderização](../rendering/README.md).

## Arquivos

| Arquivo                                             | Conteúdo                                        |
| --------------------------------------------------- | ----------------------------------------------- |
| `engine/include/byte-forge-engine/modules/image/image.h` | Declaração do `image_t`                    |
| `engine/source/modules/image/image.cpp`             | Implementação e a implementação do stb_image    |

Peças relacionadas em outros módulos:

| Arquivo                                                       | Papel                                      |
| ------------------------------------------------------------- | ------------------------------------------ |
| `modules/rendering/opengl/texture/opengl_texture.cpp`         | `opengl_texture_t::load()` é o único chamador |

## Conceitos

### Como o stb_image fica escondido

O header não inclui nada do stb. O `image.cpp` define `STB_IMAGE_IMPLEMENTATION` antes de incluir `<stb_image.h>`, o que compila a biblioteca dentro da DLL da engine. Nenhum outro arquivo pode definir isso.

### Layout dos pixels

Os pixels são sempre 4 canais, vermelho, verde, azul e alpha, um byte cada, não importa o que o arquivo guarda. Um arquivo em tons de cinza ou RGB é expandido e recebe alpha opaco. As linhas são contíguas, então o buffer tem `size.x * size.y * 4` bytes e a primeira linha é o **topo** da imagem. Isso bate com as coordenadas de textura do quad do sprite renderer.

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

O construtor de cópia e a atribuição por cópia são deletados. Uma cópia apontaria para o mesmo buffer de pixels, e as duas o liberariam. Como há um destrutor e cópias deletadas declarados, nenhuma operação de move é gerada, então um `image_t` não pode ser movido.

O destrutor chama `dispose()`, então um `image_t` na pilha libera os pixels quando sai de escopo.

### Estado

| Campo    | Significado                                                          |
| -------- | -------------------------------------------------------------------- |
| `pixels` | Bytes RGBA alocados pelo stb_image, `nullptr` quando nada está carregado |
| `size`   | Largura (`x`) e altura (`y`) em pixels, `(0, 0)` quando nada está carregado |

### `load(path)`

1. Chama `dispose()`, então uma imagem anterior é liberada primeiro.
2. Chama `stbi_load` pedindo 4 canais. O stb_image preenche `size` e retorna os pixels.
3. Se falha, registra `Could not load image '<path>': <reason>`, onde `<reason>` é `stbi_failure_reason()`, zera `size` e retorna `false`.
4. Retorna `true` em caso de sucesso.

Os formatos são os que o stb_image decodifica na build padrão: PNG, JPEG, BMP, TGA, GIF (primeiro frame), PSD, HDR, PIC e PNM. Arquivos HDR são convertidos para 8 bits por canal.

O `path` é aberto como foi passado. Um caminho relativo é relativo ao diretório de trabalho do processo, que no editor é `applications/editor`.

### `dispose()`

Libera os pixels com `stbi_image_free` se houver, define `pixels` como `nullptr` e zera `size`. Pode ser chamado duas vezes com segurança.

## Uso

O único usuário é o `opengl_texture_t::load()`, que mantém a imagem na pilha só pelo tempo de enviá-la:

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

Os pixels são liberados pelo destrutor quando o `load()` retorna. O cache de texturas chama isso na primeira vez que um sprite pede um caminho.

## Observações

- A decodificação acontece na thread que chama e bloqueia até o arquivo inteiro ser decodificado.
- `pixels` e `size` são públicos e podem ser alterados por quem chama. O `dispose()` só libera `pixels` se não for nulo, e libera com `stbi_image_free`, então o ponteiro precisa ainda ser o que o stb_image retornou.
