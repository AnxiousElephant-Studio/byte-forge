# Módulo ECS

> Idioma: [English](../../../../en/engine/modules/ecs/README.md) | **Português (Brasil)**

O módulo ECS é o entity component system da engine. Entidades são números simples, componentes são bytes crus guardados em um buffer compacto por tipo de componente, e sistemas são listas das entidades que têm um certo conjunto de componentes. Tudo pertence a um `registry_t`. O módulo fica no namespace `byteforge::ecs` e não depende de bibliotecas de terceiros. Dentro da engine ele só depende do módulo de logging: não inclui rendering, platform nem scene, então pode ser usado (e mais tarde compilado) sozinho.

O `entity.h` é só header. Os outros três tipos são declarados em headers e implementados em `engine/source/modules/ecs/`, com o `registry_t` dividido em vários arquivos fonte por assunto.

## Arquivos

| Arquivo                                                         | Conteúdo                                                        |
| --------------------------------------------------------------- | --------------------------------------------------------------- |
| `engine/include/byte-forge-engine/modules/ecs/entity.h`         | `entity_t`, um invólucro em volta de um id `u64`                |
| `engine/include/byte-forge-engine/modules/ecs/component_type.h` | `component_type_t` e `component_free_function_t`                |
| `engine/include/byte-forge-engine/modules/ecs/system.h`         | `system_t`, `system_update_function_t`, `system_free_function_t` |
| `engine/include/byte-forge-engine/modules/ecs/registry.h`       | `registry_t` e `MAX_COMPONENT_TYPES`                            |
| `engine/source/modules/ecs/component_type.cpp`                  | Armazenamento de componentes: adicionar, substituir, remover, debug |
| `engine/source/modules/ecs/system.cpp`                          | Correspondência do sistema e lista de entidades                 |
| `engine/source/modules/ecs/registry.cpp`                        | `update()`, `print_info()`, `dispose()`                         |
| `engine/source/modules/ecs/registry_entity.cpp`                 | Criar, spawnar e matar entidades                                |
| `engine/source/modules/ecs/registry_component.cpp`              | Registrar tipos de componente, adicionar e remover componentes  |
| `engine/source/modules/ecs/registry_system.cpp`                 | `create_system()`                                               |
| `engine/source/modules/ecs/registry_validation.cpp`             | `validate()` e `get_memory_bytes()`                             |

Peças relacionadas em outros módulos:

| Arquivo                                                   | Papel                                                          |
| --------------------------------------------------------- | -------------------------------------------------------------- |
| `modules/components/*.h`                                  | As structs de componente que a cena guarda no registry         |
| `modules/scene/scene.cpp`                                 | Dono do registry, registra os tipos e cria os sistemas         |
| `modules/scene/scene_player.cpp`                          | Percorre as listas dos sistemas de física, colisão e áudio     |
| `modules/scene/scene_script_system.cpp`                   | Percorre a lista do sistema de script, passa ids de entidade ao Lua |
| `applications/editor/.../ecs_observer_component.cpp`      | Janela do editor que mostra o registry e roda o `validate()`   |

## Conceitos

### Entidades são ids

Uma entidade é só um `u64`. O registry entrega os ids em ordem, começando em `0`, a partir de `entities_current_id`. Um id nunca é reutilizado: matar uma entidade não devolve o id dela.

O id também é um índice. `entities_components_bitsets[id]` guarda o bitset de componentes da entidade, então esse vetor tem uma posição para cada id já criado, inclusive ids de entidades que foram mortas.

### Bitsets de componentes

Cada tipo de componente registrado recebe um id igual à sua posição em `component_types` (`0`, `1`, `2`, ...). Esse id é uma posição de bit. O bitset de uma entidade tem o bit `n` ligado quando ela tem um componente do tipo `n`.

Como o bitset é um `u64`, pode haver no máximo 64 tipos de componente (`MAX_COMPONENT_TYPES`).

### Armazenamento compacto de componentes

