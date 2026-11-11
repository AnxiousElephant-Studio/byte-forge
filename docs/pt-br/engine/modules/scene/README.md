# Módulo de cena

> Idioma: [English](../../../../en/engine/modules/scene/README.md) | **Português (Brasil)**

O módulo de cena guarda as entidades de um mundo de jogo e as executa. Ele tem três classes. O `scene_t` é dono de um registry ECS com os componentes embutidos da engine e desenha a cena. O `scene_player_t` executa a cena em modo play: move entidades, verifica colisões, toca fontes de áudio e conduz os scripts, com pausa, passo único e escala de tempo. O `scene_script_system_t` carrega um script Lua por entidade, chama os callbacks dele e expõe a API em Lua. Tudo fica no namespace `byteforge::scene`. O módulo não é header-only.

## Arquivos

| Arquivo                                                                  | Conteúdo                                                  |
| ------------------------------------------------------------------------ | --------------------------------------------------------- |
| `engine/include/byte-forge-engine/modules/scene/scene.h`                 | Declaração do `scene_t`                                   |
| `engine/include/byte-forge-engine/modules/scene/scene_player.h`          | Declaração do `scene_player_t`                            |
| `engine/include/byte-forge-engine/modules/scene/scene_script_system.h`   | Declaração do `scene_script_system_t`                     |
| `engine/source/modules/scene/scene.cpp`                                  | Registro de componentes e sistemas, padrões, renderização |
| `engine/source/modules/scene/scene_player.cpp`                           | Play, pausa, passo, passadas de física e de colisão       |
| `engine/source/modules/scene/scene_script_system.cpp`                    | Carga de scripts, callbacks e todas as funções da API Lua |

Peças relacionadas em outros módulos:

| Arquivo                                                                   | Papel                                                     |
| ------------------------------------------------------------------------- | --------------------------------------------------------- |
| `modules/ecs/registry.h`                                                  | Armazena entidades, componentes e sistemas                |
| `modules/components/*.h`                                                  | As structs de dados simples que a cena registra           |
| `modules/physics/physics_2D.h`                                            | Movimento e teste AABB usados pelo `scene_player_t`       |
| `modules/audio/audio_engine.h`                                            | Toca as fontes de áudio e as funções `audio` do Lua       |
| `modules/platform/input/input_state.h`                                    | Estado de teclado e mouse lido pelas funções `input` do Lua |
| `modules/rendering/sprite/sprite_renderer.h`                                     | Desenha sprites e labels para o `scene_t::render()`       |
| `applications/editor/source/editor.cpp`                                   | Dono da cena e do player, roda o loop de frames           |

## Conceitos

### Cena, registry e entidades

O `scene_t` é uma camada fina sobre um `ecs::registry_t`. O registry guarda as entidades, um array compacto por tipo de componente, e os sistemas. Uma entidade é um `ecs::entity_t`, que é só um id `u64`. Os ids começam em `0` e nunca são reutilizados enquanto o registry existir.

Toda entidade criada pelo `scene_t::create_entity()` recebe um nome e um transform 2D. O `scene_t::create_empty_entity()` dá só o nome. O transform é um componente normal: pode ser removido e adicionado de volta com `add_transform_2D()`. Os sistemas que precisam dele (sprite, label, câmera, física, colisão) simplesmente ignoram entidades sem transform. Os outros componentes são opcionais e são adicionados um a um com as funções `add_*`.

### Componentes e sistemas

O `scene_t::initialize()` registra nove tipos de componente e cria sete sistemas. Um sistema aqui é só uma lista das entidades que têm um certo conjunto de componentes. A cena não associa funções de update a eles; o código que precisa de uma lista lê `system->entities` diretamente.

| Sistema        | Componentes exigidos               | Lido por                                         |
| -------------- | ---------------------------------- | ------------------------------------------------ |
| `sprite_2D`    | `transform_2D` + `sprite_2D`       | `scene_t::render()`                              |
| `script`       | `script`                           | `scene_script_system_t`                          |
| `physics_2D`   | `transform_2D` + `velocity_2D`     | Passada de física do `scene_player_t`            |
| `collision_2D` | `transform_2D` + `box_collider_2D` | Passada de colisão do `scene_player_t`           |
| `audio`        | `audio_source`                     | `scene_player_t::start()`                        |
| `label_2D`     | `transform_2D` + `label_2D`        | `scene_t::render()`                              |
| `camera_2D`    | `transform_2D` + `camera_2D`       | `scene_t::render()`                              |
| `light_2D`     | `transform_2D` + `point_light_2D`  | Stage Render: desenha o mapa de luz (veja components) |

