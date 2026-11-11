# Módulo de plataforma

> Idioma: [English](../../../../en/engine/modules/platform/README.md) | **Português (Brasil)**

O módulo de plataforma é a ligação da engine com o sistema operacional. Ele inicia o SDL3, cria e destrói janelas, lê a fila de eventos do sistema e transforma os eventos do SDL nos eventos de janela e de input da própria engine. Ele também mantém um estado de teclado e mouse por frame, que o editor e os scripts Lua consultam. Tudo fica no namespace `byteforge::platform` e se divide em duas áreas: `display/window` (janelas e eventos de janela) e `input` (códigos de tecla, botões do mouse, eventos de input e estado de input).

O módulo não é header-only. Todo header que toca no SDL só faz forward declaration dos tipos do SDL, e os headers do SDL são incluídos nos arquivos `.cpp`.

## Arquivos

| Arquivo                                                                                 | Conteúdo                                                     |
| --------------------------------------------------------------------------------------- | ------------------------------------------------------------ |
| `engine/include/byte-forge-engine/modules/platform/platform.h`                          | Declaração do `platform_t`                                   |
| `engine/include/byte-forge-engine/modules/platform/native_event_callback.h`             | `native_event_callback_t`, wrapper para eventos crus do SDL  |
| `engine/include/byte-forge-engine/modules/platform/display/display_descriptor.h`        | `display_descriptor_t`, o backend gráfico escolhido          |
| `engine/include/byte-forge-engine/modules/platform/display/window/window.h`             | Declaração do `window_t`                                     |
| `engine/include/byte-forge-engine/modules/platform/display/window/window_descriptor.h`  | `window_descriptor_t` e `WINDOW_TITLE_MAX_LENGTH`            |
| `engine/include/byte-forge-engine/modules/platform/display/window/e_window_flags.h`     | `e_window_flags` e seus operadores de bits                   |
| `engine/include/byte-forge-engine/modules/platform/display/window/window_manager.h`     | Declaração do `window_manager_t`                             |
| `engine/include/byte-forge-engine/modules/platform/display/window/window_event.h`       | `window_event_t`                                             |
| `engine/include/byte-forge-engine/modules/platform/display/window/e_window_event_types.h` | `e_window_event_types`                                      |
| `engine/include/byte-forge-engine/modules/platform/display/window/window_event_callback.h` | `window_event_callback_t`                                 |
| `engine/include/byte-forge-engine/modules/platform/display/window/window_event_parser.h` | Declaração do `window_event_parser_t`                       |
| `engine/include/byte-forge-engine/modules/platform/input/e_key_codes.h`                 | `e_key_codes` e `KEYCODES_TABLE_SIZE`                        |
| `engine/include/byte-forge-engine/modules/platform/input/e_mouse_buttons.h`             | `e_mouse_buttons` e `MOUSE_BUTTONS_TABLE_SIZE`               |
| `engine/include/byte-forge-engine/modules/platform/input/e_input_event_types.h`         | `e_input_event_types`                                        |
| `engine/include/byte-forge-engine/modules/platform/input/e_input_states.h`              | `e_input_states`                                             |
| `engine/include/byte-forge-engine/modules/platform/input/input_event.h`                 | `input_event_t`                                              |
| `engine/include/byte-forge-engine/modules/platform/input/input_event_callback.h`        | `input_event_callback_t`                                     |
| `engine/include/byte-forge-engine/modules/platform/input/input_event_parser.h`          | Declaração do `input_event_parser_t`                         |
| `engine/include/byte-forge-engine/modules/platform/input/input_names.h`                 | Declaração do `input_names_t`                                |
| `engine/include/byte-forge-engine/modules/platform/input/input_state.h`                 | Declaração do `input_state_t`                                |
| `engine/source/modules/platform/platform.cpp`                                           | Início e encerramento do SDL, o loop de eventos              |
| `engine/source/modules/platform/display/window/window.cpp`                              | Métodos do `window_t`                                        |
| `engine/source/modules/platform/display/window/window_manager.cpp`                      | Criação, busca e destruição de janelas, mapeamento das flags do SDL |
| `engine/source/modules/platform/display/window/window_event_parser.cpp`                 | Eventos de janela do SDL para `window_event_t`               |
| `engine/source/modules/platform/input/input_event_parser.cpp`                           | Tabelas de scancodes e botões do mouse, eventos de input do SDL para `input_event_t` |
| `engine/source/modules/platform/input/input_names.cpp`                                  | Tabelas de nomes de teclas e botões do mouse                 |
| `engine/source/modules/platform/input/input_state.cpp`                                  | Métodos do `input_state_t`                                   |

Peças relacionadas em outros módulos:

| Arquivo                                                       | Papel                                                        |
| ------------------------------------------------------------- | ------------------------------------------------------------ |
| `modules/runtime/runtime.h` e `runtime.cpp`                   | Donos do `platform_t`, da flag `should_close` e da criação de janelas |
| `modules/rendering/window/renderer.cpp`                       | Cria um contexto GL por janela, aplica o `use_vsync`         |
| `modules/rendering/window/opengl_window_context.cpp`          | Usa o `window_t::sdl_window` para criar o contexto GL        |
| `modules/scene/scene_player.cpp`                              | Liga, desliga e limpa o estado de input                      |
| `modules/scene/scene_script_system.cpp`                       | Tabela `input` em Lua                                        |
| `applications/editor/source/editor.cpp`                       | Dono do `input_state_t`, roda o loop de eventos              |
| `applications/editor/source/ui/debug_ui_context.cpp`          | Recebe os eventos crus do SDL para o ImGui                   |

## Conceitos

### Como o SDL fica escondido

Os headers nunca incluem o SDL. O `window.h` faz forward declaration de `struct SDL_Window;`, e os dois parsers fazem forward declaration de `union SDL_Event;`. O callback de evento cru recebe um `const void*` em vez de um `SDL_Event`. Código que inclui os headers da plataforma não precisa do SDL no include path, exceto código que quer ler o evento cru, que converte o ponteiro de volta para `SDL_Event` por conta própria (a UI de debug do editor faz isso).

O `window_t` expõe o `SDL_Window*` e o id da janela no SDL como campos públicos, porque o renderer e a UI de debug precisam deles para criar contextos GL e inicializar o ImGui.

### Fluxo de eventos

Uma chamada a `platform_t::poll_events()` esvazia a fila do SDL. Cada evento do SDL passa por três etapas, nesta ordem:

```
SDL_PollEvent
  └─► native_event_callback(&sdl_event)          every event, untranslated
  └─► SDL_EVENT_QUIT?  request_close(), return
  └─► window_event_parser_t::parse ─► window_event_callback
  └─► input_event_parser_t::parse  ─► input_event_callback
```

A plataforma não guarda nenhum evento. Ela só traduz e repassa. Manter estado (quais teclas estão pressionadas, a posição do mouse) é trabalho do `input_state_t`, que o dono alimenta a partir do callback de input.

### Ids de janela

Uma janela é identificada pelo id `u32` que o SDL atribui a ela (`SDL_GetWindowID`). Eventos de janela e de input carregam em `window_id` o id da janela a que pertencem. O `window_manager_t::get_window()` e o `destroy_window()` recebem esse id, e o renderer indexa seus contextos GL por ele.

### Códigos de tecla são teclas físicas

O parser de input lê `sdl_event.key.scancode`, não o símbolo da tecla. Scancodes do SDL descrevem a posição física de uma tecla, com nomes do layout americano. Então `e_key_codes::A` significa a tecla na posição do A num teclado americano, seja qual for o caractere que o layout do usuário imprime nela.

### Pressionada, segurada e solta

O `input_state_t` guarda três visões de cada tecla e botão do mouse:

| Visão      | Verdadeira quando                                               |
| ---------- | --------------------------------------------------------------- |
| `down`     | A tecla desceu e ainda não subiu                                |
| `pressed`  | A tecla desceu durante o frame atual                            |
| `released` | A tecla subiu durante o frame atual                             |

`pressed`, `released` e o delta da roda são zerados pelo `begin_frame()`, então descrevem só os eventos lidos desde o último `begin_frame()`.

## `platform_t`

```cpp
class BF_PLATFORM_API platform_t
{
public:
    display_descriptor_t    display_descriptor;
    window_manager_t        window_manager;
    bool*                   should_close_handler;
    native_event_callback_t native_event_callback;

    platform_t(display_descriptor_t display_descriptor, bool* should_close_handler);

    bool initialize();
    void request_close();
    void poll_events(const input_event_callback_t& input_event_callback, const window_event_callback_t& window_event_callback);
    void dispose();

private:
    bool initialized;
};
```

Não há destrutor. O SDL é encerrado e as janelas são liberadas só no `dispose()`.

### Estado

| Campo                   | Significado                                                          |
| ----------------------- | -------------------------------------------------------------------- |
| `display_descriptor`    | Backend gráfico, passado ao gerenciador de janelas em toda criação de janela |
| `window_manager`        | Dono de todas as janelas                                             |
| `should_close_handler`  | Ponteiro para uma flag de outro dono. O `request_close()` a põe em `true` |
| `native_event_callback` | Chamado com cada evento cru do SDL. Vazio por padrão                 |
| `initialized`           | `true` entre um `initialize()` bem-sucedido e o `dispose()`          |

### `initialize()`

