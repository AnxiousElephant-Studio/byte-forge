# Módulo de tempo

> Idioma: [English](../../../../en/engine/modules/time/README.md) | **Português (Brasil)**

O módulo de tempo mede frames e trechos de código. Ele tem duas partes: um frame manager que calcula o `delta_time` e limita a taxa de frames usando o relógio em milissegundos do SDL, e um pequeno profiler que cronometra trechos nomeados com `std::chrono::steady_clock` e mantém uma média móvel. Fica no namespace `byteforge::time`. `frame_settings_t`, `time_converter_t`, `profiler_sample_t` e `scoped_profile_t` são header-only; `frame_manager_t` e `profiler_t` têm arquivos de código-fonte.

## Arquivos

| Arquivo                                                            | Conteúdo                                          |
| ------------------------------------------------------------------ | ------------------------------------------------- |
| `engine/include/byte-forge-engine/modules/time/frame_settings.h`   | `frame_settings_t`, a taxa de frames alvo         |
| `engine/include/byte-forge-engine/modules/time/frame_manager.h`    | Declaração do `frame_manager_t`                   |
| `engine/include/byte-forge-engine/modules/time/time_converter.h`   | `time_converter_t`, segundos e milissegundos      |
| `engine/include/byte-forge-engine/modules/time/profiler.h`         | `profiler_sample_t`, `profiler_t`, `scoped_profile_t` |
| `engine/source/modules/time/frame_manager.cpp`                     | Delta time, limite de frames, tempo decorrido     |
| `engine/source/modules/time/profiler.cpp`                          | Busca de amostras e cálculo da média              |

Peças relacionadas em outros módulos:

| Arquivo                                                               | Papel                                               |
| --------------------------------------------------------------------- | --------------------------------------------------- |
| `modules/runtime/runtime.h`                                           | Dono de `frame_settings` e `frame_manager`          |
| `modules/scene/scene_player.cpp`                                      | Mede `physics`, `collisions` e `scripts`            |
| `applications/editor/source/editor.cpp`                               | Dono do profiler, conduz o frame manager            |
| `applications/editor/source/ui/components/status_panel_component.cpp` | Mostra FPS, tempo de frame e as fases do profiler   |

## Conceitos

### Dois relógios

O frame manager lê `SDL_GetTicks()`, que retorna milissegundos inteiros desde que o SDL foi inicializado. Por isso todo valor que ele calcula é múltiplo de um milissegundo antes da conversão. O profiler lê `std::chrono::steady_clock`, que tem resolução abaixo de um milissegundo, e reporta milissegundos como `f64`.

### O que o `delta_time` mede

`delta_time` é o tempo entre o fim de dois frames consecutivos, em segundos:

```
frame N-1                       frame N
start ... end | delay |         start ... end | delay |
              ^------------------------------^
                     delta_time after frame N
```

`end_frame()` pega a contagem de ticks antes de dormir, então o intervalo medido cobre a espera do frame anterior e o trabalho do frame atual. O valor é escrito no fim de um frame e lido pelo update do frame seguinte, então o update sempre usa a duração do frame anterior a ele.

### Limite de frames

O frame manager limita a taxa de frames dormindo. Se o trabalho do próprio frame (de `start_frame()` até `end_frame()`) levou menos que o tempo alvo de frame, `end_frame()` chama `SDL_Delay` pela diferença. Um frame que levou mais não dorme e não é compensado depois.

## `frame_settings_t`

```cpp
struct frame_settings_t
{
public:
    u8 target_fps;

    explicit frame_settings_t(u8 target_fps) : target_fps(target_fps) {}
};
```

### Estado

| Campo        | Significado                                                              |
| ------------ | ------------------------------------------------------------------------ |
| `target_fps` | Frames por segundo que o frame manager busca. Um `u8`, então no máximo `255` |

## `frame_manager_t`

```cpp
struct BF_TIME_API frame_manager_t
{
public:
    f64 delta_time;

    void reset(const frame_settings_t& frame_settings);
    void start_frame();
    void end_frame();
    f64 get_elapsed_time() const;

private:
    u64 first_frame_tick_count;
    u64 last_frame_tick_count;
    u64 current_frame_start_tick_count;
    u64 current_frame_end_tick_count;
    f64 target_frame_time_in_ms;
};
```

O construtor coloca todos os campos em zero.

### Estado

