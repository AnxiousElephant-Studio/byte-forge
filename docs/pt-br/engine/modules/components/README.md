# Módulo de componentes

> Idioma: [English](../../../../en/engine/modules/components/README.md) | **Português (Brasil)**

O módulo de componentes guarda as structs de dados que as entidades carregam: um nome, um transform 2D, um sprite, um label, uma velocidade, um box collider, uma câmera, uma fonte de áudio e um script. Cada uma é uma struct simples sem funções. O módulo é só header, fica no namespace `byteforge::components` e depende apenas dos tipos básicos da engine e dos vetores de math.

As structs não fazem nada sozinhas. A cena registra cada uma como um tipo de componente do ECS, dá os valores padrão aos componentes novos, e o scene player, o renderer, a API Lua e o editor leem e escrevem os campos.

## Arquivos

| Arquivo                                                                      | Conteúdo                                         |
| ---------------------------------------------------------------------------- | ------------------------------------------------ |
| `engine/include/byte-forge-engine/modules/components/name_component.h`       | `name_component_t`, `NAME_MAX_LENGTH`            |
| `engine/include/byte-forge-engine/modules/components/transform_2D_component.h` | `transform_2D_component_t`                     |
| `engine/include/byte-forge-engine/modules/components/sprite_2D_component.h`  | `sprite_2D_component_t`, `TEXTURE_PATH_MAX_LENGTH` |
| `engine/include/byte-forge-engine/modules/components/label_2D_component.h`   | `label_2D_component_t`, `LABEL_TEXT_MAX_LENGTH`, `FONT_PATH_MAX_LENGTH` |
| `engine/include/byte-forge-engine/modules/components/velocity_2D_component.h` | `velocity_2D_component_t`                       |
| `engine/include/byte-forge-engine/modules/components/box_collider_2D_component.h` | `box_collider_2D_component_t`               |
| `engine/include/byte-forge-engine/modules/components/camera_2D_component.h`  | `camera_2D_component_t`                          |
| `engine/include/byte-forge-engine/modules/components/point_light_2D_component.h` | `point_light_2D_component_t`                |
| `engine/include/byte-forge-engine/modules/components/audio_source_component.h` | `audio_source_component_t`, `AUDIO_CLIP_PATH_MAX_LENGTH` |
| `engine/include/byte-forge-engine/modules/components/script_component.h`     | `script_component_t`, `SCRIPT_PATH_MAX_LENGTH`   |

Peças relacionadas em outros módulos:

| Arquivo                                                  | Papel                                                           |
| -------------------------------------------------------- | --------------------------------------------------------------- |
| `modules/ecs/`                                           | Guarda os componentes como bytes crus                           |
| `modules/scene/scene.cpp`                                | Registra os tipos, define os padrões, desenha sprites e labels  |
| `modules/scene/scene_player.cpp`                         | Move entidades, verifica colisões, toca áudio no start          |
| `modules/scene/scene_script_system.cpp`                  | Roda os scripts e expõe transform, velocity, label e audio ao Lua |
| `applications/editor/.../object_inspector_component.cpp` | Edita os campos, adiciona e remove componentes                  |

## Conceitos

### Dados simples, copiados como bytes

O ECS guarda os componentes copiando os bytes com `std::memcpy`, e a cena registra todos os tipos deste módulo sem função de liberação. Todas as structs são feitas só de números, `bool`s, vetores de math e arrays de `char` de tamanho fixo, então uma cópia de bytes é uma cópia completa e não há nada para liberar.

Campos de texto são arrays de `char` com capacidade fixa em vez de `std::string`. Cada capacidade é uma constante nomeada ao lado da struct. A maior string que um campo comporta é a capacidade menos um, por causa do terminador. O código que escreve neles (`std::snprintf` na cena e no Lua, `ImGui::InputText` no editor) recebe a capacidade e corta textos mais longos.

### Os padrões vêm da cena

As structs não têm construtores nem valores padrão nos membros. Os padrões são definidos pelo `scene_t` quando ele adiciona um componente: começa de uma struct zerada (`= {}`) e atribui os campos que não devem ser zero. As tabelas abaixo listam esses padrões.