Chama `SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS)`. Se falhar, loga `Could not initialize SDL: <error>` em nível fatal, com o texto de erro do SDL, e retorna `false`. Se der certo, marca `initialized` e retorna `true`.

### `request_close()`

Escreve `true` através do `should_close_handler`, se o ponteiro não for nulo. Não fecha nenhuma janela nem para o SDL; só levanta a flag, e o loop do dono decide o que fazer com ela.

### `poll_events(input_event_callback, window_event_callback)`

1. Chama `SDL_PollEvent` até a fila ficar vazia.
2. Para cada evento, chama o `native_event_callback` com um ponteiro para o `SDL_Event`.
3. Se o evento for `SDL_EVENT_QUIT`, chama `request_close()` e retorna na hora. Eventos que ainda estão na fila ficam lá para a próxima chamada.
4. Monta um `window_event_t`. Se o `window_event_parser_t::parse()` aceitar o evento, chama o `window_event_callback`.
5. Monta um `input_event_t`. Se o `input_event_parser_t::parse()` aceitar o evento, chama o `input_event_callback`.

Os dois parsers veem todo evento. Na prática um evento do SDL é traduzido por no máximo um deles, já que cada um trata tipos de evento diferentes.

### `dispose()`

1. Chama `window_manager.dispose()`, que destrói todas as janelas. Isso acontece mesmo se o `initialize()` nunca foi chamado ou falhou.
2. Se a plataforma foi inicializada, encerra os subsistemas de eventos e de vídeo, chama `SDL_Quit()` e limpa `initialized`.

Chamar duas vezes é seguro: a lista de janelas já está vazia e `initialized` é `false`.

## `display_descriptor_t`

```cpp
struct display_descriptor_t
{
public:
    graphics::e_graphic_backends graphic_backend;

    explicit display_descriptor_t(const graphics::e_graphic_backends graphic_backend);
};
```

Descreve o lado de display da plataforma. Hoje guarda só o backend gráfico. O gerenciador de janelas o lê quando monta as flags de janela do SDL: `OpenGL` adiciona `SDL_WINDOW_OPENGL`, e qualquer outro valor de `e_graphic_backends` não adiciona nada.

## `window_descriptor_t`

```cpp
constexpr usize WINDOW_TITLE_MAX_LENGTH = 256;

struct window_descriptor_t
{
public:
    char           title[WINDOW_TITLE_MAX_LENGTH];
    math::recti_t  bounds;
    e_window_flags flags;
    bool           use_vsync;

    window_descriptor_t(const char* window_title, const math::recti_t& window_bounds, const e_window_flags window_flags, bool use_vsync);
};
```

As configurações com que uma janela é criada. Uma cópia fica guardada dentro do `window_t`.

### Estado

| Campo       | Significado                                                             |
| ----------- | ----------------------------------------------------------------------- |
| `title`     | Título da janela. Copiado com `snprintf`, então é cortado em 255 caracteres mais o terminador |
| `bounds`    | Posição e tamanho. O tamanho vira o tamanho da janela; a posição só é aplicada quando não é `(0, 0)` |
| `flags`     | `e_window_flags` a aplicar                                              |
| `use_vsync` | Não é lido pela plataforma. O renderer o lê quando anexa um contexto GL à janela |

## `e_window_flags`

```cpp
enum class e_window_flags : u8
{
    None       = 0,
    Resizable  = (1 << 0),
    Borderless = (1 << 1),
    Fullscreen = (1 << 2),
    Hidden     = (1 << 3),
    Maximized  = (1 << 4),
    Minimized  = (1 << 5),
};

constexpr e_window_flags  operator| (e_window_flags left_flag, e_window_flags right_flag);
constexpr e_window_flags  operator& (e_window_flags left_flag, e_window_flags right_flag);
constexpr e_window_flags& operator|=(e_window_flags& left_flag, e_window_flags right_flag);
constexpr bool            has_flag(e_window_flags flags, e_window_flags flag);
```

Um conjunto de bits. Os operadores combinam e mascaram flags sem casts no ponto de chamada, e o `has_flag()` retorna `true` quando todos os bits de `flag` estão ligados em `flags`. Cada flag vira a flag do SDL de mesmo nome (`SDL_WINDOW_RESIZABLE`, `SDL_WINDOW_BORDERLESS` e assim por diante). `SDL_WINDOW_HIGH_PIXEL_DENSITY` é sempre adicionada, sejam quais forem as flags.

## `window_t`

```cpp
struct BF_PLATFORM_API window_t
{
public:
    SDL_Window*         sdl_window;
    u32                 sdl_window_id;
    window_descriptor_t descriptor;

    window_t(const window_descriptor_t& descriptor, SDL_Window* sdl_window, u32 sdl_window_id);

    void          set_title(const char* new_title) const;
    bool          is_valid() const;
    math::vec2i_t get_size() const;
    void          destroy();
};
```