O componente `name` não tem sistema. As listas ficam atualizadas quando componentes são adicionados ou removidos pelo `scene_t`, porque essas funções chamam `registry.refresh_entity_systems()`.

### Coordenadas

A posição de um transform é o centro da entidade. Sprites são desenhados como um quad centrado nela, e o teste de colisão trata o collider como uma caixa centrada na posição mais o offset.

Sem câmera, uma unidade é um pixel, a origem é o canto superior esquerdo do viewport e o y cresce para baixo. Com uma câmera primária, a posição da câmera vira o centro do viewport e o zoom escala a área visível. A rotação é em graus.

### Modo de edição e modo play

A cena existe o tempo todo. Enquanto o player está parado, o editor edita os dados dos componentes diretamente e nada se move. O `scene_player_t::start()` muda para o modo play: toca as fontes de áudio, habilita o input e carrega os scripts. O `update()` então avança física, colisões e scripts a cada frame até o `stop()`.

O `scene_player_t` em si não copia a cena. O editor copia: o Play salva o level como texto antes do `start()` e o Stop restaura esse texto, então o que a física e os scripts mudam (posições, entidades criadas ou destruídas) é desfeito. O player standalone nunca restaura.

### Ambientes de script

Cada entidade com script o executa na sua própria tabela Lua, chamada de ambiente. O ambiente tem uma metatable cujo `__index` é `_G`, então o script consegue ler a biblioteca padrão e as tabelas da API da engine, mas toda global que ele atribui vai para o seu próprio ambiente. Duas entidades usando o mesmo arquivo têm, portanto, cópias separadas de toda variável e função. Os ambientes ficam guardados no registry do Lua e são referenciados pelo id da entidade.

## `scene_t`

```cpp
class BF_SCENE_API scene_t
{
public:
    ecs::registry_t registry;

    ecs::component_type_t* name_type;
    ecs::component_type_t* transform_2D_type;
    ecs::component_type_t* sprite_2D_type;
    ecs::component_type_t* script_type;
    ecs::component_type_t* velocity_2D_type;
    ecs::component_type_t* box_collider_2D_type;
    ecs::component_type_t* audio_source_type;
    ecs::component_type_t* label_2D_type;
    ecs::component_type_t* camera_2D_type;

    ecs::system_t* sprite_2D_system;
    ecs::system_t* script_system;
    ecs::system_t* physics_2D_system;
    ecs::system_t* collision_2D_system;
    ecs::system_t* audio_system;
    ecs::system_t* label_2D_system;
    ecs::system_t* camera_2D_system;

    scene_t();

    bool initialize();
    void dispose();

    ecs::entity_t create_entity(const char* name);
    ecs::entity_t create_empty_entity(const char* name);
    void destroy_entity(const ecs::entity_t entity);

    components::name_component_t* get_name(const ecs::entity_t entity);
    // ... one get_* per component type

    void add_transform_2D(const ecs::entity_t entity);
    void add_sprite_2D(const ecs::entity_t entity);
    // ... one add_* per optional component type
    void remove_component(ecs::component_type_t& component_type, const ecs::entity_t entity);

    void render(rendering::sprite_renderer_t& sprite_renderer, rendering::texture_cache_t& texture_cache, rendering::font_cache_t& font_cache, const math::vec2i_t& viewport_size);
};
```

O `scene_t` não pode ser copiado, porque o `registry_t` deleta as operações de cópia. Não há destrutor; o dono precisa chamar `dispose()`.

### Estado

| Campo          | Significado                                                                 |
| -------------- | --------------------------------------------------------------------------- |
| `registry`     | O registry ECS com toda entidade, componente e sistema da cena              |
| `*_type`       | Um ponteiro por tipo de componente registrado. `nullptr` antes do `initialize()` e depois do `dispose()` |
| `*_system`     | Um ponteiro por sistema, como listado em Conceitos. Mesmo ciclo de vida dos tipos |

Todos os campos são públicos. O editor lê o registry diretamente no painel de status e no observador do ECS, e passa os ponteiros `*_type` para o `remove_component()`.

### `initialize()`

1. Registra os nove tipos de componente com `registry.register_component_type()`, usando o tamanho de cada struct de componente e os nomes `name`, `transform_2D`, `sprite_2D`, `script`, `velocity_2D`, `box_collider_2D`, `audio_source`, `label_2D` e `camera_2D`. Nenhuma função de liberação é passada, então os componentes são bytes simples.
2. Retorna `false` se algum registro retornou `nullptr`. O próprio registry registra esse erro no log quando o limite de 64 tipos de componente é atingido.
3. Monta o bitset exigido por cada sistema a partir dos ids dos tipos (`1 << id`) e cria os sete sistemas.
4. Retorna `true`.