### Toda entidade tem um nome

Toda entidade tem um `name_component_t`. O `scene_t::create_entity()` também adiciona um `transform_2D_component_t`, e o `create_empty_entity()` não. Todo outro componente, incluindo o transform, é opcional e é adicionado um por vez, pelo menu Add Component do editor, através de `scene_t::add_transform_2D()`, `add_sprite_2D()` e as outras funções `add_*`. O editor não oferece botão Remove Component para o nome.

### Unidades e coordenadas

Posições e tamanhos são em unidades de mundo. Sem câmera, uma unidade é um pixel e a origem é o canto superior esquerdo do viewport. Com uma câmera primária, a posição da câmera é o centro do viewport e o zoom dela escala a área visível. A rotação é em graus. Cores são RGBA com cada canal de `0` a `1`.

## `name_component_t`

```cpp
constexpr usize NAME_MAX_LENGTH = 64;

struct name_component_t
{
    char name[NAME_MAX_LENGTH];
};
```

### Estado

| Campo  | Padrão                           | Significado                              |
| ------ | -------------------------------- | ---------------------------------------- |
| `name` | O nome passado ao `create_entity()` | Nome de exibição, até 63 caracteres   |

### Quem lê

O nome é só para pessoas. A scene hierarchy e o ECS Observer listam as entidades por ele, mostrando `<unnamed>` se a entidade não tem componente de nome. O object inspector mostra ele como um campo de texto no topo, fora de qualquer cabeçalho de componente. Nada procura entidades pelo nome, e duas entidades podem ter o mesmo.

## `transform_2D_component_t`

```cpp
struct transform_2D_component_t
{
    math::vec2_t position;
    math::vec2_t scale;
    f32          rotation;
};
```

### Estado

| Campo      | Padrão       | Significado                                     |
| ---------- | ------------ | ----------------------------------------------- |
| `position` | `(0, 0)`     | Onde a entidade está, em unidades de mundo      |
| `scale`    | `(1, 1)`     | Multiplica o tamanho do sprite                  |
| `rotation` | `0`          | Rotação em graus                                |

### Quem lê

Todos os sistemas, exceto `script` e `audio`, exigem ele. Como cada um usa:

| Leitor                    | Usa                                                           |
| ------------------------- | ------------------------------------------------------------- |
| Renderização de sprites   | `position` como centro do sprite, `rotation`, `scale` vezes o tamanho do sprite |
| Renderização de labels    | Só `position`                                                 |
| Física                    | Escreve `position` a cada update                              |
| Colisões                  | `position` mais o offset do collider                          |
| Câmera                    | `position` como centro da visão                               |

O Lua lê e escreve ele pela tabela `transform`: `get_position`, `set_position`, `get_rotation`, `set_rotation`, `get_scale` e `set_scale`. Vetores são passados como dois números, por exemplo `transform.set_position(entity, x, y)`.

## `sprite_2D_component_t`

```cpp
constexpr usize TEXTURE_PATH_MAX_LENGTH  = 256;
constexpr usize MATERIAL_PATH_MAX_LENGTH = 256;

struct sprite_2D_component_t
{
    char         texture_path[TEXTURE_PATH_MAX_LENGTH];
    math::vec2_t size;
    math::vec4_t tint;
    char         material_path[MATERIAL_PATH_MAX_LENGTH];
    bool         is_lit;
};
```

### Estado

| Campo          | Padrão         | Significado                                          |
| -------------- | -------------- | ---------------------------------------------------- |
| `texture_path` | vazio          | Arquivo de imagem a desenhar, até 255 caracteres     |
| `size`         | `(100, 100)`   | Tamanho em unidades de mundo antes da escala do transform |
| `tint`         | `(1, 1, 1, 1)` | Multiplica a textura, ou preenche o quad sem textura |
| `material_path` | vazio         | Arquivo de material (`surface()`); vazio usa o shader de sprite |
| `is_lit`       | `false`        | Recebe as point lights: usa o shading model `Lit`, com o material de sprite padrão quando `material_path` está vazio |

