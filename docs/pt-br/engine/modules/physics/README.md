# Módulo de física

> Idioma: [English](../../../../en/engine/modules/physics/README.md) | **Português (Brasil)**

O módulo de física guarda a matemática 2D que a cena usa para mover entidades e detectar sobreposições. Ele é uma struct, `physics_2D_t`, com duas funções estáticas e nenhum estado: uma move uma posição por uma velocidade, a outra testa duas caixas alinhadas aos eixos. Ele não encapsula nenhuma biblioteca de física. Fica no namespace `byteforge::physics` e não é header-only.

## Arquivos

| Arquivo                                                           | Conteúdo                                         |
| ----------------------------------------------------------------- | ------------------------------------------------ |
| `engine/include/byte-forge-engine/modules/physics/physics_2D.h`   | Declaração do `physics_2D_t`                     |
| `engine/source/modules/physics/physics_2D.cpp`                    | Implementação do `move` e do `check_aabb_collision` |

Peças relacionadas em outros módulos:

| Arquivo                                                                   | Papel                                             |
| ------------------------------------------------------------------------- | ------------------------------------------------- |
| `modules/components/velocity_2D_component.h`                              | Vetor de velocidade e speed de uma entidade       |
| `modules/components/box_collider_2D_component.h`                          | Offset e tamanho da caixa de colisão de uma entidade |
| `modules/scene/scene_player.cpp`                                          | Chama as duas funções a cada frame durante o play |
| `modules/scene/scene_script_system.cpp`                                   | Recebe as colisões como chamadas de `on_collision` |

## Conceitos

### Sem estado, sem mundo

Não existe mundo de física, corpo ou função de passo. O `physics_2D_t` só calcula valores a partir dos argumentos que recebe. A cena guarda os dados nos seus componentes, e o `scene_player_t` decide quando chamar as funções e o que fazer com os resultados.

### Posições são centros

As duas funções tratam uma posição como o centro do objeto. Uma caixa com posição `(10, 10)` e tamanho `(4, 2)` cobre x de `8` a `12` e y de `9` a `11`. Isso bate com o sprite renderer, que desenha um quad centrado na posição do transform.

### Unidades

As funções não têm unidade própria. Na cena, as posições estão em unidades de mundo, que são pixels quando nenhuma câmera está ativa, e o delta time está em segundos. Velocidade vezes speed é então o deslocamento por segundo.

## `physics_2D_t`

```cpp
struct BF_PHYSICS_API physics_2D_t
{
public:
    static math::vec2_t move(const math::vec2_t& position, const math::vec2_t& velocity, const f32 speed, const f32 delta_time);
    static bool check_aabb_collision(const math::vec2_t& a_position, const math::vec2_t& a_size, const math::vec2_t& b_position, const math::vec2_t& b_size);
};
```

A struct não tem campos e nunca é instanciada. Os dois membros são chamados como `physics::physics_2D_t::move(...)` e `physics::physics_2D_t::check_aabb_collision(...)`.

### `move(position, velocity, speed, delta_time)`

Retorna `position + velocity * speed * delta_time`. É um passo de Euler explícito: o resultado depende só da posição e da velocidade atuais, e nada é acumulado entre chamadas.

O `velocity` é uma direção e o `speed` a escala. A função não normaliza o `velocity`, então o comprimento dele também afeta a distância. Uma velocidade `(1, 1)` com speed `100` anda cerca de `141` unidades por segundo na diagonal, enquanto `(1, 0)` anda `100`.

### `check_aabb_collision(a_position, a_size, b_position, b_size)`

Testa se duas caixas alinhadas aos eixos se sobrepõem.

1. Calcula as metades dos tamanhos multiplicando cada tamanho por `0.5`.
2. No eixo x, as caixas se sobrepõem quando a borda esquerda de `a` é menor ou igual à borda direita de `b` e a borda direita de `a` é maior ou igual à borda esquerda de `b`.
3. O eixo y é verificado do mesmo jeito com as bordas de cima e de baixo.
4. Retorna `true` só quando os dois eixos se sobrepõem.

As comparações usam `<=` e `>=`, então duas caixas que só compartilham uma borda ou um canto contam como colidindo. A função só responde sim ou não. Ela não retorna a profundidade da sobreposição, a normal de contato nem qual lado foi atingido.

## Uso

Só o `scene_player_t` chama o módulo, a partir do `update()` enquanto a cena está em play e não pausada (ou durante um passo único):

- A passada de física percorre toda entidade com transform e velocidade e grava `move(transform.position, velocity.velocity, velocity.speed, delta_time)` de volta no transform. O `delta_time` já vem multiplicado pelo `time_scale` do player.
- A passada de colisão percorre todo par de entidades com transform e box collider. Para cada par ela chama `check_aabb_collision()` com `transform.position + collider.offset` e `collider.size` dos dois lados. Na sobreposição ela avisa os dois scripts pelo `on_collision`.

Do `scene_player.cpp`:

```cpp
transform->position = physics::physics_2D_t::move(transform->position, velocity->velocity, velocity->speed, delta_time);
```

Os scripts nunca chamam o módulo diretamente. Eles mudam as entradas dele com `velocity.set`, `velocity.set_speed` e `transform.set_position`, e reagem à saída dele no `on_collision`. Veja o [módulo de cena](../scene/README.md) para a ordem completa do frame.

## Notas

- As colisões são detectadas mas não resolvidas. Entidades sobrepostas continuam se atravessando a menos que um script mude a posição ou a velocidade delas.
- Movimento e colisão são passadas separadas. Uma entidade que anda o bastante num frame pode atravessar um collider fino sem nenhum frame em que as caixas se sobreponham.
- A caixa de colisão ignora a escala e a rotação do transform. Só o offset e o tamanho do próprio collider são usados.
- A passada de colisão testa todo par, então o custo dela cresce com o quadrado do número de colliders.
- Uma caixa com tamanho `(0, 0)` é um ponto. Ela ainda colide quando está dentro ou na borda de outra caixa.