Um `component_type_t` guarda todos os componentes do seu tipo um atrás do outro em um único `std::vector<u8>`. O elemento `i` ocupa os bytes `[i * size, (i + 1) * size)`. Um mapa, `entity_id_to_index`, diz qual elemento pertence a qual entidade.

O ECS não conhece o tipo C++ de um componente. Ele copia os bytes com `std::memcpy` e devolve `void*`. Quem chama converte o ponteiro para a struct certa. Isso só funciona para tipos que podem ser copiados byte a byte com segurança. Para um componente que é dono de memória, o tipo pode ser registrado com uma função de liberação, que o ECS chama antes de os bytes serem sobrescritos ou descartados.

### Sistemas são listas de entidades

Um `system_t` tem um bitset exigido e uma lista de entidades. Uma entidade pertence ao sistema quando o bitset dela contém todos os bits do bitset exigido. O registry só atualiza essas listas quando pedem: adicionar ou remover um componente muda o bitset, mas as listas dos sistemas só mudam no `spawn_entity()`, no `refresh_entity_systems()` ou no `kill_entity()`.

Um sistema também pode carregar uma função de update e um ponteiro de dados. O `registry_t::update()` chama as funções de update. Na engine atual nenhum sistema usa uma: o código que precisa de um sistema lê a lista `entities` dele diretamente.

### Operações imediatas e adiadas

Criar e destruir entidades existe em duas formas:

| Imediata               | Adiada                                                            |
| ---------------------- | ----------------------------------------------------------------- |
| `spawn_entity(entity)` | `publish_entity(entity)`, depois `spawn_entities()`               |
| `kill_entity(entity)`  | `add_entity_to_kill_queue(entity)`, depois `kill_entities()`      |

A forma adiada coloca a entidade em uma fila (`entities_to_add` ou `entities_to_kill`) e aplica a fila inteira depois. Isso permite pedir mudanças enquanto se itera sobre a lista de um sistema sem mudar a lista por baixo do loop. A cena e o editor hoje usam só a forma imediata.

## `entity_t`

```cpp
struct entity_t
{
public:
    u64 id;

    constexpr entity_t()       : id(0) {}
    constexpr entity_t(u64 id) : id(id) {}

    constexpr bool operator==(const entity_t& other) const { return id == other.id; }
};
```

Um tipo de valor que envolve o id. Duas entidades são iguais quando os ids são iguais. Não há `operator!=` nem hash, então os mapas do ECS usam `entity.id` como chave.

O construtor padrão dá o id `0`, que também é o id da primeira entidade que o registry cria. Um `entity_t` construído por padrão, portanto, não é um valor "vazio". É por isso que o editor guarda uma flag `has_selection` separada ao lado do `selected_entity`.

## `component_type_t`

```cpp
using component_free_function_t = void (*)(void* component_data);

struct BF_ECS_API component_type_t
{
public:
    u64                            id;
    usize                          size;
    std::string                    name;
    std::vector<u8>                data;
    std::unordered_map<u64, usize> entity_id_to_index;
    component_free_function_t      free_function;

    component_type_t(u64 id, usize size, const char* name, component_free_function_t free_function);

    usize get_count() const;
    usize get_memory_bytes() const;
    void* get(const entity_t entity);
    bool  has(const entity_t entity) const;
    void  add_or_replace(const entity_t entity, const void* const component);
    void  remove(const entity_t entity);
    void  print_info(const usize registry_index) const;
    void  dispose();
};
```

As instâncias são criadas com `new` pelo `registry_t::register_component_type()` e deletadas pelo `registry_t::dispose()`. O código fora do registry recebe um ponteiro e não é dono dele.

### Estado

