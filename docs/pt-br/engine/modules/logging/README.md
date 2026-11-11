# Módulo de logging

> Idioma: [English](../../../../en/engine/modules/logging/README.md) | **Português (Brasil)**

O módulo de logging formata mensagens de log, guarda as mais recentes na memória e imprime elas no terminal com cores. Ele é um `logger_t` estático com um conjunto de macros `LOG_*` por cima, um ring buffer (`log_buffer_t`) que o console do editor lê, e um callback de sink opcional. Fica no namespace `byteforge::logging` e usa só as bibliotecas padrão de C e C++.

## Arquivos

| Arquivo                                                              | Conteúdo                                              |
| -------------------------------------------------------------------- | ----------------------------------------------------- |
| `engine/include/byte-forge-engine/modules/logging/e_log_levels.h`    | `e_log_levels`, os sete níveis                        |
| `engine/include/byte-forge-engine/modules/logging/log_entry.h`       | `log_entry_t`, uma linha formatada e o seu nível      |
| `engine/include/byte-forge-engine/modules/logging/log_sink.h`        | `log_sink_t`, o tipo do callback                      |
| `engine/include/byte-forge-engine/modules/logging/log_buffer.h`      | Declaração do `log_buffer_t`                          |
| `engine/include/byte-forge-engine/modules/logging/logger.h`          | Declaração do `logger_t`                              |
| `engine/include/byte-forge-engine/modules/logging/log_macros.h`      | Macros `LOG_*` e `extract_file_name`                  |
| `engine/source/modules/logging/logger.cpp`                           | Estado global do logger, formatação e saída           |
| `engine/source/modules/logging/log_buffer.cpp`                       | Implementação do ring buffer                          |
| `engine/source/modules/logging/log_level_info.h` / `.cpp`            | Privado: nomes dos níveis e cores do terminal         |
| `engine/source/modules/logging/log_styles.h`                         | Privado: códigos de escape ANSI                       |

Os três últimos arquivos ficam em `engine/source`, então as aplicações não conseguem incluí-los.

Peças relacionadas em outros módulos:

| Arquivo                                                                 | Papel                                                |
| ----------------------------------------------------------------------- | ---------------------------------------------------- |
| `applications/editor/source/ui/components/console_output_component.cpp` | Mostra o buffer, muda o nível mínimo               |
| `modules/scene/scene_script_system.cpp`                                 | `log.info`, `log.warning` e `log.error` em Lua       |

## Conceitos

### Um logger global

Todo o estado do logger vive num namespace anônimo dentro de `logger.cpp`, que é compilado na DLL da engine: o buffer, o sink, o nível mínimo e a flag de console. `logger_t` só tem funções estáticas que leem e alteram esse estado. A engine e toda aplicação que linka com ela compartilham o mesmo estado, então uma mensagem logada pela engine aparece no console do editor.

### Níveis

```cpp
enum class e_log_levels : u8
{
    Info    = 0,
    Debug   = 1,
    Trace   = 2,
    Success = 3,
    Warning = 4,
    Error   = 5,
    Fatal   = 6
};
```

Uma mensagem só é mantida se o seu nível for maior ou igual ao nível mínimo, que começa em `Info`. Como `Info` é `0`, o padrão deixa toda mensagem passar. Subir o mínimo esconde os níveis abaixo dele nesta ordem, então colocar em `Debug` esconde `Info` mas mantém `Trace` e `Success`.

### Formato da linha

Toda mensagem vira uma linha:

```
[24-10-2026 14:03:27] [ERROR] [scripting.cpp:50:load_file] Could not run script 'a.lua': ...
[24-10-2026 14:03:27] [INFO] [lua] hello from on_start
```

A data é a hora local no formato `dia-mês-ano hora:minuto:segundo`. A localização entre colchetes (nome do arquivo, linha, função) só aparece em mensagens logadas pelo caminho detalhado, que as macros `LOG_FATAL`, `LOG_ERROR`, `LOG_WARNING` e `LOG_TRACE` usam. Se a hora local não puder ser lida, a data é substituída por `unknown-datetime`.

### Para onde uma linha vai

Para cada mensagem que passa na verificação de nível, `write()` em `logger.cpp` faz três coisas, em ordem:

1. Adiciona ao buffer um `log_entry_t` com o nível e a linha.
2. Chama o sink, se houver um, com essa entrada.
3. Se a saída de console estiver ligada, imprime a linha em `stderr` para `Warning`, `Error` e `Fatal`, ou em `stdout` para os outros, envolvida num código de cor ANSI e num código de reset.

Tudo roda de forma síncrona na thread que chamou, e não há locking.

## `log_entry_t`

```cpp
struct log_entry_t
{
public:
    e_log_levels level;
    std::string line;
};
```

