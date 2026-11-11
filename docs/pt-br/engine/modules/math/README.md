# Módulo de matemática

> Idioma: [English](../../../../en/engine/modules/math/README.md) | **Português (Brasil)**

O módulo de matemática reúne os tipos de valor pequenos que a engine usa para posições, tamanhos, cores, retângulos e transformações. Tudo é header-only, fica no namespace `byteforge::math` e é construído sobre os aliases de `byte-forge-engine/api/types.h` (`f32`, `i32`, `usize`).

## Arquivos

| Header                       | Tipo                 | Finalidade                                           |
| ---------------------------- | -------------------- | ---------------------------------------------------- |
| `vectors/vector2.h`          | `vec2_t`             | Vetor 2D de float (posições, tamanhos, UVs)          |
| `vectors/vector2i.h`         | `vec2i_t`            | Vetor 2D de inteiros (posições e tamanhos em pixels) |
| `vectors/vector3.h`          | `vec3_t`             | Vetor 3D de float                                    |
| `vectors/vector4.h`          | `vec4_t`             | Vetor 4D de float (tints, retângulos de UV)          |
| `colors/color.h`             | `color_t`            | Cor RGBA em float                                    |
| `rects/recti.h`              | `recti_t`            | Retângulo de inteiros (viewports, limites da janela) |
| `matrices/matrix4.h`         | `mat4_t`             | Matriz 4x4 de float (projeção e modelo)              |
| `utils/math_constants.h`     | `math_constants_t`   | `PI`, `HALF_PI`, `TAU`                               |
| `utils/math_converter.h`     | `math_converter_t`   | Conversão entre graus e radianos                     |

Todos os caminhos são relativos a `engine/include/byte-forge-engine/modules/math/`. Inclua a partir da raiz de includes:

```cpp
#include <byte-forge-engine/modules/math/vectors/vector2.h>
```

## Conceitos

- Os tipos são structs simples com campos públicos, então podem ser copiados, comparados e enviados para a GPU como estão.
- Quase tudo é `constexpr`. As exceções são as funções que chamam `<cmath>`: `length()`, `normalized()`, `distance()` e `mat4_t::rotate_z()`.
- As operações nunca alteram o objeto em que são chamadas, exceto os operadores compostos (`+=`, `-=`, `*=`, `/=`). `normalized()`, `translate()`, `scale()` e as demais retornam um valor novo.
- A igualdade usa comparação exata de float (`==`), sem epsilon.
- Os ângulos na API pública são em **graus**. Use `math_converter_t` quando precisar de radianos.
- As matrizes são armazenadas em ordem **column-major** e transformam **vetores coluna**, como o OpenGL espera.
- O espaço de tela tem o **Y apontando para baixo**: `recti_t::top()` é o menor Y, e o sprite renderer monta a projeção com `top = 0` e `bottom = height`.

## Vetores

`vec2_t`, `vec3_t` e `vec4_t` têm o mesmo formato, com um, dois ou três componentes a mais.

| Membro                                  | vec2 | vec3 | vec4 | Observações                                        |
| --------------------------------------- | :--: | :--: | :--: | -------------------------------------------------- |
| Construtor padrão                       |  ✓   |  ✓   |  ✓   | Todos os componentes valem `0`                     |
| `operator[](usize)`                     |  ✓   |  ✓   |  ✓   | Lê ou escreve um componente pelo índice            |
| `+`, `-` (vetor)                        |  ✓   |  ✓   |  ✓   | Componente a componente                            |
| `*`, `/` (escalar)                      |  ✓   |  ✓   |  ✓   | `escalar * vetor` também funciona                  |
| `+=`, `-=`, `*=`, `/=`                  |  ✓   |  ✓   |  ✓   |                                                    |
| `==`                                    |  ✓   |  ✓   |  ✓   | Comparação exata                                   |
| `length()`, `length_squared()`          |  ✓   |  ✓   |  ✓   |                                                    |
| `dot()`                                 |  ✓   |  ✓   |  ✓   |                                                    |
| `cross()`                               |      |  ✓   |      |                                                    |
| `normalized()`                          |  ✓   |  ✓   |  ✓   | Retorna `zero()` para um vetor de comprimento zero |
| `distance()`, `distance_squared()`      |  ✓   |  ✓   |  ✓   |                                                    |
| `zero()`, `one()`                       |  ✓   |  ✓   |  ✓   |                                                    |
| `up()`, `down()`, `left()`, `right()`   |  ✓   |  ✓   |      | `up()` é `+Y`                                      |