### `dispose()`

Descarta o registry, o que libera todo tipo de componente e sistema e limpa todas as entidades, e depois volta todo ponteiro de tipo e de sistema para `nullptr`. O contador de ids do registry volta para `0`.

### `create_empty_entity(name)` e `create_entity(name)`

`create_empty_entity(name)`:

1. Cria uma entidade nova no registry.
2. Monta um componente de nome copiando `name` com `snprintf`, então ele é cortado em 63 caracteres mais o terminador.
3. Adiciona o componente e chama `registry.spawn_entity()`, que coloca a entidade na hora em todo sistema que a aceita. Ela não passa pela fila de spawn do registry.
4. Retorna a entidade.

`create_entity(name)` chama `create_empty_entity(name)` e depois `add_transform_2D()`. Os arquivos de level usam `create_empty_entity()` e só adicionam o transform quando a entidade salva tem um.

### `destroy_entity(entity)`

Chama `registry.kill_entity()` na hora: a entidade é removida de todo sistema, todo componente que ela tinha é removido, o bitset de componentes dela é zerado e ela é apagada da lista de entidades.

### Getters

| Membro                       | Retorna                                                     |
| ---------------------------- | ----------------------------------------------------------- |
| `get_name(entity)`           | `name_component_t*`                                         |
| `get_transform_2D(entity)`   | `transform_2D_component_t*`                                 |
| `get_sprite_2D(entity)`      | `sprite_2D_component_t*`                                    |
| `get_script(entity)`         | `script_component_t*`                                       |
| `get_velocity_2D(entity)`    | `velocity_2D_component_t*`                                  |
| `get_box_collider_2D(entity)`| `box_collider_2D_component_t*`                              |
| `get_audio_source(entity)`   | `audio_source_component_t*`                                 |
| `get_label_2D(entity)`       | `label_2D_component_t*`                                     |
| `get_camera_2D(entity)`      | `camera_2D_component_t*`                                    |

Cada um pede ao tipo de componente os dados da entidade e faz o cast. O resultado é `nullptr` quando a entidade não tem aquele componente. O ponteiro aponta para dentro do array compacto do tipo, então só continua válido até um componente do mesmo tipo ser adicionado ou removido.

### `add_*(entity)`

Cada `add_*` monta um componente zerado, preenche os padrões dele e passa para o `add_component()` privado, que o adiciona no registry e chama `refresh_entity_systems()` para a entidade entrar nos sistemas que agora a aceitam.

| Membro                        | Padrões                                                      |
| ----------------------------- | ------------------------------------------------------------ |
| `add_sprite_2D(entity)`       | Sem textura, tamanho `(100, 100)`, tint branco `(1, 1, 1, 1)` |
| `add_script(entity)`          | Caminho de script vazio                                      |
| `add_velocity_2D(entity)`     | Velocidade `(0, 0)`, speed `100`                             |
| `add_box_collider_2D(entity)` | Offset `(0, 0)`, tamanho `(100, 100)`                        |
| `add_audio_source(entity)`    | Sem clip, volume `1.0`, sem loop, sem play on start          |
| `add_label_2D(entity)`        | Texto `Label`, sem fonte, tamanho de fonte `32`, cor branca  |
| `add_camera_2D(entity)`       | Zoom `1.0`, `is_primary` ligado (`true`)                     |

Se a entidade já tem aquele componente, o registry o substitui pelos padrões. Não existe `add_*` para o nome, já que toda entidade recebe um. O `add_transform_2D()` adiciona um transform com posição `(0, 0)`, rotação `0` e escala `(1, 1)`.

### `remove_component(component_type, entity)`

Remove o componente do registry e chama `refresh_entity_systems()`, para a entidade sair de todo sistema que não a aceita mais. Funciona para qualquer tipo, inclusive nome e transform; o editor só não oferece a remoção do transform.

### `render(sprite_renderer, texture_cache, font_cache, viewport_size)`

