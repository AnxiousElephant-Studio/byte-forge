# Módulo de runtime

> Idioma: [English](../../../../en/engine/modules/runtime/README.md) | **Português (Brasil)**

O módulo de runtime junta os módulos centrais da engine. Ele é uma classe, `runtime_t`, que é dona da plataforma, do renderer, do state Lua, da engine de áudio e do timer de frames, inicializa todos numa ordem fixa, cria janelas com um contexto de renderização anexado e faz o dispose de tudo no final. Fica no namespace `byteforge::runtime` e não encapsula nenhuma biblioteca de terceiros diretamente.

Uma aplicação cria um `runtime_t` e alcança cada serviço central pelos seus membros públicos (`runtime.platform`, `runtime.renderer`, `runtime.scripting`, `runtime.audio`, `runtime.frame_manager`).

## Arquivos

| Arquivo                                                         | Conteúdo                                   |
| --------------------------------------------------------------- | ------------------------------------------ |
| `engine/include/byte-forge-engine/modules/runtime/runtime.h`    | Declaração e construtor do `runtime_t`     |
| `engine/source/modules/runtime/runtime.cpp`                     | Inicialização, janelas, render, dispose    |

Módulos dos quais o runtime é dono:

| Membro          | Tipo                       | Documentação                               |
| --------------- | -------------------------- | ------------------------------------------ |
| `frame_settings`| `time::frame_settings_t`   | [time](../time/README.md)                  |
| `frame_manager` | `time::frame_manager_t`    | [time](../time/README.md)                  |
| `platform`      | `platform::platform_t`     | [platform](../platform/README.md)          |
| `renderer`      | `rendering::renderer_t`    | [rendering](../rendering/README.md)        |
| `scripting`     | `scripting::scripting_t`   | [scripting](../scripting/README.md)        |
| `audio`         | `audio::audio_engine_t`    | [audio](../audio/README.md)                |

## Conceitos

### Posse por valor

Cada módulo é um membro comum do `runtime_t`, não um ponteiro. Eles são construídos na ordem de declaração quando o runtime é construído, mas os construtores só colocam valores vazios nos campos. Nada toca SDL, OpenGL, Lua ou miniaudio até `initialize()` rodar.

Outro código nunca cria sua própria cópia desses serviços. Ele recebe um ponteiro ou referência para o membro do runtime, como o editor faz com `runtime.audio` e `runtime.scripting.get_lua_state()`.

### A flag de fechamento

`should_close` começa como `false`. O construtor passa o endereço dela para a plataforma (`platform(display_descriptor, &should_close)`), e `platform_t::request_close()` coloca ela em `true`. A própria plataforma chama `request_close()` quando o SDL reporta um evento de quit, e uma aplicação também pode chamar. O loop principal da aplicação roda enquanto `should_close` for `false`.

### Ordem de inicialização

```
platform.initialize()       SDL video and events subsystems
scripting.initialize()      Lua state and standard libraries
audio.initialize()          miniaudio engine on the default device
renderer.initialize()       OpenGL context attributes (core profile and version)
frame_manager.reset()       frame clock starts, target frame time computed
```

### Ordem de dispose

```
audio.dispose()             stops every sound, frees the miniaudio engine
scripting.dispose()         closes the Lua state
renderer.dispose()          destroys every OpenGL context
platform.dispose()          destroys every window, quits SDL
```

A plataforma vai por último, então as janelas e o SDL ainda existem enquanto o renderer destrói os contextos OpenGL que pertencem a elas. O frame manager e as frame settings não guardam recursos e não passam por dispose.

## `runtime_t`

```cpp
class BF_RUNTIME_API runtime_t
{
public:
    bool should_close;
    time::frame_settings_t frame_settings;
    time::frame_manager_t frame_manager;
    platform::platform_t platform;
    rendering::renderer_t renderer;
    scripting::scripting_t scripting;
    audio::audio_engine_t audio;

    runtime_t(platform::display_descriptor_t display_descriptor, time::frame_settings_t frame_settings);

    bool initialize();
    platform::window_t* create_window(const platform::window_descriptor_t& descriptor);
    void destroy_window(u32 window_id);
    void render();
    void dispose();
};
```

Não há destrutor. A limpeza só acontece em `dispose()`, que o dono precisa chamar.

### Estado

| Campo            | Significado                                                          |
| ---------------- | -------------------------------------------------------------------- |
| `should_close`   | `true` depois que um fechamento foi pedido. O loop principal verifica |
| `frame_settings` | As configurações passadas ao construtor. Guarda `target_fps`         |
| `frame_manager`  | Timer de frames. Fornece `delta_time` e limita a taxa de frames      |
| `platform`       | Setup do SDL, o gerenciador de janelas e o polling de eventos        |
| `renderer`       | Um contexto OpenGL por janela, início e fim de frame                 |
| `scripting`      | O único state Lua                                                    |
| `audio`          | A engine do miniaudio                                                |

