---
name: llm-prompting
description: Prompting para LLM local - system prompts, few-shot, function calling misto (JSON schema), fewest possible tokens. Saída estruturada obrigatoria no projeto.
---

# llm-prompting

## Regras no AI MIDI Composer

- **Sempre schema JSON** como saida esperada; nunca texto livre para downstream musical.
- Usar **System Prompt fixo** + template (poucas variaveis) + few-shot curto.
- Seed de sampling em cada chamada para reproducibilidade.
- Reduzir invocações: cache (prompt + contexto hash + modelo versao) em SQLite.

## Padrao de prompt

```
[System]
Voce e o Prompt Interpreter do AI MIDI Composer. Responda exatamente como JSON.

Schema esperado:
{
  "genres": ["..."],
  "energy": "low|mid|high",
  "emotion": { "valence": 0..1, "arousal": 0..1, "dominance": 0..1 },
  "instruments": [...],
  "duration_sec": int,
  "form_guess": [...]
}

[User]
{prompt}

[Assistant]
(aqui vai apenas o JSON)
```

## Anti-padroes

- Prompt sem schema -> saida livre; recusado.
- Multi-turn com contexto nao reconciliado; resetar entre workflows.
- Aleatoriedade sem seed (use sampling_seed explicita no dock).

## Omega

- LLM nao decide notas MIDI (jamais!); apenas intenção musical.