### Estado

| Campo   | Significado                                                         |
| ------- | ------------------------------------------------------------------- |
| `level` | O nível em que a mensagem foi logada                                |
| `line`  | A linha formatada completa, com data, nome do nível e localização, sem códigos de cor nem quebra de linha |

## `log_sink_t`

```cpp
using log_sink_t = void (*)(const log_entry_t& entry);
```

Um ponteiro de função simples. Ele recebe a mesma entrada que acabou de ser adicionada ao buffer. A referência só vale durante a chamada, então um sink que queira guardar a linha precisa copiá-la. Como é um ponteiro de função e não um `std::function`, ele não pode capturar estado.

## `log_buffer_t`

```cpp
class BF_LOGGING_API log_buffer_t
{
public:
    void add(const log_entry_t& entry);
    void clear();
    usize get_count() const;
    const log_entry_t& get(const usize index) const;
    u64 get_total_count() const;

private:
    static constexpr usize DEFAULT_CAPACITY = 2000;

    std::vector<log_entry_t> entries;
    usize capacity;
    usize oldest_index;
    u64 total_count;
};
```

Um ring buffer de capacidade fixa. Ele guarda até 2000 entradas; depois disso, cada entrada nova sobrescreve a mais antiga.

### Estado

| Campo          | Significado                                                              |
| -------------- | ------------------------------------------------------------------------ |
| `entries`      | Entradas guardadas. Cresce até `capacity` e depois é reaproveitado no lugar |
| `capacity`     | Número máximo de entradas. Sempre `DEFAULT_CAPACITY` (`2000`)            |
| `oldest_index` | Posição da entrada mais antiga dentro de `entries` quando o buffer está cheio |
| `total_count`  | Número de entradas já adicionadas, incluindo as já sobrescritas         |

### `add(entry)`

Incrementa `total_count`. Enquanto `entries` estiver abaixo da capacidade, adiciona uma cópia de `entry` no fim. Quando cheio, escreve a cópia sobre `entries[oldest_index]` e avança `oldest_index` em um, voltando a `0` no final.

### `get(index)`

Retorna a entrada em `index` contando a partir da mais antiga: `0` é a entrada mais antiga mantida e `get_count() - 1` é a mais nova. Internamente lê `entries[(oldest_index + index) % entries.size()]`. O índice não é verificado.

### `get_count()` e `get_total_count()`

`get_count()` é o número de entradas mantidas no momento, no máximo 2000. `get_total_count()` é o número já adicionado.

### `clear()`

Remove todas as entradas e volta `oldest_index` para `0`. `total_count` não é zerado.

## `logger_t`

```cpp
struct BF_LOGGING_API logger_t
{
    static void set_sink(log_sink_t sink);
    static void set_min_level(e_log_levels level);
    static e_log_levels get_min_level();
    static void set_console_output(bool enabled);
    static log_buffer_t& get_buffer();

    static void standard_log(e_log_levels level, const char* format, ...);
    static void detailed_log(e_log_levels level, const char* file_name, i32 code_line, const char* function_name, const char* format, ...);
};
```

### Estado

O estado não faz parte da struct; ele fica no nível do arquivo em `logger.cpp`:

| Variável                 | Valor inicial | Significado                                |
| ------------------------ | ------------- | ------------------------------------------ |
| `buffer`                 | vazio         | O único `log_buffer_t`                     |
| `current_sink`           | `nullptr`     | O sink, ou nenhum                          |
| `min_level`              | `Info`        | Mensagens abaixo deste nível são descartadas |
| `console_output_enabled` | `true`        | Se as linhas são impressas no terminal     |

### `set_sink(sink)`

Substitui o sink. Só existe um sink por vez; passar `nullptr` remove ele.

### `set_min_level(level)` e `get_min_level()`

Definem e leem o nível mínimo. Uma mensagem abaixo dele é descartada antes de qualquer formatação, então não chega ao buffer, nem ao sink, nem ao terminal.

### `set_console_output(enabled)`

Liga ou desliga a impressão no terminal. O buffer e o sink continuam recebendo entradas nos dois casos.

### `get_buffer()`

Retorna uma referência para o buffer global. O console do editor lê ele a cada frame e limpa com o botão Clear.

### `standard_log(level, format, ...)`

1. Retorna se `level` estiver abaixo do mínimo.
2. Formata `format` e os argumentos com `vsnprintf` num buffer de 1024 bytes. Mensagens maiores são cortadas.
3. Escreve a linha com localização vazia.

### `detailed_log(level, file_name, code_line, function_name, format, ...)`