1. Procura uma câmera. Percorre o sistema `camera_2D` em ordem e pega a primeira entidade cujo `is_primary` é `true`, lendo a posição do transform e o zoom dela. Se nenhuma é encontrada, nenhuma câmera é usada.
2. Chama `sprite_renderer.begin()` com o tamanho do viewport, a posição da câmera, o zoom e se uma câmera foi encontrada.
3. Desenha toda entidade do sistema `sprite_2D`. O tamanho desenhado é o tamanho do sprite multiplicado pela escala do transform, e a textura vem de `texture_cache.get(texture_path)`. Um caminho vazio não dá textura, e o quad é desenhado como uma cor lisa com o tint.
4. Desenha toda entidade do sistema `label_2D` com `sprite_renderer.draw_text()`, na posição do transform com a cor do label. A fonte vem de `font_cache.get(font_path, font_size)`. Quando ele retorna `nullptr` (sem caminho de fonte, tamanho não positivo ou fonte que falhou ao carregar), o label é pulado.

Os labels são sempre desenhados depois de todos os sprites, então o texto aparece por cima. Dentro de cada grupo, a ordem é a ordem das entidades no sistema.

## `scene_player_t`

```cpp
class BF_SCENE_API scene_player_t
{
public:
    scene_script_system_t script_system;
    time::profiler_t* profiler;
    f32 time_scale;

    bool initialize(lua_State* lua_state, scene_t& scene, audio::audio_engine_t& audio_engine, platform::input_state_t& input_state);
    void start();
    void update(const f64 delta_time);
    void stop();
    void set_paused(const bool value);
    void request_step();
    bool is_running() const;
    bool is_paused() const;
    void dispose();

private:
    void update_physics(const f32 delta_time);
    void update_collisions();

    scene_t* scene;
    audio::audio_engine_t* audio_engine;
    platform::input_state_t* input_state;
    bool running;
    bool paused;
    bool step_requested;
};
```

### Estado

| Campo            | Significado                                                             |
| ---------------- | ----------------------------------------------------------------------- |
| `script_system`  | O sistema de scripts que ele conduz. Público para o editor ler a contagem de ambientes |
| `profiler`       | Profiler opcional para as seções `physics`, `collisions` e `scripts`. `nullptr` por padrão |
| `time_scale`     | Multiplicador aplicado a todo delta time. `1.0` por padrão              |
| `scene`, `audio_engine`, `input_state` | Os objetos passados para o `initialize()`. Não são dele   |
| `running`        | `true` entre o `start()` e o `stop()`                                   |
| `paused`         | `true` enquanto pausado. Só tem sentido enquanto está rodando           |
| `step_requested` | `true` quando um passo fixo deve rodar no próximo `update()`            |

### Estados

```
stopped ──start()──► running ──set_paused(true)──► paused
   ▲                    │  ◄──set_paused(false)──     │
   └──────stop()────────┴──────────stop()─────────────┘
```

Enquanto pausado, o `request_step()` deixa exatamente um frame avançar no próximo `update()`.

### `initialize(lua_state, scene, audio_engine, input_state)`

Guarda a cena, a engine de áudio e o estado de input, e retorna o resultado do `script_system.initialize()` com os mesmos argumentos. Não inicia nada.

### `start()`

1. Chama `stop()`, então iniciar enquanto já está rodando recomeça de um estado limpo.
2. Percorre o sistema `audio` e chama `audio_engine->play()` para toda fonte de áudio com `play_on_start` ligado. Os handles retornados não são guardados.
3. Limpa o estado de input e liga `input_state->enabled` (`true`), para os eventos de input voltarem a ser registrados.
4. Chama `script_system.start()`, que carrega todos os scripts e roda o `on_start`.
5. Liga `running` (`true`).

### `update(delta_time)`

1. Retorna se não está rodando.
2. Lê `step_requested` para uma variável local e o limpa.
3. Retorna se está pausado e nenhum passo foi pedido.
4. Escolhe o delta time: o fixo de `1/60` de segundo ao dar um passo, `delta_time` nos outros casos. Depois multiplica por `time_scale`.
5. Roda a passada de física com esse delta time.
6. Roda a passada de colisão.
7. Chama `script_system.update()` com o mesmo delta time.

A passada de física percorre o sistema `physics_2D` e define a posição de todo transform como `physics_2D_t::move(position, velocity, speed, delta_time)`.

A passada de colisão percorre todo par de entidades do sistema `collision_2D`, cada par uma vez. Para cada par ela chama `physics_2D_t::check_aabb_collision()` com a posição do transform mais o offset do collider e o tamanho do collider. Quando há sobreposição, chama `script_system.notify_collision()` duas vezes: uma com a primeira entidade e a outra como argumento, depois ao contrário. Nada é separado; a passada só informa.