| Campo                | Significado                                                              |
| -------------------- | ------------------------------------------------------------------------ |
| `id`                 | Índice em `component_types` do registry, e o bit usado nos bitsets       |
| `size`               | Tamanho de um componente em bytes, normalmente o `sizeof` da struct      |
| `name`               | Nome dado no registro. Usado em buscas e na saída de debug               |
| `data`               | Todos os componentes desse tipo, um atrás do outro                       |
| `entity_id_to_index` | Id da entidade para o índice do elemento em `data`                       |
| `free_function`      | Chamada sobre os bytes de um componente antes de serem sobrescritos ou descartados. Pode ser `nullptr` |

### `get_count()` e `get_memory_bytes()`

| Membro               | Retorna                                                                     |
| -------------------- | --------------------------------------------------------------------------- |
| `get_count()`        | `data.size() / size`, ou `0` quando `size` é `0`                            |
| `get_memory_bytes()` | `data.capacity()` mais `(sizeof(u64) + sizeof(usize))` por entrada do mapa. Uma estimativa: não inclui o custo dos nós e buckets do próprio mapa |

### `get(entity)`

Procura o id da entidade em `entity_id_to_index`. Retorna `nullptr` se a entidade não tem componente desse tipo, senão um ponteiro para o elemento dela em `data`.

O ponteiro aponta para dentro do vetor. Qualquer `add_or_replace()` posterior que adicione um elemento novo pode realocar `data`, e qualquer `remove()` desloca os elementos depois do removido, então um ponteiro do `get()` não deve ser guardado entre essas chamadas.

### `has(entity)`

`true` se o id da entidade está em `entity_id_to_index`.

### `add_or_replace(entity, component)`

1. Se a entidade já tem um componente desse tipo, chama `free_function` sobre os bytes existentes (quando definida), copia `size` bytes de `component` por cima deles e retorna. O elemento mantém o índice.
2. Senão, aumenta `data` em `size` bytes, copia o componente para o novo último elemento e registra o índice no mapa.

Não mexe em nenhum bitset. O `add_component_to_entity()` do registry liga o bit e depois chama este.

### `remove(entity)`

1. Retorna se a entidade não tem componente desse tipo.
2. Chama `free_function` sobre os bytes do elemento, quando definida.
3. Apaga os bytes do elemento de `data`. Todo elemento depois dele desce uma posição.
4. Apaga a entidade do mapa, depois percorre o mapa inteiro e decrementa todo índice maior que o removido.

A remoção mantém os elementos restantes na ordem original. O custo cresce tanto com o número de bytes depois do elemento removido quanto com o número de entradas no mapa.

### `print_info(registry_index)`

Registra, em nível debug, um bloco com o índice recebido, o id, o tamanho, o nome e a contagem de elementos do tipo, seguido de uma linha `Entity ID: <id> -> Array Index: <index>` por entrada do mapa.

### `dispose()`

Chama `free_function` em todo elemento (quando definida), depois limpa `data` e o mapa. O objeto `component_type_t` em si continua vivo e pode ser reutilizado.

## `system_t`

```cpp
class registry_t;
struct system_t;

using system_update_function_t = void (*)(registry_t& registry, system_t& system, const f64 delta_time);
using system_free_function_t   = void (*)(void* system_data);

struct BF_ECS_API system_t
{
public:
    u64                      id;
    std::string              name;
    void*                    data;
    u64                      required_entity_components_bitset;
    system_update_function_t update_function;
    system_free_function_t   free_function;
    std::vector<entity_t>    entities;

    system_t(u64 id, const char* name, void* data, u64 required_entity_components_bitset,
             system_update_function_t update_function, system_free_function_t free_function);

    bool accepts(const u64 entity_components_bitset) const;
    bool contains_entity(const entity_t entity) const;
    void add_entity(const entity_t entity);
    void remove_entity(const entity_t entity);
    void print_info(const usize registry_index) const;
    void dispose();
};
```

Assim como os tipos de componente, os sistemas são criados com `new` pelo `registry_t::create_system()` e deletados pelo `registry_t::dispose()`.

### Estado