### Quem lê

O `scene_t::render()` desenha toda entidade do sistema `sprite_2D`, que exige um transform e um sprite. Para cada uma, multiplica `size` pela `scale` do transform componente a componente, e depois desenha um quad centrado na `position` do transform, girado pela `rotation` dele, com `tint`.

A textura vem de `texture_cache_t::get(texture_path)`. Com caminho vazio o cache retorna `nullptr`, e o shader de sprite então preenche o quad só com a cor de `tint`. Com textura, a cor da textura é multiplicada por `tint`.

Com um `material_path`, o sprite é desenhado com esse material (`sprite_renderer_t::draw_with_material()`), pego no `material_cache_t` com o shading model escolhido por `is_lit`. Se o material falha ao carregar, o sprite volta para o shader de sprite. Veja o [módulo de renderização](../rendering/README.md) para o que um material recebe.

Os sprites são desenhados na ordem da lista do sistema `sprite_2D`, que é a ordem em que os componentes foram adicionados. O Lua não tem acesso a sprites.

## `label_2D_component_t`

```cpp
constexpr usize LABEL_TEXT_MAX_LENGTH = 256;
constexpr usize FONT_PATH_MAX_LENGTH  = 256;

struct label_2D_component_t
{
    char         text[LABEL_TEXT_MAX_LENGTH];
    char         font_path[FONT_PATH_MAX_LENGTH];
    f32          font_size;
    math::vec4_t color;
};
```

### Estado

| Campo       | Padrão         | Significado                                     |
| ----------- | -------------- | ----------------------------------------------- |
| `text`      | `"Label"`      | Texto a desenhar, até 255 caracteres            |
| `font_path` | vazio          | Arquivo da fonte, até 255 caracteres            |
| `font_size` | `32`           | Tamanho em que a fonte é gerada                 |
| `color`     | `(1, 1, 1, 1)` | Cor do texto                                    |

### Quem lê

O `scene_t::render()` desenha toda entidade do sistema `label_2D` depois de todos os sprites. Ele pede uma fonte a `font_cache_t::get(font_path, font_size)` e pula o label se recebe `nullptr`, o que acontece quando o caminho está vazio, o tamanho é `0` ou menos, ou a fonte não carregou. Um label novo, com o caminho de fonte vazio padrão, portanto não é desenhado até uma fonte ser definida.

O texto começa na `position.x` do transform, com a linha de base `font_size` unidades abaixo de `position.y`. A escala e a rotação do transform não são aplicadas a labels.

O Lua lê e escreve o texto por `label.get_text(entity)` e `label.set_text(entity, text)`. O editor edita o texto, o caminho da fonte, o tamanho (um slider de `4` a `256`) e a cor.

## `velocity_2D_component_t`

```cpp
struct velocity_2D_component_t
{
    math::vec2_t velocity;
    f32          speed;
};
```

### Estado

| Campo      | Padrão   | Significado                                   |
| ---------- | -------- | --------------------------------------------- |
| `velocity` | `(0, 0)` | Direção e quantidade relativa de movimento    |
| `speed`    | `100`    | Multiplicador aplicado a `velocity`           |

### Quem lê

Enquanto a cena está tocando, o `scene_player_t` move toda entidade do sistema `physics_2D` (transform e velocity) a cada update:

```
position = position + velocity * speed * delta_time
```

O `delta_time` é o tempo do frame, ou 1/60 de segundo fixo para um único passo enquanto pausado, multiplicado pela escala de tempo do player. Com os padrões, uma entidade com `velocity` `(1, 0)` anda 100 unidades por segundo para a direita. Nada normaliza `velocity`, então o comprimento dele também escala o movimento.

O Lua lê e escreve os dois campos pela tabela `velocity`: `get`, `set`, `get_speed` e `set_speed`.

## `box_collider_2D_component_t`

```cpp
struct box_collider_2D_component_t
{
    math::vec2_t offset;
    math::vec2_t size;
};
```

