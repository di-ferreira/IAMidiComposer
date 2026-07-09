---
name: changelog
description: Manutenção de CHANGELOG.md - Keep a Changelog style (Added/Changed/Fixed/Removed), vinculado a releases, sem entradas de chores/Auth sem valor.
---

# changelog

## Padrao (Keep a Changelog)

```md
## [Unreleased]

### Added
- (nova funcionalidade X)

### Changed
- (alterada Y)

### Fixed
- (corrigido Z bug)

### Removed
- (deprecado W removido)

## [0.1.0] - YYYY-MM-DD
- Initial release
```

## Regras

- Uma seção `[Unreleased]` no topo; movida para versao no release commit.
- PRs de feature/bugfix atualizam Unreleased; chore PRs nao criam entries.
- Referencie issue/ADR: `(#123, see ADR-0007)`.
- Breaking changes explicitos: `**BREAKING**: ...`.

## Anti-padroes

- Commit que adiciona release sem tag correspondente (CI garante).
- Misturar release notes de CI (auto-gen) com changelog humano (mantemos manual).