| Campo                               | Significado                                                    |
| ----------------------------------- | -------------------------------------------------------------- |
| `id`                                | Índice em `systems` do registry                                |
| `name`                              | Nome dado na criação, usado na saída de debug e na validação   |
| `data`                              | Dados opcionais do usuário, passados ao `free_function` no dispose |
| `required_entity_components_bitset` | Bits que uma entidade precisa ter para pertencer a este sistema |
| `update_function`                   | Chamada pelo `registry_t::update()`. Pode ser `nullptr`        |
| `free_function`                     | Libera `data` no dispose. Pode ser `nullptr`                   |
| `entities`                          | As entidades que estão no sistema agora, na ordem em que entraram |

### `accepts(bitset)`

`true` quando `bitset` contém todos os bits de `required_entity_components_bitset`, ou seja `(required & bitset) == required`. Bits extras não importam. Um sistema com bitset exigido `0` aceita qualquer entidade.

### `contains_entity(entity)`

Uma busca linear em `entities`.

### `add_entity(entity)` e `remove_entity(entity)`

O `add_entity()` coloca a entidade no fim sem verificar se ela já está lá. O `remove_entity()` procura a primeira ocorrência e a apaga, mantendo a ordem das outras. Se a entidade não está na lista, não faz nada.

### `print_info(registry_index)`

Registra, em nível debug, o índice recebido, o id, o nome, o bitset exigido em 16 dígitos hexadecimais e a contagem de entidades.

### `dispose()`

Se `free_function` e `data` estão os dois definidos, chama `free_function(data)`. Depois coloca `data` em `nullptr` e limpa `entities`.

## `registry_t`

```cpp
constexpr usize MAX_COMPONENT_TYPES = 64;

class BF_ECS_API registry_t
{
public:
    std::vector<entity_t>          entities;
    std::vector<component_type_t*> component_types;
    std::vector<system_t*>         systems;
    std::vector<u64>               entities_components_bitsets;
    std::vector<entity_t>          entities_to_add;
    std::vector<entity_t>          entities_to_kill;
    u64                            entities_current_id;

    registry_t() : entities_current_id(0) {}

    void  update(const f64 delta_time);
    void  print_info() const;
    usize validate(std::vector<std::string>& out_issues) const;
    usize get_memory_bytes() const;
    void  dispose();

    entity_t create_entity();
    void     publish_entity(const entity_t entity);
    void     spawn_entities();
    void     spawn_entity(const entity_t entity);
    void     add_entity_to_kill_queue(const entity_t entity);
    void     kill_entities();
    void     kill_entity(const entity_t entity);
    void     refresh_entity_systems(const entity_t entity);

    component_type_t* register_component_type(const usize size, const char* name, component_free_function_t free_function);
    component_type_t* get_component_type_by_name(const char* name) const;
    void              add_component_to_entity(component_type_t& component_type, const void* const component, const entity_t entity);
    void              remove_component_from_entity(component_type_t& component_type, const entity_t entity);

    system_t* create_system(const u64 required_entity_components_bitset, const char* name,
                            system_update_function_t update_function, system_free_function_t free_function, void* data);
};
```

O construtor de cópia e a atribuição por cópia são deletados. O registry guarda ponteiros crus donos dos seus tipos de componente e sistemas, e uma cópia os deletaria duas vezes.

Não há destrutor. Os tipos de componente e os sistemas só são liberados no `dispose()`, que o dono precisa chamar.

Todos os campos são públicos. O editor lê eles diretamente para desenhar os painéis.

### Estado

| Campo                         | Significado                                                             |
| ----------------------------- | ----------------------------------------------------------------------- |
| `entities`                    | Entidades vivas, na ordem de criação                                    |
| `component_types`             | Tipos de componente registrados, indexados pelo id                      |
| `systems`                     | Sistemas criados, indexados pelo id                                     |
| `entities_components_bitsets` | Um bitset por id de entidade já criado, indexado pelo id                |
| `entities_to_add`             | Fila preenchida pelo `publish_entity()`, aplicada pelo `spawn_entities()` |
| `entities_to_kill`            | Fila preenchida pelo `add_entity_to_kill_queue()`, aplicada pelo `kill_entities()` |
| `entities_current_id`         | O id que o próximo `create_entity()` vai retornar. Começa em `0`        |

