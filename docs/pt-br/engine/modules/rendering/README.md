# Módulo de renderização

> Idioma: [English](../../../../en/engine/modules/rendering/README.md) | **Português (Brasil)**

O módulo de renderização desenha o conteúdo 2D da engine com OpenGL 4.6 core. Tudo fica no namespace `byteforge::rendering`, dividido em pastas que devem virar módulos separados mais tarde:

| Pasta                  | O que contém                                                                                         | Depende de                                  |
| ---------------------- | ---------------------------------------------------------------------------------------------------- | ------------------------------------------- |
| `rendering/opengl/`    | Camada fina de OpenGL: carregamento de funções, estado de render, framebuffer, render pass, shader, mesh, textura | glad, math, logging, io (shaders), image (texturas) |
| `rendering/window/`    | A ponte com uma janela da plataforma: `opengl_window_context_t` (SDL) e `renderer_t`                 | `rendering/opengl`, platform, SDL3          |
| `rendering/lighting/`  | `point_light_t`, só dados da luz                                                                     | math                                        |
| `rendering/material/`  | `material_t`: arquivo de superfície + shading model (Unlit, Lit) compilados num programa             | `rendering/opengl`, io, math                |
| `rendering/sprite/`, `collider/`, `font/`, `texture/` | Desenho 2D em cima da camada OpenGL e os caches de textura e fonte    | `rendering/opengl`, math, io, image         |

