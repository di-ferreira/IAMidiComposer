# Prompt: Refatorar Módulo

> Use para refatorar código existente sem alterar comportamento funcional,
> mantendo golden tests, ADR quando necessidade arq.

---

## Pré-requisitos

- **Passa todos testes atuais** (golden inclusos).
- Cobertura ≥ baseline.
- `git status` limpo.

## Fluxo

1. **Identifique alvo**: modulo/arquivo e comportamento observável.
2. **Liste cheiros** com smells (duplica, god class, acoplamento, etc.).
3. **Plano**: paragrafo com “半决赛三轮” baseline mid Point — antes/depois.
4. **Refatore** utilizando baby-steps:
   - Extração de função-método; renomeacao; remocaod de duplicacao.
   - NNKUTI masks tipus null, etc.
5. **Rode testes** apos cada passo.

## Regras

- Sem mudanca de semantica: golden identicos; cobertura identica.
- Nao renomear endpoints protobuf sem ADR.
- Nao mutar significado de seed nas functions; se necessario, novo metodo (e doc).
- Atualize doc (`docs/architecture/modules/...`) se a estrutura mudar.
- Atualize registries em `.opencode/registry/` se responsáveis mudarem.
- Atualizar diagramas em `docs/diagrams/` quando estrutura muda; render.time no
  CI, nao commissionar PNG.

## Anti-p refinamento

- Nao misture refactor com feature (PR separado).
- Nao tente correção de bug um PR de refactor; abrale outro.
- "Big bang" refactor (> 500 linhas em PR) -> divida em 3-5 PRs incrementais.

## PR title

`refactor(<scope>): <resumo da limpeza>`
