# Code Review — AI MIDI Composer

## Objetivos

- Garantir aderência a `prompts/master_prompt.md` e `standards/`.
- Prevenção de regressao arquitetural.
- Compartilhar conhecimento; nao catar apenas erros.

## Critérios de bloqueio

- Não-funcional: falha em lint/format/build/teste.
- Involavel: violacao de audio thread (mutex/IO/alloc).
- Determinismo: aleatoriedade sem seed.
- IA gerando MIDI (fata) -> rejeição imediata.
- Coupling: dependência circ de C++ para C quando deveria ser contrario.
- Performance: regressão >5% sem justificativa ADR.
- Teste de musical: PR de feature musical sem golden MIDI.

## Checklist do reviewer

1. O código faz o que o titulo diz?
2. Restriçes do master_prompt respeitadas?
3. Interfaces/DI usadas onde se aplica?
4. Tem teste? E golden quando aplicável?
5. Ha documentacao atualizada?
6. Ha hidden allocations/locks em path realtime?
7. Ha sugestão de simplificaçao/claridade?

## Comentários

- Seja concreto: proponha alteracao, nao apenas critica.
- `nit:` para menor stylistico (nao bloqueia).
- `block:` para critique que bloqueia merge.
- Aprove e reclame publicamente; mantenha discussao técnico.

## Tempo

- First response em PR: < 24h em dias úteis.
- Review completa: < 48h.
