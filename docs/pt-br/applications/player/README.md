# Player

> Idioma: [English](../../../en/applications/player/README.md) | **Português (Brasil)**

O player roda um projeto ByteForge sem o editor. É o executável que acompanha um jogo pronto.

```
player.exe [caminho/do/projeto.bfproject]
```

Sem argumento, ele usa o primeiro arquivo `.bfproject` da pasta atual. Depois:

1. Carrega o projeto e transforma a pasta dele no diretório de trabalho, para os caminhos dos assets funcionarem como no editor.
2. Inicia o runtime da engine com o FPS alvo e o vsync do projeto, e abre uma janela redimensionável de 1280x720 com o nome do projeto.
3. Cria o sprite renderer com os shaders de sprite do projeto, o renderer de point lights e o mapa de luz.
4. Carrega o level inicial do projeto e começa a jogar.

A cada frame ele lê o input, roda a cena (update e render) direto na janela e troca os buffers. Quando um script chama `scene.load_level(path)`, o player para, carrega esse level e começa de novo. Stamps criados pelo Lua usam `level::stamp_file_t::instantiate`.

O código está em `applications/player`: `player_application_t` é dono do runtime, do contexto da cena e dos renderers; o `main.cpp` só encontra o arquivo do projeto.
