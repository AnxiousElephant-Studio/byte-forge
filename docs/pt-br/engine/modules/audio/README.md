# Módulo de áudio

> Idioma: [English](../../../../en/engine/modules/audio/README.md) | **Português (Brasil)**

O módulo de áudio toca arquivos de som. Ele é uma camada fina sobre o [miniaudio](https://miniaud.io): uma classe, `audio_engine_t`, que carrega um arquivo, inicia a reprodução e devolve um número (um handle) para parar depois. Fica no namespace `byteforge::audio`.

## Arquivos

| Arquivo                                                           | Conteúdo                                         |
| ----------------------------------------------------------------- | ------------------------------------------------ |
| `engine/include/byte-forge-engine/modules/audio/audio_engine.h`   | Declaração do `audio_engine_t`                   |
| `engine/source/modules/audio/audio_engine.cpp`                    | Implementação e a implementação do miniaudio     |

Peças relacionadas em outros módulos:

| Arquivo                                                                   | Papel                                             |
| ------------------------------------------------------------------------- | ------------------------------------------------- |
| `modules/components/audio_source_component.h`                             | Dados que uma entidade precisa para tocar um clip |
| `modules/scene/scene_player.cpp`                                          | Toca as fontes "play on start" e para tudo        |
| `modules/scene/scene_script_system.cpp`                                   | `audio.play` e `audio.stop` em Lua                |
| `modules/runtime/runtime.h`                                               | Dono da instância da engine e do seu ciclo de vida |

## Conceitos

### Como o miniaudio fica escondido

O header só faz forward declaration dos dois tipos do miniaudio que ele guarda:

```cpp
struct ma_engine;
struct ma_sound;
```

O `audio_engine_t` guarda ponteiros para eles, então o header não inclui o `miniaudio.h`. Aplicações que incluem `audio_engine.h` nunca enxergam o miniaudio e não precisam dele no include path.

O miniaudio é uma biblioteca de um header só. O `audio_engine.cpp` define `MINIAUDIO_IMPLEMENTATION` antes de incluí-lo, o que faz esse arquivo compilar a biblioteca inteira dentro da DLL da engine. Nenhum outro arquivo pode definir isso, senão a biblioteca seria compilada duas vezes.

### Handles

Um handle é um `u32` que identifica uma reprodução. O handle `0` significa "inválido" e é o que o `play()` retorna quando falha.

Os handles começam em `1` e só sobem. Um handle nunca é reutilizado, então um handle antigo de um som que já acabou não consegue parar um som mais novo por engano. O `stop()` e o `is_playing()` simplesmente não o encontram.

Cada chamada de `play()` cria uma reprodução separada com seu próprio handle, mesmo para o mesmo arquivo. Tocar um clip duas vezes dá dois sons sobrepostos.

### Ciclo de vida

```
initialize() ──► play() / stop() / update() ... ──► dispose()
```

## `audio_engine_t`

```cpp
class BF_AUDIO_API audio_engine_t
{
public:
    bool  initialize();
    u32   play(const char* path, const f32 volume, const bool loop);
    void  stop(const u32 handle);
    void  stop_all();
    bool  is_playing(const u32 handle) const;
    void  update();
    usize get_active_count() const;
    void  dispose();

private:
    ma_engine*                         engine;
    std::unordered_map<u32, ma_sound*> sounds;
    u32                                next_handle;
};
```

O construtor de cópia e a atribuição por cópia são deletados. Uma cópia compartilharia os mesmos ponteiros de `ma_engine` e `ma_sound`, e a cópia que fosse descartada primeiro liberaria a memória por baixo da outra.

Não há destrutor. A limpeza acontece só no `dispose()`, que o dono precisa chamar.

### Estado

| Campo         | Significado                                                         |
| ------------- | ------------------------------------------------------------------- |
| `engine`      | A engine do miniaudio. `nullptr` antes do `initialize()` e depois do `dispose()` |
| `sounds`      | Todo som iniciado e ainda não removido, indexado pelo handle        |
| `next_handle` | O handle que o próximo `play()` vai retornar. Começa em `1`         |

### `initialize()`

Cria o `ma_engine` com `ma_engine_init(nullptr, engine)`. A config `nullptr` significa os padrões do miniaudio: o dispositivo de reprodução padrão do sistema, a taxa de amostragem e o número de canais nativos dele, e a thread de mixagem própria do miniaudio.

Se a inicialização falha (por exemplo, sem dispositivo de áudio), ele registra `Could not initialize the audio engine (<code>).`, libera a engine e retorna `false`. `<code>` é o `ma_result` do miniaudio.

### `play(path, volume, loop)`

1. Retorna `0` na hora se a engine não foi inicializada, ou se `path` é nulo ou vazio.
2. Cria um `ma_sound` novo e carrega o arquivo com `ma_sound_init_from_file`. Os formatos são os que o miniaudio decodifica de fábrica: WAV, FLAC e MP3.
3. Se o carregamento falha, registra `Could not load audio '<path>' (<code>).` e retorna `0`.
4. Define o volume e o loop, inicia o som, guarda ele com um handle novo e retorna o handle.

O `path` é aberto como foi passado. Um caminho relativo é relativo ao diretório de trabalho do processo, que no editor é `applications/editor` (`set_rundir` no `xmake.lua` dele).

O `volume` é um multiplicador linear: `1.0` é o nível original do arquivo, `0.5` é metade da amplitude, `0.0` é mudo. Valores acima de `1.0` amplificam e podem distorcer (clipping).

O arquivo é carregado na thread que chamou, então o `play()` bloqueia até o arquivo estar aberto e pronto.

### `stop(handle)`

Procura o handle. Se existir, para e libera o som (`ma_sound_uninit`) e remove ele do mapa. Um handle desconhecido ou já removido não faz nada.

### `stop_all()`

Libera todos os sons do mapa e limpa ele. A engine em si continua viva, então o `play()` continua funcionando depois.

### `update()`

Remove os sons que chegaram ao fim. Ele junta os handles cujo `ma_sound_at_end` é verdadeiro e depois chama `stop()` em cada um. As duas passadas existem porque o `stop()` apaga do mapa, e apagar enquanto itera sobre o mesmo mapa invalidaria o loop.

Um som em loop nunca chega ao fim, então o `update()` nunca o remove. Ele toca até um `stop()`, `stop_all()` ou `dispose()`.

O `update()` precisa ser chamado com regularidade, uma vez por frame. Sem ele, os sons que terminaram continuam na memória e o `get_active_count()` continua contando eles. O editor chama a cada frame, logo depois do `scene_player.update()`.

### `is_playing(handle)`

`true` só se o handle está no mapa **e** o miniaudio informa que ele está tocando. Handles desconhecidos, handles removidos e sons que acabaram mas ainda não foram limpos pelo `update()` retornam `false`.

### `get_active_count()`

A quantidade de sons no mapa. Entre duas chamadas de `update()` isso pode incluir sons que acabaram de terminar. O painel de status do editor mostra esse valor.

### `dispose()`

Chama `stop_all()` e depois desliga e libera o `ma_engine`. Chamar duas vezes é seguro, já que a segunda chamada encontra o mapa vazio e a engine nula.

## Uso

### Quem é o dono

O `runtime_t` tem um membro `audio_engine_t audio`.

- O `runtime_t::initialize()` inicializa a plataforma, depois o scripting, depois o áudio. Se qualquer um falha, ele retorna `false`.
- O `runtime_t::dispose()` descarta o áudio **primeiro**, antes do scripting, do renderer e da plataforma.

O resto do código recebe uma referência para `runtime.audio` e nunca cria a sua própria.

### O componente de fonte de áudio

Uma entidade toca som pelo `components::audio_source_component_t`:

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

| Campo           | Padrão    | Significado                                          |
| --------------- | --------- | ---------------------------------------------------- |
| `clip_path`     | vazio     | Arquivo a tocar, até 255 caracteres mais o terminador |
| `volume`        | `1.0`     | Passado para o `play()` como está                    |
| `loop`          | `false`   | Passado para o `play()` como está                    |
| `play_on_start` | `false`   | Toca automaticamente quando a cena inicia            |

Os padrões vêm do `scene_t::add_audio_source()`. O componente só guarda configurações. Ele não armazena um handle, então a engine não sabe quais sons pertencem a qual entidade.

O sistema `audio` do `scene_t` casa com toda entidade que tem esse componente. No editor, os campos são editados no inspetor de objetos.

### Reprodução da cena

O `scene_player_t` usa a engine em dois lugares:

- O `start()` primeiro chama `stop()`, depois passa por toda entidade do sistema de áudio e chama `play()` para cada uma com `play_on_start` ligado. Os handles que ele recebe não são guardados.
- O `stop()` chama `stop_all()`. Isso para **todos** os sons da engine, inclusive os iniciados pelo Lua.

### API em Lua

Os scripts recebem uma tabela `audio`:

| Função               | Retorna  | Descrição                                              |
| -------------------- | -------- | ------------------------------------------------------ |
| `audio.play(entity)` | handle   | Toca a fonte de áudio da entidade com o clip, volume e loop dela |
| `audio.stop(handle)` | nada     | Para um som                                            |

O `audio.play` gera um erro de Lua se a entidade não tem componente de fonte de áudio. Se o arquivo não pode ser carregado, ele não gera erro; retorna `0`.

O script precisa guardar o handle se quiser parar o som depois:

```lua
function update_audio(entity)
    if input.is_key_pressed("P") then
        sound_handle = audio.play(entity)
    end

    if input.is_key_pressed("O") and sound_handle ~= 0 then
        audio.stop(sound_handle)
        sound_handle = 0
    end
end
```

O Lua chega na engine por um upvalue: quando as tabelas são registradas, um light userdata apontando para o `audio_engine_t` é guardado como upvalue `2` de cada função, e o `get_audio_engine()` lê ele de volta.

### Ordem do frame no editor

```
poll events
scene_player.update(delta_time)   -- scripts podem chamar audio.play / audio.stop
runtime.audio.update()            -- remove os sons que terminaram
render
```

A mixagem não segue esse loop. O miniaudio mixa na própria thread, então um som iniciado continua tocando no mesmo ritmo mesmo se um frame demorar.

## Observações

- O `update()` precisa rodar todo frame. Sem ele, os sons que terminaram ficam na memória e o `get_active_count()` não desce.
- O `play()` carrega o arquivo na thread que chamou, então um arquivo grande segura aquele frame até estar pronto.
- Os handles dos sons "play on start" não são guardados por ninguém, então só o `stop_all()` consegue pará-los.
- O `scene_player_t::stop()` para todos os sons da engine, inclusive os iniciados pelo Lua.
- Não há destrutor. Se o `dispose()` nunca for chamado, os sons e a engine do miniaudio nunca são liberados.