### Estado

| Campo    | Padrão       | Significado                                      |
| -------- | ------------ | ------------------------------------------------ |
| `offset` | `(0, 0)`     | Centro da caixa relativo à posição do transform  |
| `size`   | `(100, 100)` | Largura e altura totais da caixa                 |

### Quem lê

Enquanto a cena está tocando, o `scene_player_t` testa todo par de entidades do sistema `collision_2D` (transform e box collider) a cada update. Cada caixa é centrada em `position + offset` e tem `size` de largura e altura. A escala e a rotação do transform não são aplicadas a ela. Caixas que se tocam numa borda contam como sobrepostas.

Para cada par sobreposto, o script system chama `on_collision(entity, other)` nos scripts das duas entidades, uma vez em cada direção. Isso se repete a cada update enquanto as caixas estiverem sobrepostas. Os colliders não empurram as entidades para longe uma da outra.

O Lua não tem acesso direto a colliders.

## `camera_2D_component_t`

```cpp
struct camera_2D_component_t
{
    f32  zoom;
    bool is_primary;
};
```

### Estado

| Campo        | Padrão  | Significado                                               |
| ------------ | ------- | --------------------------------------------------------- |
| `zoom`       | `1`     | Escala da visão. `2` mostra metade da largura e da altura |
| `is_primary` | `true`  | Se esta câmera pode ser usada na renderização             |

### Quem lê

No início do `scene_t::render()`, a cena percorre o sistema `camera_2D` e pega a primeira entidade cuja câmera tem `is_primary` ligado. A `position` do transform dela vira o centro da visão e o `zoom` dela escala a área visível: a visão é o tamanho do viewport dividido por `zoom`. Se nenhuma câmera é primária, ou o `zoom` da câmera escolhida é `0` ou menos, o renderer volta para coordenadas em pixels com a origem no canto superior esquerdo.

A rotação e a escala do transform da câmera não são usadas. O editor limita o `zoom` entre `0.01` e `100` no campo de arrastar. O Lua não tem acesso a câmeras.

## `point_light_2D_component_t`

```cpp
struct point_light_2D_component_t
{
    math::vec4_t color;
    f32          radius;
    f32          intensity;
};
```

### Estado

| Campo       | Padrão              | Significado                                          |
| ----------- | ------------------- | ---------------------------------------------------- |
| `color`     | `(1, 0.85, 0.6, 1)` | Cor que a luz soma                                   |
| `radius`    | `250`               | Distância em unidades de mundo onde a luz chega a zero |
| `intensity` | `1`                 | Multiplica a cor                                     |

### Quem lê

O sistema de render `light_2D` percorre toda entidade com transform e point light, depois que a câmera definiu a projeção. Ele limpa o mapa de luz com a cor ambiente do `scene_render_context_t` e soma cada luz com o `point_light_renderer_t`: a luz é mais forte na `position` do transform e cai linearmente até zero no `radius`. Só sprites com `is_lit` ligado leem o mapa de luz; o resto não é afetado pelas luzes.

## `tags_component_t` e `parent_component_t`

```cpp
struct tags_component_t
{
    char tags[TAGS_MAX_LENGTH];
    bool has_tag(const char* tag) const;
};

struct parent_component_t
{
    ecs::entity_t parent;
};
```