### `create_entity()`

Pega o próximo id, coloca a entidade no fim de `entities` e coloca um bitset zerado no fim de `entities_components_bitsets`. Retorna a entidade.

A entidade está viva a partir daí, mas não está em nenhum sistema até o `spawn_entity()` (ou `publish_entity()` e `spawn_entities()`) ser chamado.

### `register_component_type(size, name, free_function)`

Se `MAX_COMPONENT_TYPES` tipos já estão registrados, registra `Could not register component type '<name>': the limit of 64 component types was reached.` e retorna `nullptr`.

Senão cria um `component_type_t` cujo id é o número atual de tipos, coloca no fim e retorna. Não verifica se o nome já está em uso nem se `size` é `0`.

### `get_component_type_by_name(name)`

Uma busca linear comparando `name` com o nome de cada tipo. Retorna a primeira correspondência ou `nullptr`. Nada na engine nem no editor chama isso hoje.

### `add_component_to_entity(component_type, component, entity)`

1. Se `entity.id` não está abaixo do tamanho de `entities_components_bitsets`, registra `Could not add component '<type>' to entity <id>: the entity does not exist.` e retorna.
2. Liga o bit do tipo no bitset da entidade.
3. Chama `component_type.add_or_replace()`, que copia os bytes.

As listas dos sistemas não são atualizadas. Chame `refresh_entity_systems()` depois, ou `spawn_entity()` se a entidade ainda não foi spawnada.

### `remove_component_from_entity(component_type, entity)`

Se o id está no intervalo, desliga o bit do tipo e chama `component_type.remove()`. Um id fora do intervalo não faz nada e não registra nada. Assim como adicionar, não atualiza as listas dos sistemas.

### `spawn_entity(entity)`

Lê o bitset da entidade e chama `add_entity()` em todo sistema que o aceita. Ids fora do intervalo são ignorados.

Não verifica se a entidade já está em um sistema, então é para ser chamado uma vez por entidade. Mudanças posteriores passam pelo `refresh_entity_systems()`.

### `refresh_entity_systems(entity)`

Para cada sistema, compara "aceita o bitset atual" com "já contém a entidade":

| Aceita | Contém | Ação                    |
| ------ | ------ | ----------------------- |
| sim    | não    | `add_entity(entity)`    |
| não    | sim    | `remove_entity(entity)` |
| sim    | sim    | nada                    |
| não    | não    | nada                    |

Como ele verifica `contains_entity()` antes, chamar mais de uma vez é seguro.

### `publish_entity(entity)` e `spawn_entities()`

O `publish_entity()` coloca a entidade no fim de `entities_to_add`. O `spawn_entities()` chama `spawn_entity()` em cada entidade da fila, em ordem, e depois limpa a fila.

### `kill_entity(entity)`

1. Chama `remove_entity()` em todo sistema.
2. Chama `remove()` em todo tipo de componente, o que roda as funções de liberação.
3. Se o id está no intervalo, zera o bitset dele. A posição continua no vetor.
4. Procura a entidade em `entities` e a apaga, mantendo a ordem das outras.

Não verifica se a entidade está viva. Para um id que nunca foi criado ou que já foi morto, os mesmos passos rodam e normalmente não encontram nada para remover.

### `add_entity_to_kill_queue(entity)` e `kill_entities()`

O `add_entity_to_kill_queue()` coloca a entidade no fim de `entities_to_kill`. O `kill_entities()` chama `kill_entity()` em cada entidade da fila, em ordem, e depois limpa a fila.

### `create_system(required_bitset, name, update_function, free_function, data)`

Cria um `system_t` cujo id é o número atual de sistemas, coloca no fim e retorna. Não há limite para o número de sistemas.