Aliases de acesso deixam a intenção mais clara no ponto de uso:

- `vec2_t`: `width()` / `height()`, `r()` / `g()`, `u()` / `v()`
- `vec3_t`: `r()` / `g()` / `b()`
- `vec4_t`: `r()` / `g()` / `b()` / `a()`

### Fórmulas

```
length(a)          = sqrt(a · a)
length_squared(a)  = a · a
a · b              = ax*bx + ay*by (+ az*bz) (+ aw*bw)
a × b              = (ay*bz - az*by, az*bx - ax*bz, ax*by - ay*bx)
normalized(a)      = a / length(a)
distance(a, b)     = length(a - b)
```

O sinal do produto escalar indica o ângulo entre dois vetores: positivo significa menos de 90°, zero significa perpendiculares, negativo significa mais de 90°. O produto vetorial é perpendicular às duas entradas, e seu comprimento é `|a| |b| sin(theta)`.

Prefira `length_squared()` e `distance_squared()` quando só for comparar comprimentos, já que elas evitam a raiz quadrada.

### `vec2i_t`

O vetor de inteiros é menor de propósito. Ele tem `x`, `y`, `width()`, `height()`, `operator[]`, `+`, `-`, `* i32`, `+=`, `-=`, `==`, `!=`, `zero()`, `one()` e `is_positive()` (verdadeiro quando os dois componentes são maiores que zero). Não tem divisão, comprimento nem normalização.

## `color_t`

Uma cor RGBA com canais `f32`, normalmente no intervalo `[0, 1]`. O construtor padrão dá preto opaco, e o argumento de alpha vale `1` por padrão.

- Operadores: `+`, `* f32`, `f32 * cor`, `+=`, `*=`, `==`.
- Predefinidas: `black()`, `white()`, `red()`, `green()`, `blue()`, `yellow()`, `cyan()`, `magenta()`, `transparent()`.
- Os canais não são limitados (clamp). Somar duas cores ou escalar por mais de `1` pode passar de `1`.
- Escalar também afeta o alpha. `cor * 0.5f` reduz a opacidade pela metade.

## `recti_t`

Um retângulo de inteiros formado por uma `position` (canto superior esquerdo) e um `size`, ambos `vec2i_t`.

```cpp
const math::recti_t bounds(0, 0, 1600, 900);
```

| Membro        | Resultado                                     |
| ------------- | --------------------------------------------- |
| `left()`      | `position.x`                                  |
| `top()`       | `position.y`                                  |
| `right()`     | `position.x + size.width()`                   |
| `bottom()`    | `position.y + size.height()`                  |
| `is_empty()`  | `true` a menos que os dois tamanhos sejam > 0 |
| `contains(p)` | Teste semiaberto, veja abaixo                 |
| `==`          | Mesma posição e mesmo tamanho                 |
| `from_size(s)`| Estático: retângulo em `(0, 0)` cobrindo `s`, como um viewport que preenche uma janela ou framebuffer |

`contains()` inclui as bordas esquerda e superior e exclui a direita e a inferior (`left <= x < right`, `top <= y < bottom`). Um retângulo de 1600x900 na origem contém `(0, 0)` e `(1599, 899)`, mas não `(1600, 900)`.

## `mat4_t`

