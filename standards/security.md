# Security — AI MIDI Composer

> O projeto é 100% offline. As ameaças são locais; ainda assim obrigatórias.

## Princípios

- **Nunca confiar em entrada da UI/plugin**: validar sempre no proto-gen-validate.
- **Nenhuma credencial**: projeto não tem autenticação; nunca adicionar campos de
  secret/token.
- **Logs não vazam dados sensíveis**: mascaramos prompts/PII quando necessário.
- **Modelos IA locais**: checksum SHA256 + assinatura/fingerprint; sem auto-download
  sem verificação.

## Superfícies de ataque (threat model)

| Entrada                              | Risco                               | Mitigação                            |
|--------------------------------------|-------------------------------------|---------------------------------------|
| Prompt do usuario                     | texto enorme / injection de prompt  | tamanho max, escape                   |
| Arquivo MIDI importado                | DoS (loops, milhões de eventos)     | parser robusto, limits                |
| Arquivo de projeto (JSON/binary)      | corrupção, path traversal           | schema validation                     |
| Parametros VST                        | audio thread hazard                 | atomic copy, sem dereference sem check|
| GGUF/ONNX baixado                    | supply-chain, modelo maligno        | checksum, mirror verificado           |
| Proto request (Plugin -> ACE)        | campo ilimitado, deep nested        | protobuf-gen-validate                 |

## Parsers

- **MidiParser**: deve tratar truncation, tons de meta events, sysex sem end,
  durations negativas. Sem array overflow. SEM CRASH.
- **ProtoRequest**: schema deve definir `[packed=true]` apenas quando apropriado;
  `repeated` com limites (max 1k entradas).
- **ProjectLoad**: não pode executar código dentro de blueprint.

## Auditorias

- `govulncheck` + Dependabot semanal; CVE alto bloqueia.
- `cppcheck`, `clang-tidy` com `cppcoreguidelines-*` checks.
- Fuzz targets em CI para parsers (ver `fuzz-testing`).

## Reporting

- `SECURITY.md` na raiz: policy, PGP/email para reporte privado.
