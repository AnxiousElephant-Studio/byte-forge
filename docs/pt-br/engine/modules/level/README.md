# Módulo de level

> Idioma: [English](../../../../en/engine/modules/level/README.md) | **Português (Brasil)**

O módulo de level salva cenas em arquivos JSON e as carrega de volta. Ele depende do módulo de cena; o módulo de cena não depende dele. Tudo fica no namespace `byteforge::level`.

## Arquivos

| Arquivo                                                             | Conteúdo                                                |
| ------------------------------------------------------------------- | ------------------------------------------------------- |
| `engine/include/byte-forge-engine/modules/level/level_file.h`       | `level_file_t`: a cena inteira, `.bflevel`              |
| `engine/include/byte-forge-engine/modules/level/stamp_file.h`      | `stamp_file_t`: uma entidade e seus filhos, `.bfstamp` |
| `engine/source/modules/level/entity_json.h`, `.cpp`                 | Interno: entidades para JSON e de volta                 |

## Formato das entidades

Os dois arquivos guardam um array `entities`. Cada item tem o `name` da entidade e um objeto por componente que ela tem (`transform_2D`, `sprite_2D`, `tags`, `point_light_2D`...). Uma entidade com pai tem `"parent": <índice>`, a posição do pai dentro do mesmo array. Índices são usados no lugar de ids porque os ids mudam toda vez que uma cena é carregada. Chaves ausentes mantêm os padrões, então arquivos antigos continuam carregando.

## `level_file_t`

| Membro                                    | Descrição                                                                    |
| ----------------------------------------- | ---------------------------------------------------------------------------- |
| `serialize(scene_context)`                | Texto da cena inteira, com `format_version` e `default_2D_systems`           |
| `save(scene_context, path, error)`        | Escreve o `serialize()` num arquivo                                          |
| `load(scene_context, path, error)`        | Lê um arquivo e chama o `deserialize()`                                      |
| `deserialize(scene_context, text, error)` | Limpa a cena, adiciona ou remove os sistemas 2D padrão e cria as entidades   |
| `clear(scene_context)`                    | Destrói todas as entidades                                                   |

O editor usa `serialize()` e `deserialize()` para guardar a cena no Play e restaurar no Stop.

## `stamp_file_t`

Um stamp é um pedaço reutilizável de cena: uma entidade e todos os seus filhos, salvos sozinhos.

| Membro                                       | Descrição                                                              |
| -------------------------------------------- | ---------------------------------------------------------------------- |
| `save(scene, root, path, error)`             | Salva `root` e a subárvore; a raiz é salva sem pai                     |
| `instantiate(scene, path, out_root, error)`  | Cria uma cópia das entidades salvas e retorna a nova raiz              |

Uma instância é uma cópia simples: editar o arquivo do stamp depois não muda as instâncias que já estão num level. `instantiate` tem a assinatura de `scene::stamp_loader_t`, então as aplicações passam ele para `script_system.stamp_loader` para o Lua poder chamar `scene.spawn_stamp(path)`.

No editor, "Save as Stamp" no menu de contexto de uma entidade cria `<assets>/stamps/<nome>.bfstamp`. Dar duplo clique num stamp no Content Browser, ou arrastar ele para o Scene Hierarchy, instancia o stamp.