### `runtime_t(display_descriptor, frame_settings)`

Guarda `frame_settings`, coloca `should_close` em `false` e constrói a plataforma com o display descriptor e o endereço de `should_close`. Os outros membros são construídos com o construtor padrão. Nada é inicializado ainda.

### `initialize()`

1. Chama `platform.initialize()`, depois `scripting.initialize()`, depois `audio.initialize()`, parando no primeiro que retornar `false`. Nesse caso retorna `false` na hora, e os módulos depois do que falhou não são inicializados.
2. Chama `renderer.initialize()`, que não pode falhar.
3. Chama `frame_manager.reset(frame_settings)`, que registra a contagem de ticks atual e calcula o tempo alvo de frame.
4. Retorna `true`.

O runtime não loga nada por conta própria. Cada módulo loga a sua própria falha (`Could not initialize SDL: ...`, `Could not create the Lua state.`, `Could not initialize the audio engine (...)`). Os módulos que já estavam inicializados quando um posterior falha não são desfeitos aqui, então o dono ainda precisa chamar `dispose()`.

### `create_window(descriptor)`

1. Pede ao gerenciador de janelas da plataforma para criar uma janela a partir de `descriptor` e do display descriptor da plataforma. Se isso retornar `nullptr`, retorna `nullptr`.
2. Anexa a janela ao renderer, que cria um contexto OpenGL para ela. Se isso falhar, destrói a janela e retorna `nullptr`.
3. Retorna a janela.

A janela pertence ao gerenciador de janelas. O ponteiro retornado é um jeito de alcançá-la, não posse.

### `destroy_window(window_id)`

Desanexa a janela do renderer (destruindo o contexto OpenGL dela) e depois destrói a janela no gerenciador de janelas. `window_id` é o ID de janela do SDL, `window_t::sdl_window_id`.

### `render()`

Percorre todas as janelas do gerenciador de janelas. Para cada uma, `renderer.begin_frame()` torna o contexto dela atual, ajusta o viewport e limpa com a cor de limpeza do renderer. Se isso der certo, `renderer.end_frame()` troca os buffers dela. Uma janela cujo contexto não pode ser tornado atual é pulada.

`render()` não desenha nada além da cor de limpeza. O editor não chama ele; chama `renderer.begin_frame()` e `renderer.end_frame()` por conta própria para poder desenhar a cena e a UI no meio.

### `dispose()`

Faz o dispose do áudio, do scripting, do renderer e da plataforma, nessa ordem. Cada uma dessas funções `dispose()` verifica o seu próprio estado, então chamar o `dispose()` do runtime depois de um `initialize()` parcial ou que falhou é seguro.

## Uso

O editor é dono de um `runtime_t` e constrói ele no seu construtor:

```cpp
editor_t::editor_t()
    : runtime(platform::display_descriptor_t(graphics::e_graphic_backends::OpenGL), time::frame_settings_t(target_fps))
```

`editor_t::initialize()` chama `runtime.initialize()` primeiro e depois `runtime.create_window()`. Se algum falhar, loga um erro fatal e retorna `false`, e `main()` chama `editor.dispose()` mesmo assim, que termina com `runtime.dispose()`.

O loop principal usa os membros diretamente (encurtado aqui, sem as linhas de profiler e de input):

```cpp
while (!runtime.should_close)
{
    runtime.frame_manager.start_frame();
    runtime.platform.poll_events(input_callback, window_callback);
    scene_player.update(runtime.frame_manager.delta_time);
    runtime.audio.update();

    if (runtime.renderer.begin_frame(*window))
    {
        // scene and UI
        runtime.renderer.end_frame(*window);
    }

    runtime.frame_manager.end_frame();
}
```

O callback de janela chama `runtime.platform.request_close()` quando uma janela reporta `CloseRequested`, o que encerra o loop.

`editor_t::dispose()` faz o dispose do scene player, da cena, dos caches, do sprite renderer e do contexto de UI primeiro, e chama `runtime.dispose()` por último. O scene player remove suas referências Lua enquanto o state Lua ainda está aberto.

## Notas

- A ordem de dispose não é o inverso exato da ordem de inicialização. O áudio passa por dispose primeiro, e o renderer, que é inicializado por último, passa por dispose depois do scripting.
- Se `platform.initialize()` falhar, `initialize()` retorna antes de `scripting.initialize()` e `audio.initialize()` rodarem. As chamadas de `dispose()` deles então encontram um state nulo e uma engine nula e não fazem nada.
- `frame_manager.reset()` roda no fim de `initialize()`, então tudo o que a aplicação faz entre `initialize()` e o seu primeiro `end_frame()` (criar a janela, carregar shaders) conta para o `delta_time` do primeiro frame, que o frame manager limita a `0.05` segundo.
- `runtime_t` não deleta suas operações de cópia. Copiá-lo copiaria os membros dos módulos, incluindo seus ponteiros crus.
- `render()` não é usado pelo editor.
