# <Name> module

> Language: [Português (Brasil)](<relative path to the pt-br mirror>) | **English**

<One or two paragraphs: what the module does, which library it wraps (if any), its namespace (`byteforge::<module>`), and whether it is header-only.>

## Files

| File                                                     | Contents                       |
| -------------------------------------------------------- | ------------------------------ |
| `engine/include/byte-forge-engine/modules/<module>/...`  | <what this header declares>    |
| `engine/source/modules/<module>/...`                     | <what this source does>        |

<Optional: a second table "Related pieces in other modules" when other modules use this one in an important way.>

## Concepts

<The ideas a reader needs before the API: data layout, ownership, handles, coordinate conventions, threading, how a third-party library is hidden. Use ### subsections, one per idea. Skip the section if the module is trivial.>

## <Type name, e.g. `audio_engine_t`>

<One section per public type. Start with the declaration as a cpp code block (public members only, plus private state when it explains behavior).>

### State

<Table of fields: Field | Meaning. Omit for types with no state.>

### <member()>

<One ### per non-trivial public member: what it does step by step, what it returns, failure cases and what is logged. Trivial getters can share one table instead.>

## Usage

<Who owns or calls it (runtime, editor, scene, Lua), lifecycle order, and a short real example taken from the codebase.>

## Notes

<Behavior worth knowing, stated neutrally as how the code works today: things that must be called, limits, edge cases. Not a list of bugs.>

---

Writing rules:

- Brazilian Portuguese is the primary language. Every document has an English mirror in `docs/en/` at the same path, with the same structure (same headings, tables and code blocks).
- Code, identifiers and code comments stay in English in both versions.
- Describe only what the code actually does. Anything inferred is not stated as fact.
- Full sentences, no em-dashes.
