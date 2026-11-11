# API

> Idioma: [English](../../../en/engine/api/README.md) | **Português (Brasil)**

A pasta API reúne os dois headers sobre os quais todos os outros headers da engine são construídos: os aliases de tipos primitivos e as macros de exportação. Nenhum dos dois tem arquivo de código-fonte.

## Arquivos

| Header    | Conteúdo                        | Finalidade                                      |
| --------- | ------------------------------- | ----------------------------------------------- |
| `types.h` | `u8`..`u64`, `i8`..`i64`, `f32`, `f64`, `usize`, `isize` | Aliases de tipos primitivos de tamanho fixo |
| `api.h`   | `BF_EXPORT`, `BF_IMPORT`        | Base das macros de exportação de cada módulo |

Todos os caminhos são relativos a `engine/include/byte-forge-engine/api/`. Inclua a partir da raiz de includes:

```cpp
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/api/api.h>
```

## `types.h`

Aliases curtos para os tipos de largura fixa da biblioteca padrão, declarados no namespace `byteforge`. O código da engine usa esses aliases no lugar de `int`, `float`, `std::uint32_t` e afins, para que todo tamanho fique explícito.

| Alias   | Tipo             | Tamanho |
| ------- | ---------------- | ------- |
| `u8`    | `std::uint8_t`   | 8 bits  |
| `u16`   | `std::uint16_t`  | 16 bits |
| `u32`   | `std::uint32_t`  | 32 bits |
| `u64`   | `std::uint64_t`  | 64 bits |
| `i8`    | `std::int8_t`    | 8 bits  |
| `i16`   | `std::int16_t`   | 16 bits |
| `i32`   | `std::int32_t`   | 32 bits |
| `i64`   | `std::int64_t`   | 64 bits |
| `f32`   | `float`          | 32 bits |
| `f64`   | `double`         | 64 bits |
| `usize` | `std::size_t`    | Tamanho de ponteiro, sem sinal |
| `isize` | `std::ptrdiff_t` | Tamanho de ponteiro, com sinal |

`float` e `double` não têm tamanho garantido pelo padrão, então o header verifica os dois com `static_assert`. O build falha numa plataforma em que `f32` não tenha 32 bits ou `f64` não tenha 64 bits.

Código dentro de um namespace `byteforge::<module>` enxerga os aliases sem prefixo. Código fora dele, como o editor, escreve `byteforge::u32` ou adiciona uma declaração `using`.

## `api.h`

A engine é compilada como biblioteca compartilhada (`set_kind("shared")` em `engine/xmake.lua`). Classes e funções que as aplicações chamam precisam ser exportadas por ela. O `api.h` define as duas peças básicas:

| Macro       | Windows                 | Outras plataformas                       | Com `BF_STATIC` |
| ----------- | ----------------------- | ---------------------------------------- | --------------- |
| `BF_EXPORT` | `__declspec(dllexport)` | `__attribute__((visibility("default")))` | vazio           |
| `BF_IMPORT` | `__declspec(dllimport)` | `__attribute__((visibility("default")))` | vazio           |

Nas outras plataformas as duas são iguais, e só escondem algo quando a biblioteca é compilada com `-fvisibility=hidden`, o que o build ainda não faz.

### Uma macro por módulo

Cada módulo que exporta símbolos tem o seu header, `modules/<módulo>/<módulo>_api.h`, com a sua macro:

```cpp
// modules/ecs/ecs_api.h
#if defined(BF_ECS_BUILD) || defined(BF_ENGINE_BUILD)
    #define BF_ECS_API BF_EXPORT
#else
    #define BF_ECS_API BF_IMPORT
#endif
```

| Módulo  | Macro            | Módulo    | Macro              |
| ------- | ---------------- | --------- | ------------------ |
| assets  | `BF_ASSETS_API`  | platform  | `BF_PLATFORM_API`  |
| audio   | `BF_AUDIO_API`   | project   | `BF_PROJECT_API`   |
| ecs     | `BF_ECS_API`     | rendering | `BF_RENDERING_API` |
| image   | `BF_IMAGE_API`   | runtime   | `BF_RUNTIME_API`   |
| io      | `BF_IO_API`      | scene     | `BF_SCENE_API`     |
| level   | `BF_LEVEL_API`   | scripting | `BF_SCRIPTING_API` |
| logging | `BF_LOGGING_API` | time      | `BF_TIME_API`      |
| physics | `BF_PHYSICS_API` |           |                    |

Por que uma por módulo: quando um módulo virar uma biblioteca própria, ele precisa exportar os seus símbolos e importar os dos módulos que usa. Com uma macro só, a biblioteca de rendering também marcaria como exportados os símbolos do ECS que ela usa. Com o `BF_<MÓDULO>_BUILD` definido só no target daquele módulo, cada biblioteca exporta exatamente o que é dela.

Hoje todos os módulos ainda são compilados na mesma biblioteca `byte-forge-engine`, que define `BF_ENGINE_BUILD`, então todas as macros de módulo exportam. Separar um módulo depois só precisa de um target que defina o seu `BF_<MÓDULO>_BUILD`; nenhum header muda.

Módulos só com código header-only (math, components, graphics, random) não têm macro, porque não há nada para exportar.

### `BF_STATIC`

Definir `BF_STATIC` (na engine e em toda aplicação que a usa) deixa `BF_EXPORT` e `BF_IMPORT` vazios, para compilar a engine como biblioteca estática.

## Uso

Inclua o header de API do módulo e coloque a macro entre `class` ou `struct` e o nome do tipo, ou antes de uma função livre:

```cpp
#include <byte-forge-engine/modules/audio/audio_api.h>

namespace byteforge::audio
{
    class BF_AUDIO_API audio_engine_t
    {
        ...
    };
}

namespace byteforge::scene
{
    BF_SCENE_API bool add_default_2D_systems(scene_context_t& scene_context);
}
```

Um módulo novo cria o seu `<módulo>_api.h`, copiando um dos existentes.

## Observações

- Esquecer a macro numa classe que tem `.cpp` faz a aplicação falhar no link com um símbolo não resolvido daquela classe.
- Definir `BF_ENGINE_BUILD` ou um `BF_<MÓDULO>_BUILD` no target de uma aplicação faz ela tentar exportar os símbolos da engine, o que também falha no link.
- As aplicações retornam `EXIT_SUCCESS` ou `EXIT_FAILURE` no `main`; a engine não tem um tipo próprio de código de resultado.
