# Módulo <Nome>

> Idioma: **Português (Brasil)** | [English](<caminho relativo para o espelho em en>)

<Um ou dois parágrafos: o que o módulo faz, qual biblioteca ele encapsula (se houver), o namespace (`byteforge::<module>`) e se é header-only.>

## Arquivos

| Arquivo                                                  | Conteúdo                       |
| -------------------------------------------------------- | ------------------------------ |
| `engine/include/byte-forge-engine/modules/<module>/...`  | <o que esse header declara>    |
| `engine/source/modules/<module>/...`                     | <o que esse código-fonte faz>  |

<Opcional: uma segunda tabela "Peças relacionadas em outros módulos" quando outros módulos usam este de forma importante.>

## Conceitos

<As ideias que o leitor precisa antes da API: layout dos dados, posse (ownership), handles, convenções de coordenadas, threads, como uma biblioteca de terceiros fica escondida. Use subseções ###, uma por ideia. Pule a seção se o módulo for trivial.>

## <Nome do tipo, ex.: `audio_engine_t`>

<Uma seção por tipo público. Comece com a declaração num bloco de código cpp (só membros públicos, mais o estado privado quando ele explica o comportamento).>

### Estado

<Tabela de campos: Campo | Significado. Omita para tipos sem estado.>

### <membro()>

<Uma ### por membro público não trivial: o que ele faz passo a passo, o que retorna, casos de falha e o que é registrado no log. Getters triviais podem dividir uma tabela só.>

## Uso

<Quem é o dono ou quem chama (runtime, editor, cena, Lua), a ordem do ciclo de vida e um exemplo curto tirado do código.>

## Observações

<Comportamentos que vale saber, descritos de forma neutra como o código funciona hoje: coisas que precisam ser chamadas, limites, casos de borda. Não é uma lista de bugs.>

---

Regras de escrita:

- Português (Brasil) é o idioma principal. Todo documento tem um espelho em `docs/en/` no mesmo caminho, com a mesma estrutura (mesmos títulos, tabelas e blocos de código).
- Código, identificadores e comentários de código ficam em inglês nas duas versões.
- Descreva só o que o código faz de fato. O que for inferido não é afirmado como fato.
- Frases completas, sem travessões.