O sistema novo começa vazio. Entidades que já existem não são adicionadas a ele. Os sistemas são feitos para serem criados antes de as entidades serem spawnadas.

### `update(delta_time)`

Chama o `update_function` de cada sistema, na ordem de criação, com o registry, o sistema e `delta_time`. Sistemas sem função de update são pulados.

### `print_info()`

Registra o número de entidades vivas e dois títulos em nível info, depois chama `print_info()` em todo sistema e todo tipo de componente, que registram em nível debug.

### `validate(out_issues)`

Verifica se as estruturas de dados do registry estão de acordo entre si. Limpa `out_issues`, coloca uma mensagem por problema encontrado e retorna o número de mensagens. Não muda nada.

As verificações, em ordem:

1. `entities_components_bitsets` tem exatamente `entities_current_id` entradas.
2. Para cada tipo de componente:
   - `size` não é `0`. Se for, as outras verificações desse tipo são puladas.
   - `data` tem um múltiplo de `size` bytes.
   - O mapa tem tantas entradas quanto há elementos.
   - Todo índice do mapa está no intervalo, e duas entidades não dividem um índice.
   - Toda entidade do mapa está viva, e o bitset dela tem o bit do tipo ligado.
3. Para cada entidade viva, todo bit ligado pertence a um tipo registrado, e esse tipo tem um componente para a entidade.
4. Para cada sistema:
   - Nenhuma entidade aparece duas vezes.
   - Toda entidade nele está viva e é aceita pelo sistema.
   - Toda entidade viva que o sistema aceita está nele. Entidades esperando em `entities_to_add` ficam de fora dessa verificação.

Cada mensagem é formatada em um buffer de 256 bytes, então um nome de componente ou de sistema muito longo pode cortá-la.

### `get_memory_bytes()`

Uma estimativa da memória que os dados do ECS usam: as capacidades de `entities`, `entities_components_bitsets` e das duas filas vezes o tamanho do elemento, mais o `get_memory_bytes()` de todo tipo de componente, mais a capacidade da lista de entidades de todo sistema. Não conta os vetores de ponteiros `component_types` e `systems`, os objetos para onde eles apontam, os nomes nem o `data` dos sistemas.

### `dispose()`

1. Para cada sistema, chama `dispose()` e o deleta.
2. Para cada tipo de componente, chama `dispose()` (que roda as funções de liberação) e o deleta.
3. Limpa todos os vetores e volta `entities_current_id` para `0`.

Depois do `dispose()` o registry fica vazio e pode ser usado de novo. Ponteiros para tipos de componente e sistemas obtidos antes ficam pendurados.

## Uso

O `scene_t` é dono do registry como membro (`ecs::registry_t registry`). No editor, o `scene.initialize()` roda na inicialização e o `scene.dispose()` no encerramento, que chama `registry.dispose()`.

O `scene_t::initialize()` registra nove tipos de componente, todos com função de liberação `nullptr`, e depois cria sete sistemas sem função de update. Os ids seguem a ordem de registro:

| Id do tipo | Tipo de componente | Struct                        |
| ---------- | ------------------ | ----------------------------- |
| 0          | `name`             | `name_component_t`            |
| 1          | `transform_2D`     | `transform_2D_component_t`    |
| 2          | `sprite_2D`        | `sprite_2D_component_t`       |
| 3          | `script`           | `script_component_t`          |
| 4          | `velocity_2D`      | `velocity_2D_component_t`     |
| 5          | `box_collider_2D`  | `box_collider_2D_component_t` |
| 6          | `audio_source`     | `audio_source_component_t`    |
| 7          | `label_2D`         | `label_2D_component_t`        |
| 8          | `camera_2D`        | `camera_2D_component_t`       |

