# Módulo de IO

> Idioma: [English](../../../../en/engine/modules/io/README.md) | **Português (Brasil)**

O módulo de io lê e escreve arquivos e trabalha com caminhos e diretórios. Ele tem duas partes: `file_t`, uma struct header-only que lê um arquivo inteiro para um buffer na heap ou escreve um buffer num arquivo usando o `stdio` do C, e `file_system_t`, um conjunto de funções estáticas sobre `std::filesystem` para verificar, criar, listar e remover caminhos e para ler e escrever texto. Fica no namespace `byteforge::io`.

## Arquivos

| Arquivo                                                             | Conteúdo                                            |
| ------------------------------------------------------------------- | --------------------------------------------------- |
| `engine/include/byte-forge-engine/modules/io/e_file_access_modes.h` | `e_file_access_modes`: `Read`, `Write`              |
| `engine/include/byte-forge-engine/modules/io/e_file_data_types.h`   | `e_file_data_types`: `Text`, `Binary`               |
| `engine/include/byte-forge-engine/modules/io/file.h`                | `file_t`, declarado e implementado no header        |
| `engine/include/byte-forge-engine/modules/io/file_system.h`         | Declaração de `directory_entry_t` e `file_system_t` |
| `engine/source/modules/io/file_system.cpp`                          | Implementação do `file_system_t`                    |

Peças relacionadas em outros módulos:

| Arquivo                                                                  | Papel                                               |
| ------------------------------------------------------------------------ | --------------------------------------------------- |
| `modules/rendering/opengl/shader/opengl_shader.cpp`                      | Lê o código dos shaders com `file_t` (`Text`)       |
| `modules/rendering/font/font.cpp`                                        | Lê arquivos de fonte com `file_t` (`Binary`)        |
| `applications/editor/source/editor.cpp`                                  | Cria o diretório `assets` na inicialização          |
| `applications/editor/source/ui/components/content_browser_component.cpp` | Lista diretórios e cria scripts novos               |
| `applications/editor/source/ui/components/script_editor_component.cpp`   | Abre e salva scripts com `read_text` e `write_text` |

## Conceitos

### Erros são valores de retorno

Nada neste módulo loga ou lança exceção de propósito. Toda operação que pode falhar retorna `false` (ou um resultado vazio), e toda chamada de `std::filesystem` usa a sobrecarga que recebe um `std::error_code`. Quem chama decide o que logar; por exemplo, o editor de scripts loga `Could not open script '<path>'.` quando `read_text` falha.

### Texto e binário

Os arquivos são sempre abertos em modo binário (`"rb"` ou `"wb"`), então não há tradução de quebras de linha em nenhuma plataforma. A única diferença entre `e_file_data_types::Text` e `Binary` é que `Text` escreve um `'\0'` depois do último byte lido, para que o buffer possa ser usado como string C.

### Strings de caminho

Os caminhos entram como `const char*` e saem como `std::string`. As funções que retornam caminhos (`get_parent_path`, `join` e o campo `path` das entradas de diretório) usam `generic_string()`, então o separador é sempre `/`, até no Windows. Nomes de arquivo e extensões usam `string()`.

## `e_file_access_modes` e `e_file_data_types`

```cpp
enum class e_file_access_modes : u8 { Read, Write };
enum class e_file_data_types   : u8 { Text, Binary };
```

`e_file_access_modes` só é usado dentro do `file_t` para escolher entre `"rb"` e `"wb"`. `e_file_data_types` é passado para `file_t::read()`.

## `file_t`

```cpp
struct file_t
{
public:
    char* content;
    usize size;

    file_t();
    ~file_t();

    file_t(const file_t&)            = delete;
    file_t& operator=(const file_t&) = delete;

    bool read(const char* path, const e_file_data_types data_type);
    bool write(const char* path, const void* data, usize data_size);
    void dispose();

private:
    constexpr static const char* read_mode  = "rb";
    constexpr static const char* write_mode = "wb";

    constexpr static const char* get_file_mode(const e_file_access_modes access_mode);
};
```

