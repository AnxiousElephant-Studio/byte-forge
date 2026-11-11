# Módulo de gráficos

> Idioma: [English](../../../../en/engine/modules/graphics/README.md) | **Português (Brasil)**

O módulo de gráficos nomeia as APIs gráficas com que a engine poderia desenhar. Hoje ele tem um único enum, `e_graphic_backends`, usado para dizer à camada de platform que tipo de janela criar. Fica no namespace `byteforge::graphics` e é header-only. O desenho em si é feito pelo [módulo de renderização](../rendering/README.md).

## Arquivos

| Arquivo                                                               | Conteúdo                     |
| --------------------------------------------------------------------- | ---------------------------- |
| `engine/include/byte-forge-engine/modules/graphics/e_graphic_backends.h` | Enum `e_graphic_backends` |

Peças relacionadas em outros módulos:

| Arquivo                                                   | Papel                                                   |
| --------------------------------------------------------- | ------------------------------------------------------- |
| `modules/platform/display/display_descriptor.h`           | Guarda o backend escolhido                              |
| `modules/platform/display/window/window_manager.cpp`      | Adiciona `SDL_WINDOW_OPENGL` às janelas novas para `OpenGL` |
| `applications/editor/source/editor.cpp`                   | Monta o runtime com `e_graphic_backends::OpenGL`        |

## `e_graphic_backends`

```cpp
enum class e_graphic_backends : u8
{
    None       = 0,
    OpenGL     = 1,
    DirectX11  = 2,
    DirectX12  = 3,
    Vulkan     = 4,
    WebGPU     = 5,
    Metal      = 6,
    Software   = 7
};
```

O enum é com escopo e tem um byte. Os valores são fixados explicitamente, então continuam os mesmos se a lista for reordenada.

| Valor       | Significado no código atual                                    |
| ----------- | -------------------------------------------------------------- |
| `None`      | Nenhum backend. As janelas são criadas sem a flag de OpenGL    |
| `OpenGL`    | O único backend com efeito: as janelas recebem `SDL_WINDOW_OPENGL` |
| `DirectX11` | Declarado, sem referência em nenhum outro lugar                |
| `DirectX12` | Declarado, sem referência em nenhum outro lugar                |
| `Vulkan`    | Declarado, sem referência em nenhum outro lugar                |
| `WebGPU`    | Declarado, sem referência em nenhum outro lugar                |
| `Metal`     | Declarado, sem referência em nenhum outro lugar                |
| `Software`  | Declarado, sem referência em nenhum outro lugar                |

## Uso

O backend é escolhido uma vez, quando o runtime é montado, pelo `platform::display_descriptor_t`:

```cpp
struct display_descriptor_t
{
public:
    graphics::e_graphic_backends graphic_backend;

    explicit display_descriptor_t(const graphics::e_graphic_backends graphic_backend) : graphic_backend(graphic_backend) {}
};
```

O editor passa `OpenGL`:

```cpp
editor_t::editor_t()
    : runtime(platform::display_descriptor_t(graphics::e_graphic_backends::OpenGL), time::frame_settings_t(target_fps))
```

O `platform_t` guarda o descriptor, e o `window_manager_t::create_window()` lê ele quando monta as flags da janela do SDL. Quando o backend é `OpenGL`, adiciona `SDL_WINDOW_OPENGL`, que o SDL precisa antes de um contexto OpenGL poder ser criado para essa janela.

## Observações

- Só as flags da janela leem o enum. O `renderer_t` sempre cria contextos OpenGL, não importa o backend do descriptor. Com qualquer valor diferente de `OpenGL`, a janela fica sem `SDL_WINDOW_OPENGL` e é esperado que o `renderer_t::attach_window()` falhe ao tentar criar um contexto.
- O header inclui só o `api/types.h` e não declara símbolos exportados, então não precisa de `BF_GRAPHICS_API`.
