---
name: dependencies-audit
description: Auditoria de dependencias no CI - Dependabot (GitHub), govulncheck (Go), cppcheck/clang-tidy; CVES com critica de alto bloqueiam merge.
---

# dependencies-audit

## Regras

- **Go**: `govulncheck ./...` no CI; alto CVE -> bloqueia.
- **C++**: dependencias declaradas via CMake `FetchContent` com tag/commit imutável;
  mirror interno quando offline.
- **NPM** (tooling/MCP): `pnpm audit --prod`; sem `npm install` global.
- Dependabot config: atualizacoes semanais; auto-merge só para patch/lockfile.

## Licencas

- Todas deps devem ser compatíveis com GPL-3.0+:
  - MIT, BSD, Apache-2.0, ISC, MPL-2.0 OK.
  - GPL/LGPL/AGPL casos especiais.
- Nenhuma dependencia GPL incompatível (ex.: GPL-2-only com patent clauses).

## Anti-padroes

- `git submodule`-to main branch (fragil; fix commit/tag).
- Lockfile sem commit (reproducão comprometida).
- Deps sem CVE fix upstream (fork fixar e documentar ADR).