O `file_t` é dono do buffer que lê. O destrutor chama `dispose()`, então um `file_t` na pilha libera o conteúdo quando sai de escopo. A cópia é deletada e, como a struct declara um destrutor e cópias deletadas, ela também não tem operações de move.

### Estado

| Campo     | Significado                                                            |
| --------- | ---------------------------------------------------------------------- |
| `content` | Buffer na heap vindo de `malloc` com os bytes do arquivo, ou `nullptr` |
| `size`    | Número de bytes lidos, sem contar o terminador. `0` quando vazio       |

### `read(path, data_type)`

1. Chama `dispose()`, então qualquer conteúdo anterior é liberado primeiro.
2. Pega o tamanho do arquivo com `std::filesystem::file_size`. Retorna `false` se isso falhar (por exemplo, o arquivo não existe).
3. Abre o arquivo com `fopen(path, "rb")`. Retorna `false` se isso falhar.
4. Aloca `size + 1` bytes com `malloc`. Se falhar, volta `size` para `0`, fecha o arquivo e retorna `false`.
5. Lê o arquivo inteiro com um único `fread` e fecha ele.
6. Se foram lidos menos bytes que o esperado, chama `dispose()` e retorna `false`.
7. Para `Text`, escreve `'\0'` em `content[size]`.
8. Retorna `true`.

O buffer sempre tem um byte extra. Para `Binary` esse byte fica sem inicializar.

### `write(path, data, data_size)`

Abre `path` com `fopen(path, "wb")`, que cria o arquivo ou trunca um existente, escreve `data_size` bytes de `data` com um único `fwrite`, fecha o arquivo e retorna se todos os bytes foram escritos. O diretório pai já precisa existir. `write()` não usa nem altera `content` e `size`.

### `dispose()`

Libera `content` com `free` se não for `nullptr`, coloca ele em `nullptr` e coloca `size` em `0`. Chamar mais de uma vez é seguro.

## `directory_entry_t`

```cpp
struct directory_entry_t
{
public:
    std::string name;
    std::string path;
    std::string extension;
    bool is_directory;
};
```

### Estado

| Campo          | Significado                                                         |
| -------------- | ------------------------------------------------------------------- |
| `name`         | Nome do arquivo ou diretório com extensão, por exemplo `rotate.lua` |
| `path`         | Caminho completo como listado, com separadores `/`                  |
| `extension`    | Extensão incluindo o ponto, por exemplo `.lua`. Vazia se não houver |
| `is_directory` | `true` para diretórios. `false` se a própria verificação falhou     |

## `file_system_t`

```cpp
struct BF_IO_API file_system_t
{
public:
    static bool exists(const char* path);
    static bool is_directory(const char* path);
    static bool create_directories(const char* path);
    static bool remove(const char* path);
    static bool rename(const char* path, const char* new_path);
    static bool list_directory(const char* path, std::vector<directory_entry_t>& out_entries);
    static std::string get_parent_path(const char* path);
    static std::string get_file_name(const char* path);
    static std::string get_extension(const char* path);
    static std::string join(const char* base_path, const char* relative_path);
    static bool read_text(const char* path, std::string& out_text);
    static bool write_text(const char* path, const std::string& text);
};
```

Todos os membros são estáticos, e a struct não tem estado.

### `exists(path)` e `is_directory(path)`

Encapsulam `std::filesystem::exists` e `std::filesystem::is_directory`. Qualquer erro (por exemplo, um problema de permissão) faz elas retornarem `false`.

### `create_directories(path)`

Cria `path` e todos os pais que faltam com `std::filesystem::create_directories`. Retorna `true` se não houve erro e `path` é um diretório depois disso, então também retorna `true` quando o diretório já existia.

### `remove(path)`

Apaga `path` com `std::filesystem::remove_all`. Para um diretório isso apaga tudo dentro dele, recursivamente. Retorna `true` só se pelo menos uma entrada foi removida e não houve erro. Um caminho que não existe retorna `false`.