Cada passada roda dentro de um `time::scoped_profile_t`, que não faz nada quando `profiler` é `nullptr`.

### `stop()`

1. Chama `script_system.stop()`, que descarta todos os ambientes de script.
2. Chama `audio_engine->stop_all()`, que para todos os sons da engine, inclusive os iniciados pelo Lua.
3. Desliga `input_state->enabled` (`false`) e limpa o estado de input.
4. Desliga `running`, `paused` e `step_requested` (`false`).

### `set_paused(value)` e `request_step()`

O `set_paused()` guarda o valor como está, esteja o player rodando ou não. O `request_step()` só liga `step_requested` quando o player está rodando e pausado; nos outros casos não faz nada.

### `is_running()` e `is_paused()`

Retornam `running` e `paused`. O editor os usa para escolher os itens do menu e o rótulo do painel de status.

### `dispose()`

Chama `stop()` se o `initialize()` foi chamado (o ponteiro da cena está definido), depois descarta o sistema de scripts e limpa os três ponteiros. Precisa rodar enquanto o estado Lua e a engine de áudio ainda estão vivos.

## `scene_script_system_t`

```cpp
class BF_SCENE_API scene_script_system_t
{
public:
    bool initialize(lua_State* lua_state, scene_t& scene, audio::audio_engine_t& audio_engine, platform::input_state_t& input_state);
    void start();
    void update(const f64 delta_time);
    void notify_collision(const ecs::entity_t entity, const ecs::entity_t other);
    void stop();
    bool is_running() const;
    usize get_environment_count() const;
    void dispose();

private:
    bool load_script(const ecs::entity_t entity, const char* script_path);
    bool push_function(const ecs::entity_t entity, const char* function_name);
    void invoke(const ecs::entity_t entity, const char* function_name, const i32 argument_count);
    void register_api();

    lua_State* lua_state;
    scene_t* scene;
    audio::audio_engine_t* audio_engine;
    platform::input_state_t* input_state;
    bool running;
    std::unordered_map<u64, i32> entity_environments;
};
```

### Estado

| Campo                 | Significado                                                          |
| --------------------- | -------------------------------------------------------------------- |
| `lua_state`           | O estado Lua, que pertence ao módulo de scripting do runtime         |
| `scene`, `audio_engine`, `input_state` | Os objetos sobre os quais a API Lua trabalha. Não são dele |
| `running`             | `true` entre o `start()` e o `stop()`                                |
| `entity_environments` | Id da entidade para a referência, no registry do Lua, do ambiente dela |

### Callbacks

Um script define qualquer uma destas funções. As que faltam são puladas sem mensagem.

| Callback                       | Quando                                                          | Argumentos                                  |
| ------------------------------ | --------------------------------------------------------------- | ------------------------------------------- |
| `on_start(entity)`             | Uma vez, logo depois que o script da entidade é carregado no `start()` | O id da entidade                     |
| `on_update(entity, delta_time)`| Todo frame que o `scene_player_t::update()` avança              | O id da entidade, o delta escalado em segundos |
| `on_collision(entity, other)`  | Todo frame que avança em que os dois colliders se sobrepõem     | O id da entidade, o id da outra entidade    |

Dentro de um frame a ordem é: física, depois todos os `on_collision`, depois todos os `on_update`. Os ids das entidades são passados como inteiros Lua.

### `initialize(lua_state, scene, audio_engine, input_state)`

Guarda os quatro ponteiros e chama `register_api()`, que cria as tabelas globais da API. Sempre retorna `true`.

### `start()`

1. Chama `stop()` para descartar ambientes anteriores.
2. Percorre o sistema `script`. Para cada entidade cujo caminho de script não é vazio, carrega o script e, se a carga deu certo e o script define `on_start`, chama `on_start(entity)`. Cada script é carregado e iniciado antes do próximo ser carregado.
3. Liga `running` (`true`).

### `update(delta_time)`

Retorna se não está rodando. Senão percorre o sistema `script` e chama `on_update(entity, delta_time)` para toda entidade que tem ambiente e define a função.

### `notify_collision(entity, other)`

Se está rodando e a entidade tem um ambiente com `on_collision`, chama `on_collision(entity, other)`.

### `stop()`

Libera toda referência de ambiente com `luaL_unref`, limpa o mapa e desliga `running` (`false`). Sem nenhuma referência restante, as tabelas de ambiente podem ser recolhidas pelo garbage collector do Lua.

### `get_environment_count()` e `is_running()`

