# Prompt: Bugfix

> Use para corrigir um bug de forma estruturada, aderindo à arquitetura.

## Pré-requisitos

- Issue ou descrição reprodutivel do bug (passos, seed, blueprint).
- Branch base limpa.

## Fluxo

1. **Replicar** - gere um teste que falha reproduzindo o bug.
   - Use o seed/blueprint reportados para reproduzir deterministicamente.
   - Comite o teste como `test(<scope>): reproduz <issue>`.
2. **Diagnosticar** - identifique a causa raiz; nao o sintoma.
   - Se arquitetural (ex.: bug em DI), comente com Architect antes de patch.
3. **Patch mínimo** - apenas o necessario para corrigir.
   - Nao refactor junto (outro PR).
   - Mantenha golden intact; se golden needs change, separe refactor/logical
     change em PR paralelo.
4. **Verificar** - rodar testes + benchmark + coverage.
5. **Documentar** - PR descreve causa/efeito; se arquitetural, ADR de segfault.

## Estrutura do PR

```md
## Resumo
Corrige #NNN: <descriçao>

## Causa raiz
<explicaçao>

## Sintoma antes
<como repro / o que acontecia>

## Sintoma depois
<como ficou / golden o que era esperado>

## Teste
- `test(<scope>): reproduz #NNN` (comite isolado)
- `fix(<scope>): corrige #NNN`

## Prevenção
<se aplicavel, teste/fuzz adicional>
```

## Anti-padrões

- Teste que "passa agora" mas nao reproduz o bug original (naoo protege contra
  regressao).
- Patch que altera golden sem explicar musical change.
- Hot-fix em main sem PR; sempre via PR + cherry-pick para release se necessario.