### `rename(path, new_path)`

Move ou renomeia um arquivo ou pasta com `std::filesystem::rename`. Retorna `false` em qualquer erro, por exemplo quando o arquivo está aberto em outro programa. Não confere se `new_path` existe; o Content Browser confere isso antes.

### `list_directory(path, out_entries)`

1. Limpa `out_entries`.
2. Abre um `std::filesystem::directory_iterator` em `path`. Retorna `false` se isso falhar.
3. Para cada entrada diretamente dentro de `path` (não recursivo), preenche um `directory_entry_t` e adiciona.
4. Ordena as entradas: diretórios primeiro, depois arquivos, cada grupo por `name` em ordem de bytes.
5. Retorna `true`.

A ordem de bytes compara os caracteres pelo código, então nomes com maiúscula vêm antes dos com minúscula (`Zebra` antes de `apple`).

### `get_parent_path(path)`, `get_file_name(path)`, `get_extension(path)`

Wrappers finos sobre `std::filesystem::path`:

| Função            | `"assets/scripts/rotate.lua"` dá | Separador      |
| ----------------- | -------------------------------- | -------------- |
| `get_parent_path` | `assets/scripts`                 | `/` (genérico) |
| `get_file_name`   | `rotate.lua`                     | não se aplica  |
| `get_extension`   | `.lua`                           | não se aplica  |

Elas só trabalham na string. Não verificam se o caminho existe.

### `join(base_path, relative_path)`

Junta as duas partes com `std::filesystem::path::operator/` e retorna o resultado com separadores `/`. `join("assets", "script_0.lua")` dá `assets/script_0.lua`. Se `relative_path` for absoluto, o resultado é só `relative_path`, que é como o `operator/` funciona.

### `read_text(path, out_text)`

Lê o arquivo com um `file_t` local em modo `Text` e atribui os `size` bytes dele a `out_text`. Retorna `false` se a leitura falhar, deixando `out_text` intacto. O buffer temporário é liberado quando a função retorna.

### `write_text(path, text)`

Escreve `text.size()` bytes de `text` com um `file_t::write()` local. Retorna `false` se o arquivo não puder ser aberto ou se nem todos os bytes forem escritos.

## Uso

O editor cria seu diretório de assets durante a inicialização e só loga se isso falhar:

```cpp
if (!io::file_system_t::create_directories(assets_directory))
{
    LOG_ERROR(assets_error_format, assets_directory);
}
```

O content browser lista o diretório atual no seu vetor `entries`, sobe de nível com `get_parent_path` e cria scripts novos tentando `script_0.lua`, `script_1.lua` e assim por diante até `exists` retornar `false`:

```cpp
const std::string path = io::file_system_t::join(current_directory.c_str(), file_name);

if (!io::file_system_t::exists(path.c_str()))
{
    io::file_system_t::write_text(path.c_str(), new_script_template);
}
```

O editor de scripts carrega um arquivo com `read_text` e salva com `write_text`, logando um erro quando algum falha. No renderer, os shaders são lidos com `file_t` em modo `Text`, para que `file.content` possa ser entregue ao OpenGL como string C, e as fontes são lidas em modo `Binary`.

## Notas

- Caminhos relativos são relativos ao diretório de trabalho do processo. Para o editor, é `applications/editor`.
- `read()` pega o tamanho e abre o arquivo em dois passos separados. Se o arquivo mudar de tamanho no meio, a contagem do `fread` deixa de bater e `read()` falha.
- `file_t::write()` e `write_text()` substituem o arquivo inteiro. Não há modo de append.
- `list_directory` usa a sobrecarga com código de erro para abrir o diretório, mas avança o iterador com o `for` baseado em intervalo, que usa o incremento que lança exceção.
- `remove()` é recursivo e permanente. O Content Browser do editor chama ele no Delete, depois de uma confirmação.
- `read_text` e `file_t` leem o arquivo inteiro para a memória de uma vez; não há streaming.
