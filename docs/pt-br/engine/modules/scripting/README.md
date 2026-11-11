# Módulo de scripting

> Idioma: [English](../../../../en/engine/modules/scripting/README.md) | **Português (Brasil)**

O módulo de scripting é dono do interpretador Lua da engine. Ele é uma classe, `scripting_t`, que cria um único `lua_State`, abre as bibliotecas padrão e oferece alguns helpers para rodar um arquivo, chamar uma função global e ler campos de uma tabela global. Fica no namespace `byteforge::scripting` e encapsula a API C do [Lua](https://www.lua.org).

As funções que os scripts chamam (`transform`, `velocity`, `label`, `audio`, `log`, `input`) e os environments de script por entidade não fazem parte deste módulo. Elas são registradas pelo sistema de scripts da cena sobre o state criado aqui e estão documentadas no [módulo de cena](../scene/README.md).

## Arquivos

| Arquivo                                                           | Conteúdo                                   |
| ----------------------------------------------------------------- | ------------------------------------------ |
| `engine/include/byte-forge-engine/modules/scripting/scripting.h`  | Declaração do `scripting_t`                |
| `engine/source/modules/scripting/scripting.cpp`                   | Implementação sobre a API C do Lua         |

Peças relacionadas em outros módulos:

| Arquivo                                                           | Papel                                                     |
| ----------------------------------------------------------------- | --------------------------------------------------------- |
| `modules/runtime/runtime.h`                                       | Dono da instância de `scripting_t` e do seu ciclo de vida |
| `modules/scene/scene_script_system.cpp`                           | Registra a API Lua e carrega os scripts das entidades     |
| `applications/editor/source/ui/components/status_panel_component.cpp` | Mostra `get_memory_bytes()` como "Lua memory"         |

## Conceitos

### Como o Lua fica escondido

O header só faz forward declaration do tipo do state do Lua:

```cpp
struct lua_State;
```

O `scripting_t` guarda um ponteiro para ele, então `scripting.h` não inclui nenhum header do Lua. Só `scripting.cpp` inclui `<lua.hpp>`. O próprio Lua vem do pacote xmake `lua`, que o `engine/xmake.lua` requer e linka na engine.

### Um state compartilhado

Existe exatamente um `lua_State` por `scripting_t`, e o runtime é dono de exatamente um `scripting_t`. Todo script da engine roda nesse state. `get_lua_state()` entrega o ponteiro cru, e o sistema de scripts da cena usa ele para registrar suas tabelas globais e dar a cada script de entidade sua própria tabela de environment. Como o state é compartilhado, os globais definidos por `load_file()` e as tabelas registradas pela cena vivem na mesma tabela global (`_G`).

### A pilha do Lua

Todos os helpers trabalham na pilha do Lua. `stack_top` (`-1`) é o índice do valor no topo. Cada helper deixa a pilha como encontrou, exceto `push_table()`, que em caso de sucesso deixa a tabela no topo até `pop_table()` removê-la.

## `scripting_t`

```cpp
class BF_SCRIPTING_API scripting_t
{
public:
    scripting_t() : lua_state(nullptr) {}

    bool initialize();
    bool load_file(const char* file_path);
    bool call(const char* function_name);

    bool push_table(const char* table_name);
    void pop_table();
    bool get_integer(const char* field, i32& out_integer);
    bool get_number(const char* field, f64& out_number);
    bool get_string(const char* field, char* out_buffer, usize buffer_size);

    void dispose();

    lua_State* get_lua_state() const { return lua_state; }
    usize get_memory_bytes() const;

private:
    lua_State* lua_state;
};
```

Não há destrutor. O state só é fechado em `dispose()`, que o dono precisa chamar.

### Estado

| Campo       | Significado                                                             |
| ----------- | ----------------------------------------------------------------------- |
| `lua_state` | O interpretador Lua. `nullptr` antes de `initialize()` e depois de `dispose()` |

### `initialize()`

1. Cria o state com `luaL_newstate()`, que usa o alocador padrão do Lua.
2. Se isso retornar `nullptr`, loga `Could not create the Lua state.` no nível `Fatal` e retorna `false`.
3. Abre todas as bibliotecas padrão com `luaL_openlibs()`. Por isso os scripts têm todas as bibliotecas padrão, como `string`, `table`, `math`, `coroutine`, `io`, `os`, `debug` e `package`.
4. Retorna `true`.

### `load_file(file_path)`

Roda um arquivo Lua no environment global com `luaL_dofile`, que carrega o chunk e chama ele na hora. O código de nível superior do arquivo executa, e quaisquer funções ou variáveis globais que ele defina ficam visíveis para `call()` e `push_table()`.

Se o carregamento ou a execução falhar (arquivo ausente, erro de sintaxe, erro em tempo de execução), loga `Could not run script '<path>': <message>` no nível `Error`, tira a mensagem de erro da pilha e retorna `false`. `<message>` é o texto que o Lua colocou na pilha, ou `unknown error` se não for uma string.

`file_path` é aberto como foi passado, então um caminho relativo é relativo ao diretório de trabalho do processo.

### `call(function_name)`

1. Empilha o global chamado `function_name`.
2. Se não for uma função, loga `Lua function '<name>' does not exist.`, desempilha e retorna `false`.
3. Chama com `lua_pcall` sem argumentos, sem resultados e sem message handler.
4. Se a chamada levantar um erro, loga `Lua function '<name>' failed: <message>`, desempilha a mensagem e retorna `false`.

Sem message handler a mensagem logada não tem traceback, só o texto do erro.

### `push_table(table_name)`

Empilha o global chamado `table_name`. Se for uma tabela, ela fica no topo da pilha e a função retorna `true`. Se for qualquer outra coisa (incluindo `nil`), é desempilhada e a função retorna `false`. Nada é logado.

### `pop_table()`

Desempilha o topo da pilha, mas só se for uma tabela. Chamar depois de um `push_table()` que falhou não faz nada.

### `get_integer(field, out_integer)`, `get_number(field, out_number)`, `get_string(field, out_buffer, buffer_size)`

Os três getters leem `field` da tabela no topo da pilha, então devem ser chamados entre `push_table()` e `pop_table()`. Cada um empilha o campo, verifica o tipo, escreve a saída só se o tipo bater, desempilha o campo e retorna se bateu. Se não bater, a saída fica intacta.

| Getter        | Aceita                                            | Conversão                                     |
| ------------- | ------------------------------------------------- | --------------------------------------------- |
| `get_integer` | Valores com o subtipo inteiro do Lua (`lua_isinteger`) | Cast para `i32`                          |
| `get_number`  | Números e strings que convertem para número (`lua_isnumber`) | `lua_tonumber`                     |
| `get_string`  | Só strings de verdade (`LUA_TSTRING`), não números | Copiada com `snprintf` para `out_buffer`     |

`get_integer` rejeita floats, até `3.0`. `get_string` trunca em `buffer_size - 1` caracteres e sempre escreve o terminador quando `buffer_size` não é zero.

### `get_memory_bytes()`

Retorna a memória que o coletor do Lua está rastreando no momento, em bytes. Combina `lua_gc(LUA_GCCOUNT)` (kilobytes inteiros) vezes `1024` com `lua_gc(LUA_GCCOUNTB)` (os bytes restantes). Retorna `0` quando não há state.

### `dispose()`

Fecha o state com `lua_close` e coloca o ponteiro em `nullptr`. Chamar duas vezes, ou antes de `initialize()`, não faz nada.

### `get_lua_state()`

Retorna o `lua_State*` cru, ou `nullptr` se o state não existir. É assim que outros módulos alcançam o Lua diretamente.

## Uso

`runtime_t` tem um membro `scripting::scripting_t scripting`. `runtime_t::initialize()` inicializa ele logo depois da plataforma e antes do áudio, e `runtime_t::dispose()` faz o dispose dele logo depois do áudio. Veja o [módulo de runtime](../runtime/README.md).

O editor nunca chama `load_file()` nem `call()`. Ele passa o state para o scene player, que repassa para o sistema de scripts da cena:

```cpp
if (!scene.initialize() || !scene_player.initialize(runtime.scripting.get_lua_state(), scene, runtime.audio, input_state))
{
    LOG_FATAL(scene_error_format);
    return false;
}
```

A partir daí, o módulo de cena registra suas tabelas e roda o script de cada entidade no seu próprio environment. O painel de status mostra o tamanho do state a cada frame:

```cpp
ImGui::Text(lua_memory_format, static_cast<f32>(context.runtime->scripting.get_memory_bytes()) / bytes_per_kilobyte);
```

Um uso direto dos helpers ficaria assim:

```cpp
scripting.load_file("assets/scripts/config.lua");

i32 width = 0;

if (scripting.push_table("config"))
{
    scripting.get_integer("width", width);
    scripting.pop_table();
}
```

## Notas

- `initialize()` precisa ter sucesso antes de qualquer outro membro ser chamado. `load_file()`, `call()` e os helpers de tabela passam `lua_state` para o Lua sem verificar se é `nullptr`.
- Chamar `initialize()` duas vezes cria um segundo state e sobrescreve o ponteiro do primeiro, que nunca é fechado.
- `load_file()` usa `luaL_dofile`, que mantém todos os valores que o chunk retorna. Esses valores ficam na pilha depois de uma chamada bem-sucedida.
- Os getters leem do que estiver no topo da pilha. Se o valor ali não puder ser indexado, `lua_getfield` levanta um erro Lua fora de qualquer chamada protegida.
- `get_integer` estreita o inteiro de 64 bits do Lua para `i32` com um cast simples, então valores grandes dão a volta.
- `scripting_t` não deleta suas operações de cópia. Uma cópia compartilharia o mesmo ponteiro de `lua_State`.
- Todas as bibliotecas padrão são abertas, incluindo `io` e `os`, então os scripts podem ler e escrever arquivos e rodar comandos do sistema.