Uma matriz 4x4 armazenada como `f32 values[16]` em ordem column-major, onde `index = column * 4 + row`. O construtor padrão dá uma matriz toda zerada, então comece por `identity()`.

| Membro                                  | Descrição                                            |
| --------------------------------------- | ---------------------------------------------------- |
| `identity()`                            | Diagonal de uns nos índices 0, 5, 10 e 15            |
| `orthographic(l, r, b, t, near, far)`   | Mapeia a caixa para o intervalo de clip `[-1, 1]`    |
| `translate(x, y, z)` / `translate(vec3)`| Retorna `this * T`                                   |
| `scale(x, y, z)`                        | Retorna `this * S`                                   |
| `rotate_z(degrees)`                     | Retorna `this * Rz`, girando no plano XY             |
| `multiply(right)`                       | Retorna `this * right`                               |

### Ordem de composição

Cada chamada de transformação multiplica à **direita** da matriz atual. Com vetores coluna, a matriz mais à direita é aplicada primeiro, então **a última chamada é a primeira a ser aplicada ao vértice**. O sprite renderer monta a matriz de modelo assim:

```cpp
const math::mat4_t model = math::mat4_t::identity()
    .translate(position.x, position.y, 0.0f)
    .rotate_z(rotation_degrees)
    .scale(size.x, size.y, 1.0f);
```

Isso é `T * R * S`: o quad é escalado primeiro, depois girado em torno do próprio centro, depois movido para `position`.

### Projeção ortográfica

```
|  2/(r-l)        0               0               0 |
|  0              2/(t-b)         0               0 |
|  0              0              -2/(f-n)         0 |
| -(r+l)/(r-l)   -(t+b)/(t-b)    -(f+n)/(f-n)     1 |
```

A matriz acima está na ordem da memória (cada linha impressa é uma coluna). X e Y são escalados para que o intervalo tenha tamanho `2` e depois deslocados para que o centro caia na origem. A escala negativa em Z inverte o eixo Z. Passar `top = 0` e `bottom = height` dá coordenadas de tela com Y para baixo.

### Rotação em torno de Z

```
x' = x cos(theta) - y sin(theta)
y' = x sin(theta) + y cos(theta)
```

Ângulos positivos giram de +X em direção a +Y. Com o Y apontando para baixo na tela, isso aparece como sentido horário.

## Constantes e conversão

```cpp
math_constants_t::PI       // 3.14159265...
math_constants_t::HALF_PI  // PI / 2
math_constants_t::TAU      // PI * 2

math_converter_t::deg_to_rad(180.0f)                 // PI
math_converter_t::rad_to_deg(math_constants_t::PI)   // 180
```

## Uso

Todos os tipos são header-only, então usar um é só incluir o header dele. Nada precisa ser inicializado ou descartado.

| Tipo      | Principais usuários                                                         |
| --------- | --------------------------------------------------------------------------- |
| `vec2_t`  | Componentes 2D (transform, velocity, collider, sprite), física, input, bindings de Lua |
| `vec2i_t` | Tamanhos de janela, viewports, `recti_t`                                    |
| `vec4_t`  | Tints e retângulos de UV dos sprites, uniforms de shader                    |
| `vec3_t`  | `mat4_t::translate(vec3_t)`                                                 |
| `recti_t` | Viewport do renderer, limites da janela do editor                           |
| `mat4_t`  | Matrizes de projeção e de modelo do sprite renderer, uniforms de shader     |

## Observações

- O `color_t` ainda não é usado em nenhum lugar da engine. Os tints são passados como `vec4_t`.
- O `normalized()` de um vetor de comprimento zero retorna `zero()` em vez de dividir por zero.
- O `recti_t::contains()` trata as bordas direita e inferior como fora do retângulo.
- O `mat4_t()` é todo zerado, não é a identidade.
- A igualdade de float é exata, então dois vetores calculados de formas diferentes podem não ser iguais.