| Id do sistema | Sistema        | Tipos de componente exigidos      |
| ------------- | -------------- | --------------------------------- |
| 0             | `sprite_2D`    | `transform_2D`, `sprite_2D`       |
| 1             | `script`       | `script`                          |
| 2             | `physics_2D`   | `transform_2D`, `velocity_2D`     |
| 3             | `collision_2D` | `transform_2D`, `box_collider_2D` |
| 4             | `audio`        | `audio_source`                    |
| 5             | `label_2D`     | `transform_2D`, `label_2D`        |
| 6             | `camera_2D`    | `transform_2D`, `camera_2D`       |

A cena monta um bitset exigido deslocando `1` pelo id de cada tipo:

```cpp
const u64 transform_2D_bit = get_component_bit(transform_2D_type);

sprite_2D_system = registry.create_system(transform_2D_bit | get_component_bit(sprite_2D_type), sprite_2D_system_name, nullptr, nullptr, nullptr);
```

Criar uma entidade adiciona os dois componentes que toda entidade tem e depois a spawna:

```cpp
const ecs::entity_t entity = registry.create_entity();

// ... fill name_component and transform ...

registry.add_component_to_entity(*name_type, &name_component, entity);
registry.add_component_to_entity(*transform_2D_type, &transform, entity);
registry.spawn_entity(entity);
```

Adições e remoções posteriores (pelo object inspector do editor) passam pelo `scene_t::add_component()` e pelo `scene_t::remove_component()`, que chamam `add_component_to_entity()` ou `remove_component_from_entity()` e depois `refresh_entity_systems()`. Apagar uma entidade pela scene hierarchy chama `scene_t::destroy_entity()`, que chama `kill_entity()` na hora.

Os sistemas são lidos, não executados. O `registry.update()` nunca é chamado. Em vez disso:

- O `scene_t::render()` percorre `camera_2D_system`, `sprite_2D_system` e `label_2D_system`.
- O `scene_player_t` percorre `physics_2D_system` e `collision_2D_system` a cada update e `audio_system` no start.
- O `scene_script_system_t` percorre `script_system` no start e a cada update.

O Lua enxerga entidades como inteiros. Os callbacks de script recebem `entity.id`, e as funções da API Lua transformam o inteiro de volta em `entity_t` com `luaL_checkinteger`.

O editor lê o registry em dois painéis. A janela ECS Observer lista as entidades com os nomes dos componentes e o bitset, os tipos de componente com contagem e memória, os sistemas com os tipos exigidos e a contagem de entidades, e tem uma aba Validation que chama `validate()` sob demanda ou a cada frame. O status panel mostra as contagens de entidades e das filas, o número de tipos de um total de `MAX_COMPONENT_TYPES`, o número de sistemas e o `get_memory_bytes()` em kilobytes.

## Observações

- Os ids nunca são reutilizados e `entities_components_bitsets` nunca diminui até o `dispose()`. Uma entidade morta mantém a posição dela com bitset `0`.
- O `add_component_to_entity()` só verifica se o id foi criado, não se a entidade está viva. Adicionar um componente a uma entidade morta guarda o componente, e o `validate()` passa a acusá-lo como componente de uma entidade que não está viva.
- O `kill_entity()` não tira a entidade de `entities_to_add`. Se uma entidade é publicada e depois morta antes do `spawn_entities()`, ela ainda é adicionada aos sistemas depois.
- Adicionar ou remover um componente não atualiza as listas dos sistemas sozinho. Um `refresh_entity_systems()` (ou `spawn_entity()` para uma entidade nova) precisa vir em seguida.
- Chamar `spawn_entity()` duas vezes para a mesma entidade a adiciona duas vezes em todo sistema que a aceita. O `validate()` acusa isso como duplicata.
- Os componentes são copiados com `std::memcpy` e devolvidos como `void*`, então precisam poder ser copiados byte a byte com segurança, e quem chama é responsável por converter para o tipo certo.
- Ponteiros retornados pelo `component_type_t::get()` ficam inválidos depois que o mesmo tipo adiciona um componente novo ou remove um.
- A remoção é linear: remover um componente desloca os bytes depois dele e reescreve índices no mapa, e remover uma entidade de um sistema ou de `entities` é uma busca mais um erase.
