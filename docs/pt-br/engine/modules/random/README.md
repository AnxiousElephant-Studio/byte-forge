# Módulo de aleatoriedade

> Idioma: [English](../../../../en/engine/modules/random/README.md) | **Português (Brasil)**

O módulo de aleatoriedade produz números pseudoaleatórios. Ele é uma struct header-only, `random_t`, que encapsula um Mersenne Twister da biblioteca padrão (`std::mt19937`) e oferece intervalos para inteiros, floats e vetores 2D. Fica no namespace `byteforge::random` e não tem arquivo de código-fonte.

## Arquivos

| Arquivo                                                      | Conteúdo                                 |
| ------------------------------------------------------------ | ---------------------------------------- |
| `engine/include/byte-forge-engine/modules/random/random.h`   | Declaração e implementação do `random_t` |

## Conceitos

### Gerador e distribuições

Cada `random_t` é dono do seu próprio gerador. O construtor faz o seed dele uma vez com um valor de `std::random_device`, então duas instâncias, ou duas execuções do programa, produzem sequências diferentes. Não há como passar um seed, então uma sequência não pode ser reproduzida.

Cada chamada cria uma distribuição padrão nova (`std::uniform_int_distribution` ou `std::uniform_real_distribution`) com os limites pedidos, tira um valor do gerador e descarta a distribuição.

### Limites

| Família de função | Intervalo                                     |
| ----------------- | --------------------------------------------- |
| Inteiro           | Fechado: tanto `min` quanto `max` podem ser retornados |
| Float             | Semiaberto: `min` pode ser retornado, `max` não |

Essas são as regras das distribuições padrão que a struct usa.

## `random_t`

```cpp
struct random_t
{
public:
    random_t() : generator(std::random_device{}()) {}

    i32 range_int(i32 min, i32 max);
    f32 range_float(f32 min, f32 max);
    math::vec2_t range_vec2(const math::vec2_t& min, const math::vec2_t& max);

    f32 next_float();
    i32 next_int();
    math::vec2_t next_vec2();

private:
    std::mt19937 generator;
};
```

Todos os membros são definidos inline no header. A struct não é marcada com `BF_RANDOM_API`, então cada aplicação que inclui ela compila sua própria cópia.

### Estado

| Campo       | Significado                                                    |
| ----------- | -------------------------------------------------------------- |
| `generator` | Mersenne Twister de 32 bits. Avança a cada chamada             |

### `range_int(min, max)`

Retorna um inteiro uniformemente distribuído em `[min, max]`. `min` não pode ser maior que `max`.

### `range_float(min, max)`

Retorna um float uniformemente distribuído em `[min, max)`. `min` não pode ser maior que `max`.

### `range_vec2(min, max)`

Retorna um `math::vec2_t` cujo `x` é `range_float(min.x, max.x)` e cujo `y` é `range_float(min.y, max.y)`. Os dois componentes são sorteados de forma independente, então o resultado fica dentro do retângulo formado por `min` e `max`.

### `next_float()`, `next_int()`, `next_vec2()`

Atalhos para os intervalos unitários:

| Função        | Mesmo que                                        | Resultado                      |
| ------------- | ------------------------------------------------ | ------------------------------ |
| `next_float()`| `range_float(0.0f, 1.0f)`                        | Float em `[0, 1)`              |
| `next_int()`  | `range_int(0, 1)`                                | `0` ou `1`                     |
| `next_vec2()` | `range_vec2(vec2_t::zero(), vec2_t::one())`      | Os dois componentes em `[0, 1)` |

## Uso

Nenhum código da engine ou do editor usa `random_t` ainda. Quem chama cria uma instância e guarda ela, para que o gerador receba o seed uma vez só:

```cpp
#include <byte-forge-engine/modules/random/random.h>

byteforge::random::random_t random;

const i32 die = random.range_int(1, 6);
const math::vec2_t spawn = random.range_vec2(math::vec2_t(0.0f, 0.0f), math::vec2_t(800.0f, 600.0f));
```

## Notas

- `next_int()` retorna só `0` ou `1`, como um cara ou coroa, não um inteiro qualquer.
- Passar `min` maior que `max` quebra a pré-condição das distribuições padrão, e o resultado é indefinido.
- `random_t` guarda um `std::mt19937`, que tem cerca de 5 KB. Copiar um `random_t` copia o estado do gerador, então a cópia produz a mesma sequência que o original a partir daquele ponto.
- A struct não tem locking. Usar uma instância de várias threads ao mesmo tempo é uma data race.
- Scripts Lua não alcançam este módulo. Os scripts podem usar o `math.random` do próprio Lua.
