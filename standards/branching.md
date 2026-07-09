# Branching — AI MIDI Composer

## Estratégia: trunk-based + release branches

- **`main`**: branch principal, sempre release-ready.
- **Feature branches**: `feat/<scope>-<short>`; curta vida (< 3 dias).
- **Fix branches**: `fix/<scope>-<short>`.
- **Refactor branches**: `refactor/<scope>-<short>`.
- **Release branches**: `release/vX.Y.Z`N.Y (somente quando estabilizando release).

## Naming de branches

- `<type>/<scope>-<slug>` (kebab-case; sem maiúsculas).
- Exemplos: `feat/mte-ii-v-i`, `fix/plugin-midi-fifo-underflow`, `docs/addr-002`.

## Regras

- PRs em `main` apenas (não commits diretos).
- `main` merge via squash; conventional commit no squash title.
- Rebase feature branch em `main` antes de abrir PR; nao merge `main` na feature.
- Release branches: bugfixes > pick-back para `main` (cherry-pick).
