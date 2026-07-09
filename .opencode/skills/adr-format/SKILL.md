---
name: adr-format
description: Architecture Decision Records (ADR) - formato MADR simplificado em docs/adr/NNNN-titulo.md com Contexto, Decisao, Alternativas, Consequencias.
---

# adr-format

## Template `docs/adr/NNNN-titulo-kebab-case.md`

```
# NNNN. Titulo da Decisao

Data: YYYY-MM-DD
Status: Proposta | Aceita | Depreciada | Substituida por NNNN

## Contexto
(Por que precisamos decidir? Quais forcas?)

## Decisao
(O que decidimos?)

## Alternativas consideradas
- A: ... (por que nao)
- B: ... (por que nao)

## Consequencias
- Positivas: ...
- Negativas: ...
- Neutras: ...

## Relacionado
- ADR NNNN: ...
- Issues: #...
```

## Regras

- Numeracao zero-padded de 4 digitos; nunca reusar numero.
- Status muda via commit; PR de mudanca traz ADR novo referenciando antigo.
- ADR válido em PR: revisado por Software Architect + CTO.