O `get_environment_count()` retorna o número de ambientes carregados, que o painel de status do editor mostra. Ele volta para `0` depois do `stop()`. O `is_running()` retorna `running`.

### `dispose()`

Chama `stop()` se um estado Lua foi definido, depois limpa os quatro ponteiros.

### `load_script(entity, script_path)`

1. Compila o arquivo com `luaL_loadfile`. Um caminho relativo é relativo ao diretório de trabalho do processo. Se falha, registra `Could not load script '<path>' for entity <id>: <message>` e retorna `false`.
2. Cria a tabela de ambiente e uma metatable com `__index` apontando para `_G`.
3. Guarda uma referência ao ambiente no registry do Lua.
4. Define o ambiente como o primeiro upvalue (`_ENV`) do chunk compilado.
5. Roda o chunk, que define as funções e variáveis do script dentro do ambiente. Se ele gera um erro, registra o mesmo formato de mensagem, libera a referência e retorna `false`.
6. Salva a referência sob o id da entidade e retorna `true`.

### `push_function(entity, function_name)` e `invoke(entity, function_name, argument_count)`

O `push_function()` procura o ambiente da entidade, empilha ele e depois empilha o campo `function_name`. Se não há ambiente ou o campo não é uma função, desempilha o que empilhou e retorna `false`. Em caso de sucesso o ambiente e a função ficam na pilha e quem chamou empilha os argumentos.

O `invoke()` roda a função com `lua_pcall`. Em caso de erro registra `Script function '<name>' failed for entity <id>: <message>` e desempilha a mensagem. Nos dois casos depois desempilha o ambiente. Um erro não remove o ambiente, então o callback é tentado de novo no próximo frame.

### `register_api()`

Cria seis tabelas globais: `transform`, `velocity`, `label`, `audio`, `log` e `input`. Toda função nelas recebe três upvalues de light userdata: a cena (`1`), a engine de áudio (`2`) e o estado de input (`3`). As funções C os leem de volta com `lua_upvalueindex`.

### API em Lua

As funções que recebem uma entidade a leem do argumento 1 como inteiro. Se a entidade não tem o componente necessário, elas geram o erro Lua `entity <id> has no <component>`, onde `<component>` é `transform_2D`, `velocity_2D`, `label_2D` ou `audio_source`. Tipos de argumento errados geram o erro padrão `bad argument` do Lua. Dentro de um callback, qualquer um desses erros interrompe o callback e é registrado pelo `invoke()`.

Tabela `transform`:

| Função                                | Retorna    | Descrição                                         |
| ------------------------------------- | ---------- | ------------------------------------------------- |
| `transform.get_position(entity)`      | `x, y`     | Posição do transform                              |
| `transform.set_position(entity, x, y)`| nada       | Define a posição do transform                     |
| `transform.get_rotation(entity)`      | `degrees`  | Rotação do transform                              |
| `transform.set_rotation(entity, deg)` | nada       | Define a rotação do transform                     |
| `transform.get_scale(entity)`         | `x, y`     | Escala do transform                               |
| `transform.set_scale(entity, x, y)`   | nada       | Define a escala do transform                      |

Tabela `velocity`:

| Função                                | Retorna    | Descrição                                         |
| ------------------------------------- | ---------- | ------------------------------------------------- |
| `velocity.get(entity)`                | `x, y`     | Vetor de velocidade                               |
| `velocity.set(entity, x, y)`          | nada       | Define o vetor de velocidade                      |
| `velocity.get_speed(entity)`          | `speed`    | Multiplicador de speed                            |
| `velocity.set_speed(entity, speed)`   | nada       | Define o multiplicador de speed                   |

Tabela `label`:

| Função                                | Retorna    | Descrição                                         |
| ------------------------------------- | ---------- | ------------------------------------------------- |
| `label.get_text(entity)`              | `string`   | Texto do label                                    |
| `label.set_text(entity, text)`        | nada       | Copia `text`, cortado em 255 caracteres           |