Uma janela do sistema operacional. É criada e apagada pelo `window_manager_t`; o resto do código recebe um `window_t*` ou uma referência a ela.

### Estado

| Campo           | Significado                                                     |
| --------------- | --------------------------------------------------------------- |
| `sdl_window`    | A janela do SDL. `nullptr` depois do `destroy()`                |
| `sdl_window_id` | O id da janela no SDL. `0` depois do `destroy()`                |
| `descriptor`    | Cópia do descritor com que a janela foi criada                  |

### `set_title(new_title)`

Chama `SDL_SetWindowTitle`. O novo título não é copiado para `descriptor.title`, que mantém o original.

### `is_valid()`

`true` quando `sdl_window` não é nulo e `sdl_window_id` não é `0`.

### `get_size()`

Retorna o tamanho desenhável em pixels, via `SDL_GetWindowSizeInPixels`. Como as janelas são criadas com `SDL_WINDOW_HIGH_PIXEL_DENSITY`, numa tela com escala isso pode ser maior que o tamanho dado no descritor. Se o SDL falhar, retorna `(0, 0)`.

### `destroy()`

Destrói a janela do SDL se houver uma, põe `sdl_window` em `nullptr` e `sdl_window_id` em `0`. Chamar de novo não faz nada. Não libera o próprio `window_t`; isso é feito pelo gerenciador de janelas.

## `window_manager_t`

```cpp
class BF_PLATFORM_API window_manager_t
{
public:
    window_t* create_window(const window_descriptor_t& descriptor, const display_descriptor_t& display_descriptor);
    window_t* get_window(const u32 window_id);
    window_t* get_window_at(const usize index);
    usize     get_window_count() const;
    void      destroy_window(const u32 window_id);
    void      dispose();

private:
    std::vector<window_t*> windows;
};
```

Dono de todas as janelas. Cada `window_t` é alocado com `new`, então seu endereço não muda enquanto outras janelas são adicionadas ou removidas.

### Estado

| Campo     | Significado                                     |
| --------- | ----------------------------------------------- |
| `windows` | Todas as janelas vivas, em ordem de criação     |

### `create_window(descriptor, display_descriptor)`

1. Retorna `nullptr` se `descriptor.bounds` estiver vazio (largura ou altura não positiva).
2. Monta as flags do SDL: `SDL_WINDOW_HIGH_PIXEL_DENSITY`, uma flag do SDL por bit de `e_window_flags`, e `SDL_WINDOW_OPENGL` quando o backend é `OpenGL`.
3. Chama `SDL_CreateWindow` com o título, a largura e a altura. Retorna `nullptr` se o SDL falhar.
4. Se `descriptor.bounds.position` não for `(0, 0)`, move a janela para lá com `SDL_SetWindowPosition`. Caso contrário a posição fica a cargo do SDL.
5. Aloca um `window_t` com o descritor, a janela do SDL e seu id, adiciona em `windows` e o retorna.

Nenhuma das falhas é logada aqui. O `runtime_t::create_window()` envolve essa chamada e também não loga nada; quem chama (o editor) loga `Could not create the editor window.`.

### Busca

| Membro               | Retorna                                                         |
| -------------------- | --------------------------------------------------------------- |
| `get_window(id)`     | A janela com esse id do SDL, ou `nullptr`. Busca linear         |
| `get_window_at(i)`   | A janela na posição `i` em ordem de criação, ou `nullptr` se fora do intervalo |
| `get_window_count()` | Número de janelas vivas                                         |

### `destroy_window(window_id)`

Acha a janela com esse id, chama `destroy()`, a apaga e a remove da lista. Um id desconhecido não faz nada. Qualquer `window_t*` guardado em outro lugar para essa janela fica pendurado, então quem chama precisa descartá-lo.

### `dispose()`

Destrói e apaga todas as janelas, depois limpa a lista. Chamar duas vezes é seguro.

## `window_event_t`

```cpp
enum class e_window_event_types : u8
{
    None,
    Resize,
    CloseRequested
};

struct window_event_t
{
public:
    e_window_event_types type;
    u32                  window_id;

    union
    {
        struct
        {
            math::vec2i_t new_size;
        } resize;
    };
};
```

Um evento de janela. `type` diz qual membro da union é válido; só `Resize` tem dados. O construtor padrão põe `type` em `None`, `window_id` em `0` e zera `resize`.

## `window_event_parser_t`

```cpp
class BF_PLATFORM_API window_event_parser_t
{
public:
    static bool parse(const SDL_Event& sdl_event, window_event_t& event);
};
```