Nada em `rendering/opengl/` inclui SDL, `platform`, o ECS ou a cena. Ela só precisa de um contexto OpenGL current e das funções carregadas, então pode ser usada com uma janela do SDL, do GLFW ou de qualquer outra biblioteca (veja [Usando a camada OpenGL sem o módulo de plataforma](#usando-a-camada-opengl-sem-o-módulo-de-plataforma)). `rendering/window/` é o único lugar que conhece o SDL e o `platform::window_t`.

As funções do OpenGL são carregadas com o [glad](https://github.com/Dav1dde/glad). As fontes são geradas com o `stb_truetype`, e as imagens são decodificadas pelo [módulo de imagem](../image/README.md).

## Arquivos

| Arquivo                                                                                           | Conteúdo                                                        |
| ------------------------------------------------------------------------------------------------- | --------------------------------------------------------------- |
| `engine/include/byte-forge-engine/modules/rendering/opengl/functions/opengl_functions.h`          | `opengl_functions_t`, `opengl_function_loader_t`                |
| `engine/include/byte-forge-engine/modules/rendering/opengl/state/opengl_render_state.h`           | `opengl_render_state_t` (blending, viewport, clear)             |
| `engine/include/byte-forge-engine/modules/rendering/opengl/framebuffer/opengl_framebuffer.h`      | Declaração do `opengl_framebuffer_t`                            |
| `engine/include/byte-forge-engine/modules/rendering/opengl/framebuffer/opengl_framebuffer_descriptor.h` | `opengl_framebuffer_descriptor_t`                         |
| `engine/include/byte-forge-engine/modules/rendering/opengl/pass/opengl_render_pass.h`             | Declaração do `opengl_render_pass_t`                            |
| `engine/include/byte-forge-engine/modules/rendering/opengl/shader/opengl_shader.h`                | Declaração do `opengl_shader_t`                                 |
| `engine/include/byte-forge-engine/modules/rendering/opengl/mesh/opengl_mesh.h`                    | Declaração do `opengl_mesh_t`                                   |
| `engine/include/byte-forge-engine/modules/rendering/opengl/texture/opengl_texture.h`              | Declaração do `opengl_texture_t`                                |
| `engine/include/byte-forge-engine/modules/rendering/window/opengl_window_context.h`               | Declaração do `opengl_window_context_t`                         |
| `engine/include/byte-forge-engine/modules/rendering/window/renderer.h`                            | Declaração do `renderer_t`                                      |
| `engine/include/byte-forge-engine/modules/rendering/lighting/point_light.h`                       | `point_light_t`                                                 |
| `engine/include/byte-forge-engine/modules/rendering/material/material.h`                          | `material_t`, `e_material_shading_models`                       |
| `engine/include/byte-forge-engine/modules/rendering/material/material_cache.h`                    | `material_cache_t`                                     |
| `engine/include/byte-forge-engine/modules/rendering/lighting/point_light_renderer.h`              | `point_light_renderer_t`                               |
| `engine/source/modules/rendering/material/material.cpp`                                           | Templates de shader e montagem do material                      |
| `engine/include/byte-forge-engine/modules/rendering/sprite/sprite_renderer.h`                     | Declaração do `sprite_renderer_t`                               |
| `engine/include/byte-forge-engine/modules/rendering/font/font.h`                                  | `font_glyph_t`, `font_t` e as constantes do intervalo de glifos |
| `engine/include/byte-forge-engine/modules/rendering/font/font_cache.h`                            | Declaração do `font_cache_t`                                    |
| `engine/include/byte-forge-engine/modules/rendering/texture/texture_cache.h`                      | Declaração do `texture_cache_t`                                 |
| `engine/source/modules/rendering/opengl/functions/opengl_functions.cpp`                           | Carregamento do glad                                            |
| `engine/source/modules/rendering/opengl/state/opengl_render_state.cpp`                            | Chamadas de blend, viewport e clear                             |
| `engine/source/modules/rendering/opengl/framebuffer/opengl_framebuffer.cpp`                       | Framebuffer e attachments                                       |
| `engine/source/modules/rendering/opengl/pass/opengl_render_pass.cpp`                              | Início de um pass: bind, viewport, clear                        |
| `engine/source/modules/rendering/opengl/shader/opengl_shader.cpp`                                 | Compilação, link e setters de uniform                           |
| `engine/source/modules/rendering/opengl/mesh/opengl_mesh.cpp`                                     | Vertex array e buffers, o quad unitário                         |
| `engine/source/modules/rendering/opengl/texture/opengl_texture.cpp`                               | Upload e bind de textura                                        |
| `engine/source/modules/rendering/window/opengl_window_context.cpp`                                | Criação do contexto no SDL                                      |
| `engine/source/modules/rendering/window/renderer.cpp`                                             | Lista de contextos, início e fim do frame                       |
| `engine/source/modules/rendering/sprite/sprite_renderer.cpp`                                      | Projeção, desenho de quads e layout de texto                    |
| `engine/source/modules/rendering/collider/collider_renderer.cpp`                                  | Contorno dos colliders                                          |
| `engine/source/modules/rendering/font/font.cpp`                                                   | Geração da fonte e a implementação do `stb_truetype`            |
| `engine/source/modules/rendering/font/font_cache.cpp`                                             | Cache de fontes                                                 |
| `engine/source/modules/rendering/texture/texture_cache.cpp`                                       | Cache de texturas                                               |

Peças relacionadas em outros módulos:

| Arquivo                                                   | Papel                                                       |
| --------------------------------------------------------- | ----------------------------------------------------------- |
| `modules/runtime/runtime.h`                               | Dono do `renderer_t`, que ele liga às janelas novas         |
| `modules/scene/scene.cpp`                                 | `scene_t::render()` desenha sprites e labels                |
| `modules/components/sprite_2D_component.h`                | Caminho da textura, tamanho e tint de um sprite             |
| `modules/components/label_2D_component.h`                 | Texto, caminho da fonte, tamanho da fonte e cor de um label |
| `modules/components/camera_2D_component.h`                | Zoom e flag de "primária" de uma câmera                     |
| `modules/platform/display/window/window_manager.cpp`      | Adiciona `SDL_WINDOW_OPENGL` quando o backend é OpenGL      |
| `applications/editor/assets/shaders/sprite.vert`, `.frag` | Os shaders que o editor passa para o sprite renderer        |

## Conceitos

### Como o SDL e o glad ficam escondidos

O `opengl_window_context_t` guarda a janela e o contexto do SDL como ponteiros para tipos com forward declaration:

```cpp
struct SDL_Window;
struct SDL_GLContextState;
```

Nenhum header público do módulo inclui o SDL ou o glad. O único `.cpp` que inclui o SDL é o `opengl_window_context.cpp`. Os arquivos `.cpp` incluem `<glad/gl.h>` e usam chamadas OpenGL diretas. O glad é gerado para o perfil core, versão 4.6 (`engine/xmake.lua`).

### Um contexto por janela

Cada janela tem o seu próprio contexto OpenGL. O primeiro contexto é criado sozinho. Cada um dos seguintes é criado com `SDL_GL_SHARE_WITH_CURRENT_CONTEXT`, compartilhando com o primeiro contexto da lista. Contextos compartilhados enxergam as mesmas texturas, buffers e programas de shader, então uma textura carregada com uma janela current pode ser desenhada em outra. Vertex arrays não são compartilhados pelo OpenGL, então uma mesh só pode ser usada no contexto que a criou.

As funções do OpenGL são carregadas uma vez, depois que o primeiro contexto existe, porque o glad precisa de um contexto current para buscá-las. Os ponteiros das funções ficam dentro da biblioteca da engine: uma aplicação que só usa os tipos OpenGL da engine não precisa do glad, mas uma que chama `gl*` diretamente precisa carregar a sua própria cópia.

### O contexto current

As chamadas OpenGL agem sobre o contexto que estiver current na thread que chama. Nenhum dos wrappers deixa um contexto current por conta própria. Criar, desenhar e descartar shaders, meshes e texturas só funciona enquanto o contexto certo está current. É o `renderer_t::begin_frame()` que deixa current o contexto de uma janela.

### Fluxo do frame

```
renderer.begin_frame(window)    -- make context current, set viewport, clear
sprite_renderer.begin(...)      -- enable alpha blending, upload projection
sprite_renderer.draw(...)       -- one draw call per quad
sprite_renderer.draw_text(...)  -- one draw call per visible glyph
(editor UI draws here)
renderer.end_frame(window)      -- swap buffers
```

### Coordenadas

O sprite renderer usa coordenadas de tela com Y para baixo e uma matriz de modelo `T * R * S`. As duas convenções, o layout column-major e a projeção ortográfica estão explicados no [módulo de matemática](../math/README.md). A posição de um sprite é o **centro** do quad. A posição de um texto é o canto **superior esquerdo** da linha.

### Posse dos objetos de GPU

Os wrappers de OpenGL e o `font_t` não têm destrutor. Os objetos de GPU só são liberados pelo `dispose()` ou `destroy()`, que o dono precisa chamar enquanto ainda existe um contexto do grupo de compartilhamento. Os wrappers são structs simples e podem ser copiados. Uma cópia guarda os mesmos ids do OpenGL, então descartar uma cópia deixa a outra com ids mortos.

## `renderer_t`

```cpp
class BF_RENDERING_API renderer_t
{
public:
    math::vec4_t clear_color;

    void initialize();
    bool attach_window(const platform::window_t& window);
    void detach_window(u32 window_id);
    bool begin_frame(const platform::window_t& window);
    void end_frame(const platform::window_t& window);
    void dispose();

    void clear(const math::vec4_t& color) const;
    void set_viewport(const math::recti_t& bounds) const;

private:
    opengl_window_context_t* find_window_context(u32 window_id);

    std::vector<opengl_window_context_t> window_contexts;
};
```

### Estado

| Campo             | Significado                                                                   |
| ----------------- | ----------------------------------------------------------------------------- |
| `clear_color`     | Cor usada pelo `begin_frame()`. O padrão é preto opaco `(0, 0, 0, 1)`         |
| `window_contexts` | Um `opengl_window_context_t` por janela ligada, na ordem em que foram ligadas |

### `initialize()`

Chama `opengl_window_context_t::set_context_attributes()`, que define os atributos do SDL para um contexto 4.6 core. Precisa rodar antes da primeira janela ser ligada.

### `attach_window(window)`

1. Cria um `opengl_window_context_t` para a janela. Se já existem outros contextos, o novo compartilha com `window_contexts.front()`.
2. Se a criação falha, registra `Could not create an OpenGL context for window <id>.`, destrói o contexto parcial e retorna `false`.
3. Se as funções ainda não foram carregadas, carrega com `opengl_functions_t::load(opengl_window_context_t::get_function_loader())`. Se falhar, registra `Could not load the OpenGL functions.`, destrói o contexto e retorna `false`.
4. Define o vsync a partir de `window.descriptor.use_vsync`. Se o SDL recusa, registra o aviso `Could not set vsync for window <id>.` e segue.
5. Adiciona o contexto à lista e retorna `true`.

O contexto novo fica current.

### `detach_window(window_id)`

Encontra o contexto dessa janela, destrói e remove da lista. Um id desconhecido não faz nada.

### `begin_frame(window)`

1. Encontra o contexto da janela e deixa ele current. Retorna `false` se não há contexto ou se o `SDL_GL_MakeCurrent` falha.
2. Faz bind do framebuffer da janela (`0`) e define o viewport como a janela inteira, de `(0, 0)` até `window.get_size()`. Esse tamanho é em pixels físicos (`SDL_GetWindowSizeInPixels`).
3. Limpa o color buffer com `clear_color` e retorna `true`.

Só o color buffer é limpo. Não há limpeza de depth nem de stencil.

### `end_frame(window)`

Troca os buffers da janela com `SDL_GL_SwapWindow`, o que mostra o frame. Uma janela desconhecida não faz nada.

### `dispose()`

Destrói todos os contextos e limpa a lista. As funções do OpenGL continuam carregadas.

### `clear(color)` e `set_viewport(rect)`

Atalhos para `opengl_render_state_t::clear_color()` e `opengl_render_state_t::set_viewport()`. Agem sobre o contexto current. Blending não faz parte do renderer: use o `opengl_render_state_t`.

## `opengl_functions_t`

```cpp
using opengl_function_t        = void (*)();
using opengl_function_loader_t = opengl_function_t (*)(const char* function_name);

struct BF_RENDERING_API opengl_functions_t
{
public:
    static bool load(opengl_function_loader_t function_loader);
    static bool are_loaded();
};
```

As funções do OpenGL vêm do driver em tempo de execução, pela biblioteca de janela que criou o contexto. `load()` passa o loader para o `gladLoadGL` e retorna `true` em caso de sucesso; um contexto precisa estar current. `SDL_GL_GetProcAddress` e `glfwGetProcAddress` têm o formato de `opengl_function_loader_t`, então a camada OpenGL não precisa saber qual biblioteca de janela está em uso.

## `opengl_render_state_t`

O OpenGL guarda um estado global por contexto. Estes helpers estáticos dão nome a cada mudança, para que um pass diga o que precisa:

| Membro                      | Chamada OpenGL                                                         |
| --------------------------- | ---------------------------------------------------------------------- |
| `enable_blending()`         | `glEnable(GL_BLEND)`                                                   |
| `disable_blending()`        | `glDisable(GL_BLEND)`                                                  |
| `use_alpha_blending()`      | `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`                    |
| `use_additive_blending()`   | `glBlendFunc(GL_SRC_ALPHA, GL_ONE)`: cores sobrepostas se somam        |
| `set_viewport(rect)`        | `glViewport` com a posição e o tamanho do retângulo                    |
| `clear_color(color)`        | `glClearColor` com a cor, depois `glClear(GL_COLOR_BUFFER_BIT)`        |
| `clear_depth_and_stencil()` | `glClear(GL_DEPTH_BUFFER_BIT \| GL_STENCIL_BUFFER_BIT)`                |

`sprite_renderer_t::begin()` e `collider_renderer_t::begin()` chamam `enable_blending()` e `use_alpha_blending()`.

## `opengl_framebuffer_t`

```cpp
struct opengl_framebuffer_descriptor_t
{
    bool use_depth_stencil;     // adiciona um renderbuffer com 24 bits de depth + 8 de stencil
    bool read_color_as_opaque;  // lê a textura de cor com alpha 1 (GL_TEXTURE_SWIZZLE_A = GL_ONE)
};

class BF_RENDERING_API opengl_framebuffer_t
{
public:
    opengl_framebuffer_descriptor_t descriptor;

    explicit opengl_framebuffer_t(const opengl_framebuffer_descriptor_t& descriptor);

    bool resize(const math::vec2i_t& size);
    void dispose();
    void bind() const;
    static void bind_window();

    const opengl_texture_t& get_color_texture() const;
    math::vec2i_t get_size() const;
    math::recti_t get_bounds() const;
    bool is_valid() const;
};
```

Um framebuffer é só um contêiner. As imagens onde ele desenha são os attachments: uma textura de cor que um pass seguinte pode ler e, quando `use_depth_stencil` está ligado, um renderbuffer de depth/stencil que é testado mas nunca lido.

`resize(size)` cria o framebuffer na primeira chamada e só recria os attachments quando o tamanho muda, então chamar todo frame com o tamanho da janela é barato. Ele confere o `glCheckFramebufferStatus`, registra `Could not create the framebuffer with size: <w>x<h>. Cause: incomplete attachment (status: <hex>).` e descarta tudo quando o framebuffer está incompleto. Ao final, deixa o framebuffer da janela com bind.

`bind_window()` faz bind do framebuffer `0`, o que a biblioteca de janela criou junto com a janela. Framebuffers não são compartilhados entre contextos.

`read_color_as_opaque` existe para o editor: o alpha blending deixa bordas translúcidas com alpha abaixo de 1, e desenhar a textura da cena dentro de um painel do ImGui aplicaria esse alpha uma segunda vez.

## `opengl_render_pass_t`

Um render pass é uma rodada de desenho em um alvo. Todo pass começa do mesmo jeito, e estes helpers fazem essa parte:

| Membro                             | O que faz                                                                                          |
| ---------------------------------- | -------------------------------------------------------------------------------------------------- |
| `begin(framebuffer, clear_color)`  | Faz bind do framebuffer, define o viewport como `get_bounds()`, limpa a cor (e depth/stencil, se tiver) |
| `begin_window(window_size)`        | Faz bind do framebuffer da janela e define o viewport como a janela inteira, sem limpar             |
| `begin_window(window_size, color)` | O mesmo, e depois limpa a cor                                                                      |

O blending e o que é desenhado ficam por conta do pass. O sandbox (`applications/sandbox/source/main.cpp`) mostra três passes: a cena em um framebuffer, um mapa de luz com blending aditivo em outro, e os dois combinados na janela.

## `point_light_t`

```cpp
struct point_light_t
{
    math::vec2_t position;   // pixels, no mesmo espaço da projeção da cena
    f32 radius;              // pixels, padrão 250
    math::vec4_t color;      // padrão (1, 0.85, 0.6, 1)
    f32 intensity;           // padrão 1

    f32 get_diameter() const;
};
```

Só os dados de uma luz que brilha igual em todas as direções e some até o raio. Não tem código de desenho: o shader que a desenha decide como fica o fade. `get_diameter()` é o lado do quad quadrado que cobre a luz.

## `material_t`

Um material é uma função de superfície mais um shading model. O arquivo do material só decide a cor de cada pixel; a engine envolve ele com um vertex shader, as entradas e o `main()` do shading model, e compila o resultado como um programa OpenGL normal. Cada material é o seu próprio programa: não existe um uber shader.

```cpp
class BF_RENDERING_API material_t
{
public:
    bool load(const char* material_surface_path, const e_material_shading_models material_shading_model);
    bool reload();
    void dispose();
    void bind() const;

    void set_projection(const math::mat4_t& projection) const;
    void set_model(const math::mat4_t& model) const;
    void set_time(f32 time_in_seconds) const;
    void set_light_map(const opengl_texture_t& light_map, u32 texture_slot) const;

    void set_float(const char* parameter_name, f32 value);
    void set_int(const char* parameter_name, i32 value);
    void set_vector_2(const char* parameter_name, const math::vec2_t& value);
    void set_vector_3(const char* parameter_name, const math::vec3_t& value);
    void set_vector_4(const char* parameter_name, const math::vec4_t& value);
    void set_texture(const char* parameter_name, const opengl_texture_t& texture, u32 texture_slot);
};
```

### O arquivo do material

O arquivo declara os seus parâmetros como uniforms e define o `surface()`:

```glsl
uniform vec4 u_line_color;

void surface(in material_input_t surface_input, inout material_output_t surface_output)
{
    surface_output.base_color = u_line_color.rgb;
    surface_output.opacity    = surface_input.uv.x;
}
```

| `material_input_t` | Significado                                |
| ------------------ | ------------------------------------------ |
| `uv`               | Coordenada de textura da mesh              |
| `world_position`   | Posição depois do `u_model`                |
| `normal`           | Normal depois do `u_model`, normalizada    |
| `time`             | Valor do `set_time()`, em segundos         |

| `material_output_t` | Padrão       | Significado                                     |
| ------------------- | ------------ | ----------------------------------------------- |
| `base_color`        | `vec3(1.0)`  | Cor que o shading model ilumina                 |
| `opacity`           | `1.0`        | Alpha do resultado                              |
| `emissive`          | `vec3(0.0)`  | Somada depois da luz, então nunca fica escura   |

`input` e `output` são palavras reservadas no GLSL, então os parâmetros precisam de outros nomes. O arquivo é inserido depois de um `#line 1`, então os erros de compilação apontam linhas do arquivo do material.

### Shading models

| `e_material_shading_models` | Cor final                                                                       |
| --------------------------- | ------------------------------------------------------------------------------- |
| `Unlit`                     | `base_color + emissive`, com `opacity`                                          |
| `Lit`                       | `base_color * light + emissive`, onde `light` é o mapa de luz lido na posição do pixel na tela (`set_light_map()`) |

O `Lit` combina com o mapa de luz que o sandbox desenha no segundo pass: o material lê ele direto, sem um pass separado de combinação.

### Uniforms e parâmetros

O template de vértice usa `u_projection` e `u_model`; o cabeçalho de fragmento usa `u_time`; o `Lit` adiciona `u_light_map`. As locations deles são buscadas uma vez no `reload()`. Os setters de parâmetro buscam a location pelo nome na primeira chamada e guardam em cache; um nome desconhecido dá `-1`, que o OpenGL ignora. Como os setters do shader, eles escrevem no programa com bind, então chame `bind()` antes.

`reload()` lê o arquivo de novo e mantém o programa antigo se o novo falhar, então dá para editar um material com a aplicação rodando.

### Materiais em sprites

O `sprite_renderer_t::draw_with_material()` desenha o quad do sprite com um material em vez do shader de sprite. Além de projeção, model e tempo, ele define três parâmetros que o material pode declarar:

| Uniform         | Tipo        | Valor                                  |
| --------------- | ----------- | -------------------------------------- |
| `u_texture`     | `sampler2D` | A textura do sprite, no slot `0`       |
| `u_use_texture` | `bool`      | `true` quando o sprite tem textura     |
| `u_tint`        | `vec4`      | O tint do sprite                       |

Sprites com `is_lit` e sem `material_path` usam o `get_default_lit_material()`, um material `Lit` montado a partir do `sprite_material_template` (textura vezes tint), então qualquer sprite pode receber luz. Materiais `Lit` também recebem o mapa de luz no slot `1`, definido pelo `set_material_frame()`; o sistema `light_2D` da cena chama ele depois de desenhar o mapa de luz. Sem os recursos de iluminação, um material `Lit` lê uma textura sem bind, então use `Unlit` nesse caso. O botão "New Material" do Content Browser do editor cria um arquivo que já usa esses três uniforms.

## `material_cache_t`

Carrega um material no primeiro uso e guarda, como o `texture_cache_t`. A chave é o caminho mais o shading model, então o mesmo arquivo pode estar em cache como `Unlit` e como `Lit`. Um material que falha fica marcado como falho e retorna `nullptr` até o `invalidate(path)`, que remove os dois shading models daquele caminho; o botão "Reload Material" do editor chama ele. O `asset_manager_t` tem um. O `reload_modified()` recompila todo material em cache cujo arquivo mudou no disco desde que foi carregado (incluindo os que falharam); o editor chama ele cerca de duas vezes por segundo, então salvar um arquivo de material atualiza o viewport.

## `point_light_renderer_t`

Desenha valores de `point_light_t` no alvo com bind, com o seu próprio shader embutido: `begin(projection)` liga o blending aditivo, e cada `draw(light)` desenha um quad quadrado do diâmetro da luz, cujos pixels caem linearmente do centro até o raio. O sistema `light_2D` da cena usa ele no mapa de luz; o sandbox mantém o seu próprio shader de luz para estudo.

## `opengl_window_context_t`

```cpp
struct BF_RENDERING_API opengl_window_context_t
{
public:
    u32 window_id;
    SDL_Window* sdl_window;
    SDL_GLContextState* gl_context;

    static void set_context_attributes();
    static opengl_function_loader_t get_function_loader();

    bool create(const platform::window_t& window, const opengl_window_context_t* shared_context);
    bool make_current() const;
    bool set_vsync(bool enabled) const;
    void swap_buffers() const;
    void destroy();
};
```

### Estado

| Campo        | Significado                                                    |
| ------------ | -------------------------------------------------------------- |
| `window_id`  | Id do SDL da janela onde este contexto desenha, `0` se nenhuma |
| `sdl_window` | A janela do SDL                                                |
| `gl_context` | O contexto OpenGL do SDL, `nullptr` se não foi criado          |

### `set_context_attributes()`

Define os atributos do SDL para os próximos contextos: perfil core, versão major `4`, minor `6`. O SDL lê esses atributos quando um contexto é criado, então eles valem para todo contexto criado depois.

### `get_function_loader()`

Retorna `SDL_GL_GetProcAddress` como um `opengl_function_loader_t`, para o `opengl_functions_t::load()`.

### `create(window, shared_context)`

1. Guarda o id da janela e a janela do SDL.
2. Se `shared_context` não é nulo, deixa ele current e define `SDL_GL_SHARE_WITH_CURRENT_CONTEXT` como `1`.
3. Cria o contexto com `SDL_GL_CreateContext` e depois volta o atributo de compartilhamento para `0`.
4. Retorna `true` se o contexto existe e pôde ficar current.

### `make_current()`, `set_vsync()`, `swap_buffers()`

`make_current()` chama `SDL_GL_MakeCurrent` e retorna `false` se não há contexto. `set_vsync(enabled)` deixa o contexto current e define o swap interval como `1` ou `0`. `swap_buffers()` chama `SDL_GL_SwapWindow`.

### `destroy()`

Destrói o contexto do SDL se existir e depois limpa os três campos. Pode ser chamado duas vezes com segurança.

## `opengl_shader_t`

```cpp
struct BF_RENDERING_API opengl_shader_t
{
public:
    u32 program_id;

    bool create(const char* vertex_shader_path, const char* fragment_shader_path);
    bool create_from_source(const char* vertex_shader_source, const char* fragment_shader_source);
    void bind() const;

    i32 get_uniform_location(const char* name) const;
    void set_matrix_4(i32 uniform_location, const math::mat4_t& matrix) const;
    void set_vector_2(i32 uniform_location, const math::vec2_t& vector) const;
    void set_vector_3(i32 uniform_location, const math::vec3_t& vector) const;
    void set_vector_4(i32 uniform_location, const math::vec4_t& vector) const;
    void set_float(i32 uniform_location, f32 value) const;
    void set_int(i32 uniform_location, i32 value) const;
    void set_bool(i32 uniform_location, bool value) const;

    void dispose();

private:
    bool link(u32 vertex_id, u32 fragment_id, const char* vertex_label, const char* fragment_label);
};
```

### Estado

| Campo        | Significado                                            |
| ------------ | ------------------------------------------------------ |
| `program_id` | Id do programa OpenGL, `0` se não foi criado ou falhou |

### `create(vertex_path, fragment_path)`

Lê cada arquivo como texto com `io::file_t`, compila e faz o link dos dois estágios. Se um arquivo não pode ser lido, registra `Could not read shader file '<path>'.`. Se um estágio não compila, registra `Could not compile shader '<path>': <info log>`. Os dois estágios são sempre compilados antes da tentativa de link, então os dois erros podem aparecer.

Um caminho relativo é relativo ao diretório de trabalho do processo.

### `create_from_source(vertex_source, fragment_source)`

Igual ao `create()`, mas compila as strings diretamente. Os erros usam `<embedded>` como rótulo.

### Link

O `link()` retorna `false` sem criar um programa se algum estágio falhou, depois de apagar o estágio que compilou. Caso contrário, cria o programa, anexa e faz o link dos dois estágios, e apaga os objetos de estágio, que não são mais necessários. Se o link falha, registra `Could not link shader program ('<vertex>', '<fragment>'): <info log>`, apaga o programa e retorna `false`.

O buffer do info log tem 1024 caracteres. Mensagens maiores são cortadas.

### `bind()` e setters de uniform

`bind()` chama `glUseProgram`. `get_uniform_location(name)` retorna a location do OpenGL, que é `-1` para um nome que não existe ou foi removido pela otimização.

Os setters chamam `glUniform*` com a location, então eles escrevem no programa **atualmente em uso**, não necessariamente neste. Faça o bind do shader antes. Uma location `-1` é ignorada pelo OpenGL.

| Setter         | Chamada OpenGL                                 |
| -------------- | ---------------------------------------------- |
| `set_matrix_4` | `glUniformMatrix4fv`, uma matriz, sem transpor |
| `set_vector_2` | `glUniform2f`                                  |
| `set_vector_3` | `glUniform3f`                                  |
| `set_vector_4` | `glUniform4f`                                  |
| `set_float`    | `glUniform1f`                                  |
| `set_int`      | `glUniform1i`                                  |
| `set_bool`     | `glUniform1i` com `1` ou `0`                   |

`set_matrix_4` passa `values` como está, o que funciona porque o `mat4_t` já é column-major.

### `dispose()`

Apaga o programa e define `program_id` como `0`.

## `opengl_mesh_t`

```cpp
struct BF_RENDERING_API opengl_mesh_t
{
public:
    u32 vertex_array_id;
    u32 vertex_buffer_id;
    u32 index_buffer_id;
    u32 index_count;

    bool create(const f32* vertices, usize vertex_float_count,
                const u32* indices, usize index_count,
                const u32* attribute_sizes, usize attribute_count);
    bool create_quad();
    void draw() const;
    void dispose();
};
```

### Estado

| Campo              | Significado                                |
| ------------------ | ------------------------------------------ |
| `vertex_array_id`  | O vertex array object (VAO)                |
| `vertex_buffer_id` | O vertex buffer (VBO)                      |
| `index_buffer_id`  | O index buffer (EBO)                       |
| `index_count`      | Número de índices desenhados pelo `draw()` |

### `create(...)`

Monta uma mesh indexada a partir de vértices float intercalados.

1. Soma os `attribute_sizes` para obter o stride em floats. Para os tamanhos `{ 3, 2 }` cada vértice tem 5 floats (20 bytes).
2. Gera o VAO e os dois buffers, faz o bind do VAO e envia vértices e índices com `GL_STATIC_DRAW`.
3. Para cada atributo `i`, chama `glVertexAttribPointer(i, size, GL_FLOAT, ...)` no offset acumulado e o habilita. O atributo `i` é a location `i` do shader.
4. Desfaz o bind do VAO, guarda `index_count` e retorna `true`.

`vertex_float_count` é o número de floats, não de vértices. Não há caminho de falha: o resultado é sempre `true`.

### `create_quad()`

Cria o quad unitário usado pelo sprite renderer: quatro vértices de `-0.5` a `0.5`, centrados na origem, cada um com uma posição (3 floats) e uma coordenada de textura (2 floats).

| Vértice | Posição        | UV       |
| ------- | -------------- | -------- |
| 0       | `(-0.5,  0.5)` | `(0, 1)` |
| 1       | `( 0.5,  0.5)` | `(1, 1)` |
| 2       | `( 0.5, -0.5)` | `(1, 0)` |
| 3       | `(-0.5, -0.5)` | `(0, 0)` |

Os índices `0, 1, 2, 2, 3, 0` formam dois triângulos. Com a projeção de Y para baixo, `y = -0.5` é a borda de cima na tela e tem `v = 0`, o que bate com imagens cuja primeira linha é a de cima.

### `draw()` e `dispose()`

`draw()` faz o bind do VAO, desenha `index_count` índices como `GL_TRIANGLES` com índices `GL_UNSIGNED_INT`, e desfaz o bind. `dispose()` apaga o VAO e os dois buffers e zera todos os campos.

## `opengl_texture_t`

```cpp
struct BF_RENDERING_API opengl_texture_t
{
public:
    u32 texture_id;
    math::vec2i_t size;

    bool load(const char* path);
    bool upload(const u8* rgba_pixels, const math::vec2i_t& pixel_size);
    void bind(u32 slot) const;
    void dispose();
};
```

### Estado

| Campo        | Significado                                    |
| ------------ | ---------------------------------------------- |
| `texture_id` | Id da textura no OpenGL, `0` se não há textura |
| `size`       | Largura e altura em pixels                     |

### `load(path)`

Carrega o arquivo com `image::image_t` (sempre 4 canais) e passa os pixels para o `upload()`. Se a imagem não pode ser carregada, retorna `false`; o módulo de imagem já registrou o motivo. Os pixels decodificados são liberados quando a função retorna.

### `upload(rgba_pixels, pixel_size)`

1. Gera um id de textura se ainda não há um. Um id existente é reaproveitado, então um novo upload substitui o conteúdo.
2. Guarda o tamanho e envia os pixels como `GL_RGBA` com componentes `GL_UNSIGNED_BYTE`.
3. Define filtragem linear para minificação e magnificação, e `GL_CLAMP_TO_EDGE` nos dois eixos.

Nenhum mipmap é gerado. A função sempre retorna `true`.

### `bind(slot)` e `dispose()`

`bind(slot)` ativa a unidade de textura `GL_TEXTURE0 + slot` e faz o bind da textura nela. `dispose()` apaga a textura e define `texture_id` como `0`. O `size` fica como estava.

## `font_t`

```cpp
constexpr u32 FONT_FIRST_CHARACTER = 32;
constexpr u32 FONT_CHARACTER_COUNT = 96;

struct font_glyph_t
{
    u16 x0, y0, x1, y1;
    f32 x_offset, y_offset, x_advance;
};

struct BF_RENDERING_API font_t
{
public:
    opengl_texture_t atlas;
    font_glyph_t glyphs[FONT_CHARACTER_COUNT];
    f32 size;

    bool load(const char* path, const f32 font_size);
    void dispose();
};
```

Uma fonte é um arquivo TrueType gerado em um tamanho de pixel dentro de um atlas de textura. Ela cobre os 96 caracteres do código 32 (espaço) até o 127.

### Estado

| Campo    | Significado                                                           |
| -------- | --------------------------------------------------------------------- |
| `atlas`  | Textura RGBA 512x512 que guarda todos os glifos                       |
| `glyphs` | Uma entrada por caractere, indexada por `code - FONT_FIRST_CHARACTER` |
| `size`   | Altura em pixels em que a fonte foi gerada, `0` se não carregada      |

Cada `font_glyph_t` guarda:

| Campo                | Significado                                             |
| -------------------- | ------------------------------------------------------- |
| `x0, y0, x1, y1`     | O retângulo do glifo no atlas, em pixels                |
| `x_offset, y_offset` | Onde o retângulo começa em relação à caneta na baseline |
| `x_advance`          | Quanto a caneta anda para a direita depois deste glifo  |

### `load(path, font_size)`

1. Descarta o atlas anterior.
2. Lê o arquivo inteiro como binário. Se falhar, registra `Could not read font '<path>'.` e retorna `false`.
3. Chama `stbtt_BakeFontBitmap` para a primeira fonte do arquivo, em `font_size` pixels, num bitmap de cobertura 512x512 de um canal. Se o resultado é `0` ou negativo (nem todos os caracteres couberam), registra `Could not fit font '<path>' at size <size> into the atlas.` e retorna `false`.
4. Converte o bitmap de cobertura para RGBA: cor branca, cobertura como alpha. Aplicar o tint no glifo branco dentro do shader dá a cor do texto.
5. Copia os retângulos, offsets e avanços gerados para `glyphs`, guarda o tamanho e envia o atlas.

O `stb_truetype` é compilado dentro da engine pelo `font.cpp`, que define `STB_TRUETYPE_IMPLEMENTATION`.

### `dispose()`

Descarta a textura do atlas e define `size` como `0`. O `glyphs` fica como estava.

## `sprite_renderer_t`

```cpp
class BF_RENDERING_API sprite_renderer_t
{
public:
    bool initialize(const char* vertex_shader_path, const char* fragment_shader_path);
    void begin(const math::vec2i_t& viewport_size, const math::vec2_t& camera_position, const f32 zoom, const bool use_camera);
    void draw(const opengl_texture_t* texture, const math::vec2_t& position, const math::vec2_t& size, f32 rotation_degrees, const math::vec4_t& tint);
    void draw_region(const opengl_texture_t* texture, const math::vec2_t& position, const math::vec2_t& size, f32 rotation_degrees, const math::vec4_t& tint, const math::vec4_t& uv_rect);
    void draw_text(const font_t& font, const char* text, const math::vec2_t& position, const math::vec4_t& color);
    void dispose();

private:
    opengl_shader_t shader;
    opengl_mesh_t quad;
    i32 projection_location;
    i32 model_location;
    i32 tint_location;
    i32 use_texture_location;
    i32 uv_rect_location;
};
```

### Estado

| Campo        | Significado                                              |
| ------------ | -------------------------------------------------------- |
| `shader`     | O programa de shader de sprites                          |
| `quad`       | O quad unitário de `opengl_mesh_t::create_quad()`        |
| `*_location` | Locations de uniform em cache, `-1` até o `initialize()` |

### O contrato do shader

O renderer não traz shaders próprios. Quem chama passa dois arquivos, e eles precisam declarar estes nomes:

| Nome                | Estágio  | Tipo        | Definido por                                        |
| ------------------- | -------- | ----------- | --------------------------------------------------- |
| location `0`        | vertex   | `vec3`      | Posição do quad                                     |
| location `1`        | vertex   | `vec2`      | Coordenada de textura do quad                       |
| `projection_matrix` | vertex   | `mat4`      | `begin()`                                           |
| `model_matrix`      | vertex   | `mat4`      | Todo draw                                           |
| `uv_rect`           | vertex   | `vec4`      | Todo draw: `(u, v, largura, altura)` da região      |
| `tint`              | fragment | `vec4`      | Todo draw                                           |
| `use_texture`       | fragment | `bool`      | Todo draw: `true` quando uma textura foi passada    |
| `in_texture`        | fragment | `sampler2D` | Definido como unidade `0` uma vez no `initialize()` |

Os shaders do editor fazem exatamente isso:

```glsl
// sprite.vert
texture_coordinates = uv_rect.xy + in_texture_coordinates * uv_rect.zw;
gl_Position = projection_matrix * model_matrix * vec4(position, 1.0);

// sprite.frag
out_color = use_texture ? texture(in_texture, texture_coordinates) * tint : tint;
```

### `initialize(vertex_path, fragment_path)`

Cria o shader a partir dos dois arquivos e o quad unitário. Retorna `false` se o shader falha (o shader já registrou o motivo). Depois busca as cinco locations de uniform, faz o bind do shader e define `in_texture` como a unidade de textura `0`. Um contexto precisa estar current.

### `begin(viewport_size, camera_position, zoom, use_camera)`

1. Habilita o blending com `GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA` (alpha blending padrão).
2. Escolhe a área visível:
   - Sem câmera, ou com `zoom <= 0`, ela é `left = 0`, `right = width`, `top = 0`, `bottom = height`. Uma unidade é um pixel e a origem é o canto superior esquerdo.
   - Com câmera, a área é centrada em `camera_position` e a metade do tamanho é `viewport / 2 / zoom`. Um zoom de `2` mostra metade da área, então tudo parece duas vezes maior.
3. Faz o bind do shader e envia `mat4_t::orthographic(left, right, bottom, top, -1, 1)`.

O `begin()` não define o viewport; ele só usa `viewport_size` para a projeção.

### `draw(texture, position, size, rotation_degrees, tint)`

Chama `draw_region()` com o retângulo de UV completo `(0, 0, 1, 1)`.

### `draw_region(texture, position, size, rotation_degrees, tint, uv_rect)`

1. Monta a matriz de modelo `identity().translate(position).rotate_z(rotation).scale(size)`. O quad unitário vira um retângulo de tamanho `size` centrado em `position`, rotacionado em torno do próprio centro.
2. Envia `model_matrix`, `tint`, `uv_rect` e `use_texture = texture != nullptr`.
3. Se há textura, faz o bind dela na unidade `0`.
4. Desenha o quad.

Passar `nullptr` como textura desenha um retângulo sólido na cor do tint. `uv_rect` é `(u, v, largura, altura)` em coordenadas normalizadas, com `v = 0` no topo da imagem.

### `draw_text(font, text, position, color)`

Faz o layout de uma linha de texto e desenha um quad por glifo.

1. A caneta começa em `position.x`. A baseline fica em `position.y + font.size`, então `position` é aproximadamente o canto superior esquerdo da linha.
2. Para cada byte de `text`, caracteres fora do intervalo de 32 a 127 são pulados sem mover a caneta.
3. Para um glifo com retângulo não vazio, calcula o centro da caixa do glifo (`caneta + offset + metade do tamanho`) e o retângulo de UV dele no atlas, e chama `draw_region()` com o atlas, sem rotação e com `color` como tint.
4. A caneta anda para a direita `x_advance`, também em glifos com retângulo vazio, como o espaço.

Não há kerning, quebra de linha nem tratamento de `\n`. O texto nunca é rotacionado nem escalado.

### `dispose()`

Descarta o quad e o shader.

## `texture_cache_t`

```cpp
class BF_RENDERING_API texture_cache_t
{
public:
    const opengl_texture_t* get(const char* path);
    usize get_count() const;
    usize get_failed_count() const;
    void invalidate(const char* path);
    void dispose();

private:
    std::unordered_map<std::string, opengl_texture_t> textures;
};
```

### Estado

| Campo      | Significado                                                                          |
| ---------- | ------------------------------------------------------------------------------------ |
| `textures` | Todo caminho já pedido, com a sua textura. Cargas que falharam têm `texture_id == 0` |

### `get(path)`

1. Retorna `nullptr` para um caminho nulo ou vazio.
2. Se o caminho não está no mapa, carrega com `opengl_texture_t::load()` e guarda o resultado **mesmo se o carregamento falhou**.
3. Retorna um ponteiro para a textura guardada, ou `nullptr` se o id dela é `0`.

Um caminho que falhou uma vez não é tentado de novo nas chamadas seguintes; ele continua retornando `nullptr` até ser invalidado. A chave é a string do caminho exatamente como foi passada, então `a.png` e `./a.png` são duas entradas. O ponteiro retornado continua válido até essa entrada ser removida.

### `get_count()` e `get_failed_count()`

`get_count()` é o número de entradas, incluindo as que falharam. `get_failed_count()` conta as entradas com `texture_id == 0`. O painel de status do editor mostra os dois.

### `invalidate(path)`

Descarta a textura desse caminho e remove a entrada, então o próximo `get()` carrega o arquivo de novo. Um caminho desconhecido não faz nada.

### `dispose()`

Descarta todas as texturas e limpa o mapa.

## `font_cache_t`

```cpp
class BF_RENDERING_API font_cache_t
{
public:
    const font_t* get(const char* path, const f32 size);
    usize get_count() const;
    usize get_failed_count() const;
    void dispose();

private:
    std::unordered_map<std::string, font_t*> fonts;
};
```

### Estado

| Campo   | Significado                                                                                  |
| ------- | -------------------------------------------------------------------------------------------- |
| `fonts` | Fontes alocadas no heap, indexadas por `"<path>@<size>"`, com o tamanho impresso como `%.1f` |

### `get(path, size)`

1. Retorna `nullptr` para um caminho nulo ou vazio, ou um tamanho `<= 0`.
2. Monta a chave com `"%s@%.1f"`, por exemplo `assets/fonts/arial.ttf@32.0`, num buffer de 300 caracteres.
3. Se a chave é nova, aloca um `font_t`, carrega e guarda **mesmo se o carregamento falhou**.
4. Retorna a fonte, ou `nullptr` se o id do atlas dela é `0`.

O mesmo arquivo em dois tamanhos gera duas entradas e dois atlas. Tamanhos que ficam iguais impressos com uma casa decimal compartilham uma entrada, que foi gerada com o tamanho pedido primeiro. Uma fonte que falhou nunca é tentada de novo; o `font_cache_t` não tem `invalidate()`.

### `get_count()`, `get_failed_count()` e `dispose()`

Funcionam como no cache de texturas: número de entradas, entradas cujo id do atlas é `0`, e `dispose` mais `delete` de cada fonte seguido da limpeza do mapa.

## Uso

O `runtime_t` é dono do `renderer_t` e cuida do lado das janelas:

- `runtime_t::initialize()` chama `renderer.initialize()` depois que platform, scripting e áudio deram certo.
- `runtime_t::create_window()` cria a janela do SDL e chama `attach_window()`. Se isso falha, a janela é destruída e o retorno é `nullptr`.
- `runtime_t::destroy_window()` chama `detach_window()` antes de destruir a janela.
- `runtime_t::dispose()` descarta o renderer depois do áudio e do scripting, antes da platform.

O editor (`editor_t`) é dono do `sprite_renderer_t`, do `texture_cache_t` e do `font_cache_t`, e passa os caches para a UI pelo `editor_context_t`. No `initialize()` ele chama `runtime.renderer.begin_frame(*window)` logo depois de criar a janela, o que deixa o contexto da janela current antes do `sprite_renderer.initialize()` criar o shader e o quad.

Em todo frame do editor:

```cpp
if (runtime.renderer.begin_frame(*window))
{
    scene.render(sprite_renderer, texture_cache, font_cache, window->get_size());

    debug_ui_context.begin_frame();
    draw_ui();
    debug_ui_context.end_frame();

    runtime.renderer.end_frame(*window);
}
```

O `scene_t::render()` acha a primeira câmera com `is_primary` ligado e passa a posição e o zoom dela para o `begin()`; sem câmera, usa coordenadas em pixels. Depois desenha todos os sprites, com `size * transform.scale` e `transform.rotation`, e em seguida todos os labels. Texturas e fontes são pedidas aos caches durante esse loop, então elas carregam no frame em que são necessárias pela primeira vez. Um sprite com `texture_path` vazio é desenhado como um retângulo sólido com o tint. Um label cuja fonte falha é pulado.

No encerramento, o editor descarta o cache de fontes, o cache de texturas e o sprite renderer antes do `runtime.dispose()` destruir os contextos.

## Usando a camada OpenGL sem o módulo de plataforma

`rendering/opengl/` só precisa de um contexto current e das funções carregadas. Com GLFW, por exemplo:

```cpp
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

GLFWwindow* window = glfwCreateWindow(1280, 720, "Demo", nullptr, nullptr);
glfwMakeContextCurrent(window);

rendering::opengl_functions_t::load(reinterpret_cast<rendering::opengl_function_loader_t>(glfwGetProcAddress));

rendering::opengl_framebuffer_t framebuffer;
rendering::opengl_shader_t shader;
rendering::opengl_mesh_t quad;
// ... o mesmo código do sandbox, com glfwSwapBuffers(window) no fim de cada frame
```

Com SDL, sem o `platform_t`, o loader é o `SDL_GL_GetProcAddress`. Nos dois casos o `renderer_t` e o `opengl_window_context_t` não são usados. Hoje todas as pastas ainda são compiladas na mesma biblioteca `byte-forge-engine`; a regra de que `rendering/opengl/` não inclui nada de `platform`, `ecs` ou `scene` é o que mantém possível separar depois.

## Observações

- `runtime_t::render()` começa e termina um frame para cada janela sem nada no meio, então ele só limpa e troca os buffers. O editor não chama esse método e controla o frame da própria janela.
- Cada quad e cada glifo é um draw call separado, com os seus próprios envios de uniform. Não há batching nem ordenação: os sprites são desenhados na ordem da lista de entidades do sistema de sprites, e todos os labels vêm depois de todos os sprites.
- As texturas usam filtragem linear sem mipmaps, então pixel art escalada fica borrada e texturas muito reduzidas podem cintilar.
- Chamar `create()` de novo num shader ou mesh que já tem objetos OpenGL não libera os antigos; os ids deles são sobrescritos. Uma textura é diferente: o `upload()` reaproveita o id, então carregar de novo substitui o conteúdo.