Igual a `standard_log`, mas também formata ` [file_name:code_line:function_name]` num buffer de 256 bytes e coloca entre o nome do nível e a mensagem. A linha final é montada num buffer de 1400 bytes.

## Macros

```cpp
#define LOG_FATAL(...)   ... detailed_log(e_log_levels::Fatal,   BF_LOG_FILE_NAME, __LINE__, __func__, __VA_ARGS__)
#define LOG_ERROR(...)   ... detailed_log(e_log_levels::Error,   ...)
#define LOG_WARNING(...) ... detailed_log(e_log_levels::Warning, ...)
#define LOG_TRACE(...)   ... detailed_log(e_log_levels::Trace,   ...)

#define LOG_SUCCESS(...) ... standard_log(e_log_levels::Success, __VA_ARGS__)
#define LOG_DEBUG(...)   ... standard_log(e_log_levels::Debug,   __VA_ARGS__)
#define LOG_INFO(...)    ... standard_log(e_log_levels::Info,    __VA_ARGS__)
```

Os argumentos são uma string de formato `printf` seguida dos seus valores. `BF_LOG_FILE_NAME` chama `extract_file_name(__FILE__)`, uma função `constexpr` que retorna um ponteiro para a parte depois do último `/` ou `\`, então a localização mostra `scripting.cpp` em vez do caminho completo. `__func__` é o nome puro da função, sem classe nem namespace.

## Helpers privados

`log_level_info_t` mapeia cada nível para o seu nome e cor de terminal, e `log_style_t` guarda os códigos de escape:

| Nível     | Nome      | Estilo no terminal   | Stream   |
| --------- | --------- | -------------------- | -------- |
| `Info`    | `INFO`    | texto branco claro   | `stdout` |
| `Debug`   | `DEBUG`   | texto ciano          | `stdout` |
| `Trace`   | `TRACE`   | texto azul           | `stdout` |
| `Success` | `SUCCESS` | texto verde          | `stdout` |
| `Warning` | `WARNING` | texto amarelo        | `stderr` |
| `Error`   | `ERROR`   | texto vermelho       | `stderr` |
| `Fatal`   | `FATAL`   | fundo vermelho       | `stderr` |

Um valor fora do enum recebe o nome `UNKNOWN` e o código de reset.

## Uso

O código da engine e do editor inclui `log_macros.h` e loga com strings de formato guardadas como constantes nomeadas:

```cpp
const char* const file_error_format = "Could not run script '%s': %s";

LOG_ERROR(file_error_format, file_path, get_error_message(lua_state));
```

O painel Console Output do editor lê `logger_t::get_buffer()` diretamente, então não precisa de sink e também mostra entradas logadas antes da janela do editor existir. Os controles dele se relacionam com o logger assim:

| Controle    | Efeito                                                                   |
| ----------- | ------------------------------------------------------------------------ |
| Capture     | Chama `set_min_level()`. Muda o que o logger mantém, globalmente         |
| Show        | Filtra as linhas exibidas por nível. O buffer não muda                   |
| Search      | Filtro de substring sem diferenciar maiúsculas em `line`                 |
| Clear       | Chama `clear()` no buffer                                                |
| Auto-scroll | Rola até o fim quando `get_total_count()` mudou desde o último frame     |

A linha de resumo mostra as contagens de exibidas, mantidas e total, mais o número de warnings e de erros (`Error` e `Fatal` juntos) entre as entradas mantidas.

Scripts Lua logam pela tabela `log`, que o módulo de cena registra (veja o [módulo de cena](../scene/README.md)). `log.info`, `log.warning` e `log.error` chamam `LOG_INFO`, `LOG_WARNING` e `LOG_ERROR` com o formato `[lua] %s`.

## Notas

- `LOG_FATAL` só loga. Ele não aborta nem encerra; quem chama decide o que acontece depois.
- A ordem dos níveis não é uma ordem de verbosidade. `Info` é o mais baixo, então não existe nível mínimo que mostre `Info` escondendo `Debug` e `Trace`.
- Mensagens do `log.warning` e do `log.error` do Lua passam pelo caminho detalhado, então a localização delas é o binding C++ em `scene_script_system.cpp`, não a linha no script Lua.
- As mensagens são cortadas em 1023 caracteres, e a linha inteira, incluindo data e localização, em 1399.
- As funções de formatação são funções C variádicas sem verificação de formato, então uma diferença entre a string de formato e os argumentos não é pega em tempo de compilação.
- Os códigos de cor ANSI são sempre escritos quando a saída de console está ligada. Se aparecem como cores depende do terminal.
- Nenhum código define um sink ou desliga a saída de console hoje. O console do editor funciona só pelo buffer.
- `log_buffer_t::get()` num buffer vazio divide por zero. Quem chama faz o loop até `get_count()` primeiro.