### `parse(sdl_event, event)`

1. Copia `sdl_event.window.windowID` para `event.window_id`. Isso é feito antes de checar o tipo, então o campo é escrito mesmo quando a função depois retorna `false`.
2. `SDL_EVENT_WINDOW_RESIZED`: põe `type` em `Resize`, pede ao SDL o tamanho atual em pixels dessa janela (`SDL_GetWindowSizeInPixels`) e o guarda em `resize.new_size`. O tamanho que vem no próprio evento do SDL não é usado. Retorna `true`.
3. `SDL_EVENT_WINDOW_CLOSE_REQUESTED`: põe `type` em `CloseRequested` e retorna `true`.
4. Qualquer outro evento retorna `false`.

## Callbacks de evento

```cpp
struct native_event_callback_t
{
    std::function<void(const void*)> callback;
    void operator()(const void* native_event) const;
};

struct window_event_callback_t
{
    std::function<void(const window_event_t&)> callback;
    void operator()(const window_event_t& event) const;
};

struct input_event_callback_t
{
    std::function<void(const input_event_t&)> callback;
    void operator()(const input_event_t& event) const;
};
```

Três wrappers pequenos com o mesmo formato. Cada um guarda uma `std::function` e só a chama no `operator()` se ela estiver definida, então um callback vazio é um valor válido de "ignore esses eventos". O nativo recebe um ponteiro para o `SDL_Event`; ele só é válido durante a chamada.

## `input_event_t`

```cpp
enum class e_input_event_types : u8 { None, Key, MouseButton, MouseMove, MouseWheel };
enum class e_input_states      : u8 { Pressed = 0, Released = 1, Held = 2 };

struct input_event_t
{
public:
    e_input_event_types type;
    u32                 window_id;

    union
    {
        struct { e_key_codes code; e_input_states state; }                      key;
        struct { e_mouse_buttons button; e_input_states state; f32 x, y; }      mouse_button;
        struct { f32 x, y; f32 delta_x, delta_y; }                              mouse_move;
        struct { f32 delta_x, delta_y; }                                        mouse_wheel;
    };
};
```

Um evento de teclado ou mouse, já traduzido. `type` diz qual membro da union é válido. O construtor padrão põe `type` em `None`, `window_id` em `0` e zera `key`.

| Tipo          | Membro         | Dados                                                         |
| ------------- | -------------- | ------------------------------------------------------------- |
| `Key`         | `key`          | Código de tecla da engine e `Pressed`, `Released` ou `Held`   |
| `MouseButton` | `mouse_button` | Botão da engine, `Pressed` ou `Released`, posição do cursor   |
| `MouseMove`   | `mouse_move`   | Posição do cursor e movimento desde o evento de movimento anterior |
| `MouseWheel`  | `mouse_wheel`  | Quantidade de rolagem horizontal e vertical                   |

`Held` só é usado em eventos de auto-repetição de tecla. Botões do mouse nunca são `Held`.

## `e_key_codes`

```cpp
constexpr u32 KEYCODES_TABLE_SIZE = 512;

enum class e_key_codes : u16
{
    Unknown   = 0,
    Backspace = 8, Tab = 9, Return = 13, Escape = 27, Space = 32, Delete = 127,
    Insert    = 256, Home, End, PageUp, PageDown, ArrowLeft, ArrowRight, ArrowUp, ArrowDown,
    // CapsLock ... Menu, F1 ... F24, Numpad0 ... NumpadEqual, A ... Z,
    // Num0 ... Num9, Minus ... Slash, MediaPlay ... MediaMute
};
```

Os códigos de tecla da própria engine. As primeiras teclas reaproveitam seus valores ASCII; o resto conta a partir de `256`, e o último valor (`MediaMute`) fica bem abaixo de `512`. `KEYCODES_TABLE_SIZE` é o tamanho de todo array por tecla: a tabela de scancodes no parser e os três arrays de teclas do `input_state_t`. É também a contagem de scancodes do SDL, o que deixa o parser indexar sua tabela direto pelo scancode.

## `e_mouse_buttons`

```cpp
constexpr u8 MOUSE_BUTTONS_TABLE_SIZE = 10;

enum class e_mouse_buttons : u8
{
    None = 0, Left = 1, Right = 2, Middle = 3,
    WheelUp = 4, WheelDown = 5, WheelLeft = 6, WheelRight = 7,
    XButton1 = 8, XButton2 = 9,
};
```

Os botões do mouse da engine. `MOUSE_BUTTONS_TABLE_SIZE` é o tamanho da tabela de botões no parser e dos três arrays de botões do `input_state_t`. Rolar a roda não gera os botões `Wheel*`; gera eventos `MouseWheel` (veja o parser abaixo).