`tags` é uma lista separada por vírgula, como `"enemy, flying"`; o Lua encontra entidades com tag usando `scene.find_with_tag(tag)`. `parent_component_t` torna o transform da entidade local ao `parent`. Os dois estão explicados no [módulo de cena](../scene/README.md#hierarquia). O editor edita tags no inspector e define pais arrastando entidades no Scene Hierarchy.

## `audio_source_component_t`

```cpp
constexpr usize AUDIO_CLIP_PATH_MAX_LENGTH = 256;

struct audio_source_component_t
{
    char clip_path[AUDIO_CLIP_PATH_MAX_LENGTH];
    f32  volume;
    bool loop;
    bool play_on_start;
};
```

### Estado

| Campo           | Padrão  | Significado                                             |
| --------------- | ------- | ------------------------------------------------------- |
| `clip_path`     | vazio   | Arquivo de som a tocar, até 255 caracteres              |
| `volume`        | `1`     | Volume linear passado à engine de áudio                 |
| `loop`          | `false` | Se o som se repete                                      |
| `play_on_start` | `false` | Tocar automaticamente quando a cena começa              |

### Quem lê

Quando o scene player começa, ele percorre o sistema `audio` e chama `audio_engine_t::play(clip_path, volume, loop)` para toda fonte com `play_on_start` ligado. O Lua chama a mesma função por `audio.play(entity)`, que retorna um handle para `audio.stop(handle)`.

O componente só guarda configurações. Ele não guarda o handle de um som que iniciou. O slider de volume do editor vai de `0` a `1`. Veja o [módulo de áudio](../audio/README.md) para entender como a reprodução funciona.

## `script_component_t`

```cpp
constexpr usize SCRIPT_PATH_MAX_LENGTH = 256;

struct script_component_t
{
    char script_path[SCRIPT_PATH_MAX_LENGTH];
};
```

### Estado

| Campo         | Padrão  | Significado                              |
| ------------- | ------- | ---------------------------------------- |
| `script_path` | vazio   | Arquivo Lua a rodar, até 255 caracteres  |

### Quem lê

Quando o scene player começa, o script system percorre o sistema `script`, que exige só este componente. Para cada entidade com caminho não vazio, carrega o arquivo em um ambiente próprio e chama `on_start(entity)` se o script define essa função. A cada update chama `on_update(entity, delta_time)`, e nas colisões `on_collision(entity, other)`. Entidades com caminho vazio, ou cujo script falha ao carregar, são puladas.

O caminho é aberto como foi passado, então um caminho relativo é relativo ao diretório de trabalho do processo. No editor, o botão Edit Script abre o arquivo no script editor e fica desabilitado enquanto o caminho está vazio.

## Uso

O `scene_t::initialize()` registra cada struct com o tamanho dela e um nome, e guarda o `component_type_t*` retornado em um membro correspondente (`name_type`, `transform_2D_type`, e assim por diante):

```cpp
transform_2D_type = registry.register_component_type(sizeof(components::transform_2D_component_t), transform_2D_type_name, nullptr);
```

Adicionar um componente monta a struct na pilha com os padrões e copia ela para o registry:

```cpp
void scene_t::add_velocity_2D(const ecs::entity_t entity)
{
    components::velocity_2D_component_t velocity = {};
    velocity.speed = default_speed;

    add_component(*velocity_2D_type, &velocity, entity);
}
```

Ler um componente passa por um getter tipado que converte o ponteiro vindo do ECS, e retorna `nullptr` se a entidade não tem o componente:

```cpp
components::transform_2D_component_t* transform = scene->get_transform_2D(entity);
```

O object inspector do editor chama esses getters para a entidade selecionada e liga os widgets do ImGui diretamente aos campos, então as edições mudam o componente guardado no lugar. As funções Lua fazem o mesmo: buscam o componente para o id de entidade que recebem e levantam um erro Lua como `entity 3 has no velocity_2D` se ele não existe.

## Observações

- Os headers não definem padrões. Uma struct criada fora do `scene_t` começa com o que o inicializador der, e um sprite zerado tem tamanho `(0, 0)` e um tint totalmente transparente.
- Câmeras novas são primárias por padrão. Com várias câmeras primárias, a que foi adicionada primeiro é a usada.
- Labels precisam de um caminho de fonte antes de serem desenhados. Sprites sem textura continuam sendo desenhados, como retângulos sólidos.
- Labels e colliders ignoram a escala e a rotação do transform. Só os sprites usam os três campos do transform.
- Física e colisões só rodam enquanto a cena está tocando. Sprites e labels são desenhados a cada frame, tocando ou não.
- Ponteiros retornados pelos getters da cena apontam para dentro do buffer do ECS, então deixam de ser válidos depois que um componente do mesmo tipo é adicionado ou removido.