| Campo                            | Significado                                                     |
| -------------------------------- | --------------------------------------------------------------- |
| `delta_time`                     | Segundos entre o fim dos dois últimos frames, limitado a `0.05` |
| `first_frame_tick_count`         | Contagem de ticks quando `reset()` rodou. Origem do tempo decorrido |
| `last_frame_tick_count`          | Contagem de ticks no fim do frame anterior                      |
| `current_frame_start_tick_count` | Contagem de ticks quando o frame atual começou                  |
| `current_frame_end_tick_count`   | Contagem de ticks no fim do último frame completo               |
| `target_frame_time_in_ms`        | `1000 / target_fps`, o orçamento do frame em milissegundos      |

### `reset(frame_settings)`

Lê a contagem de ticks atual para `first_frame_tick_count` e `last_frame_tick_count`, zera as contagens de início e fim e o `delta_time`, e coloca `target_frame_time_in_ms` em `sec_to_ms(1.0 / target_fps)`. `runtime_t::initialize()` chama ele como último passo.

### `start_frame()`

Guarda a contagem de ticks atual como o início do frame. Chame no topo do loop, antes de qualquer trabalho que deva contar para o orçamento do frame.

### `end_frame()`

1. Lê a contagem de ticks atual como o fim do frame.
2. Coloca em `delta_time` o tempo desde o fim do frame anterior, convertido para segundos e limitado a `0.05` (a constante `max_delta_time`).
3. Guarda o fim como o fim do frame anterior para a próxima chamada.
4. Calcula a duração do próprio frame (fim menos início) em milissegundos. Se for menor que `target_frame_time_in_ms`, dorme com `SDL_Delay` pela diferença, convertida para `u32`, o que descarta a parte fracionária.

### `get_elapsed_time()`

Retorna os segundos entre `reset()` e o fim do último frame completo. Ele não lê o relógio, então só avança quando `end_frame()` roda.

## `time_converter_t`

```cpp
struct time_converter_t
{
public:
    constexpr static f64 MS_PER_SEC = 1000.0;

    constexpr static f64 sec_to_ms(const f64 secs);
    constexpr static f64 ms_to_sec(const f64 ms);
};
```

| Membro         | Retorna                     |
| -------------- | --------------------------- |
| `MS_PER_SEC`   | `1000.0`                    |
| `sec_to_ms(s)` | `s * 1000.0`                |
| `ms_to_sec(m)` | `m / 1000.0`                |

As duas funções são `constexpr` e podem ser usadas em expressões constantes.

## `profiler_sample_t`

```cpp
struct profiler_sample_t
{
public:
    std::string name;
    std::chrono::steady_clock::time_point start;
    f64 last_ms;
    f64 average_ms;
};
```

### Estado

| Campo        | Significado                                                            |
| ------------ | ---------------------------------------------------------------------- |
| `name`       | O nome do trecho, como passado para `begin()` ou `end()`               |
| `start`      | Quando o último `begin()` para esse nome rodou                         |
| `last_ms`    | Duração do último intervalo de `begin()` até `end()`, em milissegundos |
| `average_ms` | Média móvel exponencial de `last_ms`                                   |

## `profiler_t`

```cpp
class BF_TIME_API profiler_t
{
public:
    void begin(const char* name);
    void end(const char* name);
    usize get_sample_count() const;
    const profiler_sample_t& get_sample(const usize index) const;
    const profiler_sample_t* find(const char* name) const;

private:
    profiler_sample_t* find_or_create(const char* name);

    std::vector<profiler_sample_t> samples;
};
```

### Estado

| Campo     | Significado                                                             |
| --------- | ----------------------------------------------------------------------- |
| `samples` | Uma amostra por nome distinto, na ordem em que cada nome apareceu       |

### `begin(name)`

Encontra a amostra chamada `name`, criando se não existir, e coloca o `start` dela em agora.

### `end(name)`

1. Encontra a amostra chamada `name`, criando se não existir.
2. Coloca em `last_ms` o tempo desde o `start` dela, em milissegundos.
3. Atualiza `average_ms`. Se ainda for `0.0` (a primeira medição), vira `last_ms`. Senão vira `average_ms * 0.9 + last_ms * 0.1`, então cada valor novo move a média em 10 por cento.

### `find(name)`

Retorna um ponteiro para a amostra chamada `name`, ou `nullptr`. Nunca cria uma. O ponteiro aponta para dentro do vetor e é invalidado quando um nome novo é adicionado.