Tabela `audio`, descrita em detalhe no [módulo de áudio](../audio/README.md#api-em-lua):

| Função                                | Retorna    | Descrição                                         |
| ------------------------------------- | ---------- | ------------------------------------------------- |
| `audio.play(entity)`                  | `handle`   | Toca a fonte de áudio da entidade, `0` se falhar  |
| `audio.stop(handle)`                  | nada       | Para um som                                       |

Tabela `input`:

| Função                                | Retorna    | Descrição                                         |
| ------------------------------------- | ---------- | ------------------------------------------------- |
| `input.is_key_down(name)`             | `boolean`  | A tecla está pressionada                          |
| `input.is_key_pressed(name)`          | `boolean`  | A tecla desceu neste frame                        |
| `input.is_key_released(name)`         | `boolean`  | A tecla subiu neste frame                         |
| `input.is_mouse_down(name)`           | `boolean`  | O botão do mouse está pressionado                 |
| `input.is_mouse_pressed(name)`        | `boolean`  | O botão do mouse desceu neste frame               |
| `input.is_mouse_released(name)`       | `boolean`  | O botão do mouse subiu neste frame                |
| `input.get_mouse_position()`          | `x, y`     | Última posição informada por um evento de movimento do mouse |
| `input.get_mouse_wheel()`             | `x, y`     | Rolagem da roda acumulada neste frame             |

Os nomes de teclas e de botões do mouse são comparados exatamente, incluindo maiúsculas, com as tabelas do `platform::input_names_t` (por exemplo `"A"`, `"Space"`, `"ArrowLeft"`, `"Left"`, `"XButton1"`). Um nome desconhecido gera `unknown key name '<name>'` ou `unknown mouse button name '<name>'`.

Tabela `log`:

| Função                                | Retorna    | Descrição                                         |
| ------------------------------------- | ---------- | ------------------------------------------------- |
| `log.info(message)`                   | nada       | Registra `[lua] <message>` como info              |
| `log.warning(message)`                | nada       | Registra `[lua] <message>` como warning           |
| `log.error(message)`                  | nada       | Registra `[lua] <message>` como error             |

O `message` precisa ser uma string ou um número; um número é convertido pelo `luaL_checkstring`.

`transform.get_world_position(entity)` retorna a posição depois de aplicar todos os transforms dos pais (veja [Hierarquia](#hierarquia)).

Tabela `sprite`:

| Função                                    | Retorna       | Descrição                                   |
| ----------------------------------------- | ------------- | ------------------------------------------- |
| `sprite.get_texture(entity)`              | `path`        | Caminho da textura                          |
| `sprite.set_texture(entity, path)`        | nada          | Define o caminho da textura                 |
| `sprite.get_tint(entity)`                 | `r, g, b, a`  | Tint                                        |
| `sprite.set_tint(entity, r, g, b [, a])`  | nada          | Define o tint, alpha padrão `1`             |
| `sprite.get_size(entity)`                 | `x, y`        | Tamanho antes da escala do transform        |
| `sprite.set_size(entity, x, y)`           | nada          | Define o tamanho                            |

Tabela `camera`: `camera.get_zoom(entity)` e `camera.set_zoom(entity, zoom)`.

Tabela `time`: `time.get_elapsed()` retorna os segundos jogados desde o `start()` (com a escala de tempo), `time.get_delta()` o delta do update atual.

Tabela `scene`:

| Função                                      | Retorna           | Descrição                                                        |
| ------------------------------------------- | ----------------- | ---------------------------------------------------------------- |
| `scene.create_entity([name])`               | `entity`          | Entidade nova com nome e transform                               |
| `scene.destroy(entity)`                     | nada              | Destrói a entidade e os filhos depois do update atual            |
| `scene.exists(entity)`                      | `boolean`         | A entidade não foi destruída                                     |
| `scene.find(name)`                          | `entity` ou nil   | Primeira entidade com esse nome exato                            |
| `scene.find_with_tag(tag)`                  | `{ entity... }`   | Todas as entidades cujas tags incluem `tag`                      |
| `scene.has_tag(entity, tag)`                | `boolean`         | A entidade tem a tag                                             |
| `scene.get_name(entity)`                    | `string`          | Nome da entidade                                                 |
| `scene.get_parent(entity)`                  | `entity` ou nil   | Entidade pai                                                     |
| `scene.set_parent(entity, parent ou nil)`   | `boolean`         | Define ou remove o pai, mantendo o transform de mundo            |
| `scene.get_children(entity)`                | `{ entity... }`   | Filhos diretos                                                   |
| `scene.spawn_stamp(path [, x, y])`         | `entity` ou nil   | Instancia um `.bfstamp` e retorna a raiz                        |
| `scene.load_level(path)`                    | nada              | Pede à aplicação para carregar outro level depois deste frame    |

Entidades criadas ou instanciadas durante o play têm o script iniciado no próximo update (ou na hora, durante o `start()`). O `destroy` é adiado: a entidade sai depois do stage atual, então loops sobre entidades nunca ficam inválidos e `scene.exists` passa a `false` no próximo frame. `spawn_stamp` e `load_level` precisam que a aplicação defina `script_system.stamp_loader` e trate `take_level_request()`; o editor e o player fazem as duas coisas.

### Hierarquia

Uma entidade pode ter um pai pelo `parent_component_t`. O `transform_2D` dela passa a ser **local**: relativo à posição, rotação e escala do pai. `scene_t::get_world_transform_2D(entity)` compõe a cadeia: a posição local é escalada pela escala do pai, girada pela rotação do pai e somada à posição do pai; rotações se somam e escalas se multiplicam. Um pai sem transform conta como identidade.

Os sistemas de render, luz, câmera e colisão usam transforms de mundo. A física move a posição local, então um filho em movimento continua seguindo o pai. `set_parent` mantém o filho no mesmo lugar na tela recalculando o transform local; `attach_to_parent` mantém os valores locais (usado ao carregar levels e stamps). Destruir uma entidade destrói a subárvore inteira.

### Tags

`tags_component_t` guarda tags separadas por vírgula (`"enemy, flying"`), até 127 caracteres. `has_tag` ignora espaços em volta de cada tag e compara exatamente. `scene_t::find_with_tag` e `find_by_name` percorrem todas as entidades.

## Uso

O editor (`editor_t`) tem um `scene_t` e um `scene_player_t` como membros. No `initialize()` ele chama `scene.initialize()` e depois `scene_player.initialize()` com o estado Lua do runtime, `runtime.audio` e o seu próprio `input_state`, e define `scene_player.profiler`. No `dispose()` ele descarta o player primeiro, depois a cena, e o runtime por último.

A cada frame:

```
input_state.begin_frame()
poll events                         -- fills input_state while enabled
scene_player.update(delta_time)     -- physics, collisions, scripts
runtime.audio.update()
scene.render(...)                   -- sprites, then labels
draw UI
```

A barra de menu principal mostra `Play` enquanto parado, que chama `start()`. Enquanto roda ela mostra `Stop`, `Pause` ou `Resume` (`set_paused(!is_paused())`), e `Step` enquanto pausado (`request_step()`). O painel de status oferece os mesmos botões de pausa e passo e um slider de escala de tempo de `0` a `4` que escreve direto em `time_scale`. A hierarquia da cena chama `create_entity("Entity")` (Create Entity), `create_empty_entity("Entity")` (Create Empty) e `destroy_entity()`, e o inspetor de objetos chama as funções `add_*` e o `remove_component()`.

Um script de `applications/editor/assets/scripts/rotate.lua`:

```lua
local speed = 5

function on_start(entity)
    log.info("rotate.lua started for entity " .. entity)
end

function on_update(entity, delta_time)
    local rotation = transform.get_rotation(entity)
    transform.set_rotation(entity, rotation + speed * delta_time)
end
```

O `speed` é local ao chunk e o `on_start` e o `on_update` são globais do ambiente da entidade, então toda entidade que usa esse arquivo tem as suas próprias cópias.

## Notas

- Os scripts são lidos do disco a cada `start()`. Mudanças num arquivo valem na próxima vez que o Play é apertado.
- Os scripts só são carregados no `start()`. Uma entidade que ganha um componente de script, ou é criada, enquanto a cena está rodando não tem ambiente e não recebe callbacks até o próximo `start()`.
- Um script cujo código de topo falha ao carregar ou rodar não ganha ambiente nenhum. Um erro dentro de um callback é registrado e o callback roda de novo no próximo frame, então um erro no `on_update` é registrado a cada frame.
- O `on_collision` dispara a cada frame que avança enquanto dois colliders se sobrepõem. Não há evento separado de entrada ou de saída, e caixas que só se tocam numa borda contam como sobrepostas.
- A caixa de colisão usa só o offset e o tamanho do collider. A escala e a rotação do transform não são aplicadas, enquanto os sprites são desenhados com escala e rotação.
- Ao dar um passo, o `1/60` de segundo fixo também é multiplicado por `time_scale`. Com `time_scale` em `0`, os frames continuam rodando: scripts e colisões são chamados com delta `0` e nada se move.
- O `add_camera_2D()` marca toda câmera nova como primária. Com várias câmeras primárias, o `render()` usa a primeira do sistema `camera_2D`.
- O erro de componente ausente formata o id da entidade com `%d` depois de convertê-lo para `i32`.
- O `register_api()` define as globais `transform`, `velocity`, `label`, `audio`, `log` e `input` no estado Lua compartilhado, substituindo qualquer valor que elas tinham. Um script que atribui um desses nomes só o encobre no seu próprio ambiente.