## `input_event_parser_t`

```cpp
class BF_PLATFORM_API input_event_parser_t
{
public:
    static const std::array<e_key_codes, KEYCODES_TABLE_SIZE>          keycodes_mapper;
    static const std::array<e_mouse_buttons, MOUSE_BUTTONS_TABLE_SIZE> mouse_buttons_mapper;

    static bool parse(const SDL_Event& sdl_event, input_event_t& event);

private:
    static std::array<e_key_codes, KEYCODES_TABLE_SIZE> create_keycodes_mapper();
};
```

### Estado

| Campo                  | Significado                                                          |
| ---------------------- | -------------------------------------------------------------------- |
| `keycodes_mapper`      | Indexada por scancode do SDL. Toda entrada começa como `Unknown`, depois o `create_keycodes_mapper()` preenche uma entrada por tecla suportada |
| `mouse_buttons_mapper` | Indexada por número de botão do SDL. A entrada `i` é o valor `i` de `e_mouse_buttons`, de `None` a `XButton2` |

As duas tabelas são estáticas e montadas uma vez, quando a biblioteca da engine é carregada.

### `parse(sdl_event, event)`

| Evento do SDL                                        | Resultado                                                   |
| ---------------------------------------------------- | ----------------------------------------------------------- |
| `SDL_EVENT_KEY_DOWN`, `SDL_EVENT_KEY_UP`             | `Key`. O código vem de `keycodes_mapper[scancode]`. O estado é `Held` se o SDL marca o evento como repetição, senão `Pressed` para tecla descendo e `Released` para tecla subindo |
| `SDL_EVENT_MOUSE_BUTTON_DOWN`, `SDL_EVENT_MOUSE_BUTTON_UP` | `MouseButton`. O botão vem de `mouse_buttons_mapper[button]`, o estado é `Pressed` ou `Released`, e `x`, `y` são a posição do cursor |
| `SDL_EVENT_MOUSE_MOTION`                             | `MouseMove` com `x`, `y` e o movimento relativo `xrel`, `yrel` |
| `SDL_EVENT_MOUSE_WHEEL`                              | `MouseWheel` com as quantidades de rolagem `x` e `y` do SDL |
| Qualquer outro                                       | Retorna `false`                                             |

Uma tecla com scancode `512` ou maior, ou um botão do mouse com número `10` ou maior, é rejeitado com `false`. Um scancode dentro do intervalo mas fora da tabela vira `e_key_codes::Unknown`, e o evento é retornado mesmo assim. Todo evento aceito também copia o `windowID` do SDL para `event.window_id`.

## `input_names_t`

```cpp
struct BF_PLATFORM_API input_names_t
{
public:
    static bool get_key_code(const char* name, e_key_codes& out_key_code);
    static bool get_mouse_button(const char* name, e_mouse_buttons& out_mouse_button);
};
```

Traduz nomes em códigos, para os scripts Lua. Cada função percorre uma tabela fixa com `strcmp`. Quando encontra, escreve o código no parâmetro de saída e retorna `true`; senão retorna `false` e não mexe na saída.

Os nomes de tecla são os nomes dos enumeradores de `e_key_codes`, escritos exatamente igual (`"A"`, `"Num1"`, `"ArrowLeft"`, `"F12"`, `"NumpadEnter"`, `"MediaMute"`, ...). `Unknown` não tem nome. A comparação diferencia maiúsculas, então `"a"` e `"space"` não são encontrados.

Os nomes de botão do mouse são `"Left"`, `"Right"`, `"Middle"`, `"XButton1"` e `"XButton2"`. Os valores `None` e `Wheel*` não têm nome.

## `input_state_t`

```cpp
struct BF_PLATFORM_API input_state_t
{
public:
    bool         enabled;
    bool         keys_down[KEYCODES_TABLE_SIZE];
    bool         keys_pressed[KEYCODES_TABLE_SIZE];
    bool         keys_released[KEYCODES_TABLE_SIZE];
    bool         mouse_buttons_down[MOUSE_BUTTONS_TABLE_SIZE];
    bool         mouse_buttons_pressed[MOUSE_BUTTONS_TABLE_SIZE];
    bool         mouse_buttons_released[MOUSE_BUTTONS_TABLE_SIZE];
    math::vec2_t mouse_position;
    math::vec2_t mouse_wheel;

    input_state_t();

    void begin_frame();
    void process(const input_event_t& event);
    void clear();

    bool is_key_down(const e_key_codes key_code) const;
    bool is_key_pressed(const e_key_codes key_code) const;
    bool is_key_released(const e_key_codes key_code) const;
    bool is_mouse_button_down(const e_mouse_buttons mouse_button) const;
    bool is_mouse_button_pressed(const e_mouse_buttons mouse_button) const;
    bool is_mouse_button_released(const e_mouse_buttons mouse_button) const;
};
```