### `get_sample_count()` e `get_sample(index)`

O número de amostras e a amostra em `index`. `get_sample` não verifica o índice.

### `find_or_create(name)` (privado)

Busca no vetor comparando o `name` de cada amostra com `name`. Se nenhuma bater, adiciona uma amostra com `start` em agora e os dois tempos em `0.0`. A busca é linear, o que é suficiente para os poucos nomes que o editor usa.

## `scoped_profile_t`

```cpp
struct scoped_profile_t
{
public:
    scoped_profile_t(profiler_t* profiler, const char* name);
    ~scoped_profile_t();

    scoped_profile_t(const scoped_profile_t&)            = delete;
    scoped_profile_t& operator=(const scoped_profile_t&) = delete;

private:
    profiler_t* profiler;
    const char* name;
};
```

Chama `profiler->begin(name)` no construtor e `profiler->end(name)` no destrutor, então mede o escopo em que vive. Se `profiler` for `nullptr`, os dois não fazem nada. Isso deixa código da engine, como o scene player, medir seus trechos só quando uma aplicação entregou um profiler para ele. Ele guarda o ponteiro `name`, então a string precisa viver mais que o objeto.

## Uso

`runtime_t` é dono de `frame_settings` e `frame_manager`, e faz o reset do frame manager no fim de `initialize()`. O editor conduz o frame manager e é dono de um `profiler_t`:

```cpp
while (!runtime.should_close)
{
    profiler.begin(frame_profile_name);
    runtime.frame_manager.start_frame();

    {
        time::scoped_profile_t update_profile(&profiler, update_profile_name);
        scene_player.update(runtime.frame_manager.delta_time);
        runtime.audio.update();
    }

    // events, scene render and ui are profiled the same way

    runtime.frame_manager.end_frame();
    profiler.end(frame_profile_name);
}
```

O editor faz `scene_player.profiler = &profiler`, e o scene player então adiciona as amostras `physics`, `collisions` e `scripts` enquanto uma cena está tocando. As amostras do próprio editor são `frame`, `events`, `update`, `scene render` e `ui`.

O painel de status lê o profiler:

- O FPS é `1000 / average_ms` da amostra `frame`, mostrado ao lado de `frame_settings.target_fps`.
- "Frame time" é o `last_ms` da amostra `frame`, e os últimos 120 valores são plotados.
- "Work" é a soma do `last_ms` de todas as amostras exceto `frame`, e "waiting" é o tempo de frame menos essa soma.
- A tabela "Phases" lista todas as amostras exceto `frame` com seus tempos último e médio.

## Notas

- O editor declara seu alvo como `const u16 target_fps = 500` e passa para `frame_settings_t`, que recebe um `u8`. A conversão mantém os 8 bits baixos, então o alvo guardado é `244`, e é esse o valor que o painel de status mostra.
- Um `target_fps` igual a `0` torna `1.0 / target_fps` infinito, e a espera em `end_frame()` é então convertida a partir de um valor infinito.
- Como os ticks são milissegundos inteiros e a espera descarta frações, em alvos altos o tempo real de frame arredonda para milissegundos inteiros. Com o `244` do editor, o orçamento é de cerca de `4.1` ms e a espera é no máximo `4` ms.
- `delta_time` é `0.0` durante o primeiro frame depois de `reset()`. O primeiro valor diferente de zero inclui tudo o que rodou entre `reset()` e o primeiro `end_frame()`, limitado a `0.05` segundo.
- O limite faz com que um frame lento (acima de 50 ms) avance a simulação menos que o tempo real.
- `get_elapsed_time()` subtrai `first_frame_tick_count` de `current_frame_end_tick_count` como inteiros sem sinal. Antes do primeiro `end_frame()`, a contagem de fim é `0`, então a subtração dá a volta e o resultado é um número muito grande.
- As amostras do profiler nunca são removidas. Um nome que deixa de ser medido mantém seus últimos valores; por exemplo, as amostras do scene player continuam na soma "Work" do painel de status depois do Stop.
- As amostras do scene player rodam dentro da amostra `update` do editor, então a soma "Work" do painel de status conta esse tempo duas vezes enquanto uma cena está tocando.
- O profiler não é thread-safe e guarda um `start` por nome, então o mesmo nome não pode ser aninhado nem sobreposto.