O estado de teclado e mouse de um frame, montado a partir de valores `input_event_t`. Os arrays são indexados pelo valor numérico de `e_key_codes` e `e_mouse_buttons`.

### Estado

| Campo                       | Significado                                                    |
| --------------------------- | -------------------------------------------------------------- |
| `enabled`                   | Quando `false`, eventos de tecla, botão e roda são ignorados. Começa `true` |
| `keys_down` e `mouse_buttons_down` | Segurados agora                                         |
| `keys_pressed` e `mouse_buttons_pressed` | Desceram desde o último `begin_frame()`           |
| `keys_released` e `mouse_buttons_released` | Subiram desde o último `begin_frame()`          |
| `mouse_position`            | Última posição do cursor vinda de um evento `MouseMove`        |
| `mouse_wheel`               | Soma dos deltas da roda desde o último `begin_frame()`         |

### Construtor

Põe `enabled` em `true`, zera os dois vetores e chama `clear()`.

### `begin_frame()`

Limpa os arrays `pressed` e `released` e zera `mouse_wheel`. Os arrays `down` e o `mouse_position` são mantidos. Chame uma vez por frame, antes de ler os eventos.

### `process(event)`

1. Um evento `MouseMove` atualiza `mouse_position`. Isso acontece mesmo quando `enabled` é `false`.
2. Se `enabled` for `false`, para aqui.
3. `Key`: em `Pressed`, liga `down` e `pressed` daquela tecla; em `Released`, desliga `down` e liga `released`. `Held` não muda nada, já que a tecla já está pressionada.
4. `MouseButton`: igual às teclas, usando os arrays de botões. O `x`, `y` do evento não são usados.
5. `MouseWheel`: soma os deltas em `mouse_wheel`.

Códigos fora do intervalo dos arrays são ignorados.

### `clear()`

Limpa os arrays `down` e depois chama `begin_frame()`. Depois dele, nada está segurado, pressionado ou solto. `mouse_position` e `enabled` não mudam.

### Consultas

| Membro                             | Retorna                                         |
| ---------------------------------- | ----------------------------------------------- |
| `is_key_down(code)`                | `keys_down[code]`                               |
| `is_key_pressed(code)`             | `keys_pressed[code]`                            |
| `is_key_released(code)`            | `keys_released[code]`                           |
| `is_mouse_button_down(button)`     | `mouse_buttons_down[button]`                    |
| `is_mouse_button_pressed(button)`  | `mouse_buttons_pressed[button]`                 |
| `is_mouse_button_released(button)` | `mouse_buttons_released[button]`                |

Cada uma retorna `false` para um valor fora do intervalo do array em vez de ler além dele.

## Uso

### Quem é o dono

O `runtime_t` tem um membro `platform::platform_t platform` e um membro `bool should_close`. Seu construtor passa `&should_close` como `should_close_handler` da plataforma, então o `request_close()` acaba ligando `runtime.should_close`.

- `runtime_t::initialize()` inicializa a plataforma primeiro, depois o scripting e o áudio. Se qualquer um falhar, retorna `false`.
- `runtime_t::create_window()` chama `window_manager.create_window()` com o descritor de display da plataforma, depois `renderer.attach_window()`. Se o renderer não conseguir anexar um contexto GL, a janela é destruída de novo e `nullptr` é retornado.
- `runtime_t::destroy_window()` desanexa a janela do renderer e depois a destrói.
- `runtime_t::render()` percorre as janelas com `get_window_count()` e `get_window_at()`.
- `runtime_t::dispose()` faz o dispose da plataforma **por último**, depois do áudio, do scripting e do renderer.

O `input_state_t` não pertence à plataforma nem ao runtime. O editor tem seu próprio membro `platform::input_state_t input_state` e o passa por referência ao `scene_player_t::initialize()`, que o entrega ao sistema de scripts.

### Setup do editor

O editor cria o runtime com `display_descriptor_t(graphics::e_graphic_backends::OpenGL)` e uma janela:

```cpp
platform::window_descriptor_t descriptor(window_title, window_bounds, platform::e_window_flags::Resizable | platform::e_window_flags::Maximized, true);
window = runtime.create_window(descriptor);
```

`window_title` é `"ByteForge Editor"` e `window_bounds` é `(0, 0, 1600, 900)`, então a posição fica a cargo do SDL. Em seguida ele define o callback nativo para repassar todo evento do SDL ao ImGui (`ImGui_ImplSDL3_ProcessEvent`), e monta os outros dois callbacks no `run()`:

```cpp
input_callback.callback = [this](const platform::input_event_t& event)
{
    input_state.process(event);
};

window_callback.callback = [this](const platform::window_event_t& event)
{
    if (event.type == platform::e_window_event_types::CloseRequested)
    {
        runtime.platform.request_close();
    }
};
```

O editor não reage a eventos `Resize`. Ele lê `window->get_size()` a cada frame quando renderiza a cena.

O painel de status mostra `window_manager.get_window_count()`.

### Ordem do frame no editor

```
while (!runtime.should_close)
    frame_manager.start_frame()
    input_state.begin_frame()             -- clears pressed, released, wheel
    platform.poll_events(...)             -- fills input_state, may request close
    scene_player.update(delta_time)       -- Lua reads input_state
    audio.update()
    render
    frame_manager.end_frame()
```

### Reprodução da cena

O `scene_player_t` liga e desliga o estado de input:

- `start()` chama `clear()` e põe `enabled` em `true`.
- `stop()` põe `enabled` em `false` e chama `clear()`.

Enquanto a cena está parada, eventos de tecla, botão e roda são descartados, e só a posição do mouse continua sendo atualizada.

### API em Lua

Os scripts recebem uma tabela `input`:

| Função                            | Retorna          | Descrição                                       |
| --------------------------------- | ---------------- | ----------------------------------------------- |
| `input.is_key_down(name)`         | boolean          | `is_key_down` para a tecla com esse nome        |
| `input.is_key_pressed(name)`      | boolean          | `is_key_pressed` para a tecla com esse nome     |
| `input.is_key_released(name)`     | boolean          | `is_key_released` para a tecla com esse nome    |
| `input.is_mouse_down(name)`       | boolean          | `is_mouse_button_down` para o botão com esse nome |
| `input.is_mouse_pressed(name)`    | boolean          | `is_mouse_button_pressed` para o botão com esse nome |
| `input.is_mouse_released(name)`   | boolean          | `is_mouse_button_released` para o botão com esse nome |
| `input.get_mouse_position()`      | `x`, `y`         | `mouse_position` como dois números              |
| `input.get_mouse_wheel()`         | `x`, `y`         | `mouse_wheel` como dois números                 |

Os nomes passam pelo `input_names_t`. Um nome de tecla desconhecido gera o erro Lua `unknown key name '<name>'`, e um nome de botão desconhecido gera `unknown mouse button name '<name>'`. As funções chegam ao `input_state_t` através de um light userdata guardado como upvalue `3`.

De `applications/editor/assets/scripts/demo.lua`:

```lua
if input.is_key_down("ArrowLeft")  or input.is_key_down("A") then direction_x = direction_x - 1 end
if input.is_key_down("ArrowRight") or input.is_key_down("D") then direction_x = direction_x + 1 end

if input.is_key_pressed("Space") then
    transform.set_position(entity, 0, 0)
    log.info("Space pressed: position reset")
end
```

## Observações

- O `poll_events()` retorna assim que vê um `SDL_EVENT_QUIT`. O evento de quit ainda chega primeiro ao callback nativo, e os eventos atrás dele na fila ficam para uma chamada posterior.
- A tabela de botões do mouse mapeia os números de botão do SDL direto para valores de `e_mouse_buttons`. O SDL3 numera seus botões como esquerdo `1`, meio `2`, direito `3`, X1 `4` e X2 `5`, então com a tabela atual o botão do meio físico chega como `Right`, o botão direito físico como `Middle`, e os dois botões laterais como `WheelUp` e `WheelDown`. `XButton1` e `XButton2` só correspondem aos números de botão `8` e `9` do SDL.
- Teclas que não estão na tabela de scancodes chegam como `Unknown`, cujo índice é `0`, então `is_key_down(e_key_codes::Unknown)` fica `true` enquanto qualquer uma delas estiver segurada. O Lua não consegue consultá-la, já que `Unknown` não tem nome.
- O editor entrega todo evento de input ao `input_state_t`, incluindo eventos que o ImGui também trata. Digitar num campo de texto do editor enquanto a cena roda também é visto pelos scripts.
- O `mouse_position` vem das coordenadas de mouse do SDL, enquanto o `window_t::get_size()` retorna pixels. Numa tela com escala os dois podem usar unidades diferentes.
- O `create_window()` não consegue posicionar uma janela exatamente em `(0, 0)`: essa posição é tratada como "deixe o SDL decidir".
- O `window_t::set_title()` muda só o título no sistema; `descriptor.title` mantém o antigo.
- `platform_t`, `window_manager_t` e `window_t` não têm destrutores e suas operações de cópia não são deletadas. As janelas só são liberadas por `destroy_window()` ou `dispose()`, e uma cópia do `window_manager_t` teria os mesmos ponteiros de janela.
