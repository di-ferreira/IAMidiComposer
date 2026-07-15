# 0005. Integração LLM Local (llama.cpp) para Interpretação de Prompts

Data: 2026-07-15
Status: Proposta

## Contexto

O AI MIDI Composer precisa interpretar prompts em linguagem natural do usuário e convertê-los em intenção musical estruturada (estilo, energia, instrumentação, emoção, densidade, estrutura) — um `MusicBlueprint`.

O ACE (AI Composition Engine) em Go (`engine/go/`) orquestra os workflows. O MTE (Music Theory Engine) em C++20 (`engine/cpp/`) gera o MIDI propriamente dito, mas nunca recebe texto — apenas blueprints estruturados.

A interpretação do prompt é a única etapa que requer um LLM. Após a decisão arquitetural de que o LLM **nunca gera notas MIDI diretamente** (apenas intenção), resta definir o mecanismo de integração entre Go ACE e o LLM local.

Forças em conflito:

- **100% offline**: o modelo e o runtime devem funcionar sem internet após download inicial.
- **Sem Python no stack**: o projeto evita Python para não adicionar runtime frágil (~500MB) e dependência de pip/conda.
- **Consistência arquitetural**: o boundary Go ↔ C++ já foi definido como CLI tool no ADR-0004.
- **Latência aceitável**: interpretação de prompt é assíncrona e ocorre antes da composição — tolera segundos de latência.
- **Determinismo relativo**: o LLM é inerentemente probabilístico, mas seed fixa + temperature=0 garante reprodutibilidade para o mesmo prompt.
- **Modelo pequeno**: Llama-3.2-3B-Instruct Q4_K_M (~2GB) cabe em hardware médio e roda bem em CPU.
- **Modelo deve ser opcional**: se o modelo não estiver presente, o sistema deve fallback para um parser de intenção baseado em regras.

## Decisao

Adotar um **CLI tool (`prompt_interpreter`)** como boundary entre Go ACE e o LLM local, seguindo o mesmo padrão do ADR-0004.

- O diretório `engine/cpp/tools/prompt_interpreter/` conterá um executável C++ que:
  1. Linka o llama.cpp via `FetchContent` no CMake.
  2. Carrega o modelo GGUF de `engine/cpp/data/models/` (caminho configurável por variável de ambiente `AIMIDI_MODEL_PATH`).
  3. Lê um par `(system_prompt, user_prompt)` de `stdin` como JSON.
  4. Executa a inferência com seed fixa, temperature=0, contexto máximo configurável.
  5. Valida a saída JSON contra o schema do `MusicBlueprint`.
  6. Escreve o `MusicBlueprint` (JSON validado) em `stdout`.
  7. Escreve metadados (tokens gerados, tempo de inferência, modelo usado) em `stderr` como JSON.
  8. Retorna código de erro zero em sucesso, não-zero em falha (modelo ausente, schema inválido, timeout).

- O ACE em Go chamará `prompt_interpreter` via `os/exec` com `context.Context` para timeout.
- O caminho do modelo é configurável e aponta para `engine/cpp/data/models/` (gitignored).
- Se o binário ou o modelo não existirem, o ACE fallback para um interpretador de intenção baseado em regras (parser de palavras-chave) — viabilizando o desenvolvimento sem LLM.
- Para produção futura, o CLI pode ser substituído por um processo persistente com gRPC (unix domain socket) que mantém o modelo carregado em memória.

## Alternativas consideradas

- **A. cgo com llama.cpp linkado via cgo.**
  - Vantagens: chamada direta `llama_eval()` sem fork, latência mínima (~ms).
  - Por que não: cgo acopla build Go à toolchain C++ (clang, cabeçalhos do llama.cpp); cross-compilação inviável; lock do scheduler Go durante chamadas cgo; viola o padrão estabelecido no ADR-0004.

- **B. Python sidecar (FastAPI + llama-cpp-python).**
  - Vantagens: ecossistema maduro, instalação simples via pip, boa integração com bibliotecas de NLP.
  - Por que não: introduz dependência de Python (~500MB com torch/transformers), gerenciamento de venv/pip, processo separado a ser monitorado; o projeto decidiu deliberadamente evitar Python no stack (CONTEXT.md lista apenas C++, Go, e ferramentas build).

- **D. ONNX Runtime apenas.**
  - Vantagens: runtime mais leve que llama.cpp, sem GGUF, footprint reduzido.
  - Por que não: ONNX é adequado para classificadores pequenos (estilo, mood), mas incapaz de interpretar prompts abertos em linguagem natural; limitaria severamente a expressividade dos Workflows 1-10.

## Consequencias

- **Positivas:**
  - Consistência arquitetural total com ADR-0004 (mesmo padrão CLI + `os/exec`).
  - Zero dependência de cgo ou Python — ACE continua buildando puramente em Go.
  - `prompt_interpreter` pode ser testado isoladamente: `echo '{"system_prompt": "...", "user_prompt": "..."}' | ./prompt_interpreter`.
  - Modelo pode ser trocado sem recompilar nada (basta apontar `AIMIDI_MODEL_PATH` para outro GGUF).
  - Fallback rule-based permite desenvolvimento e testes offline sem LLM.
  - Saída validada contra schema do `MusicBlueprint` — evita que o LLM gere JSON inválido que chegue ao MTE.

- **Negativas:**
  - Overhead de processo (~100-500ms para carregar o modelo na primeira chamada). Aceitável pois interpretação de prompt é pré-composição e ocorre uma vez por sessão ou workflow.
  - O modelo de ~2GB precisa estar em disco. Download único, mas necessário.

- **Neutras:**
  - Pode-se migrar para gRPC persistente (manter modelo carregado em memória) mantendo a lógica de interpretação inalterada — cria-se um `prompt_interpreter_server` que reusa o mesmo código.
  - O schema de entrada/saída pode ser versionado e evoluir independentemente do transporte.
  - Outros modelos GGUF (maiores ou especializados) podem ser usados apenas alterando a variável de ambiente.

## Relacionado

- ADR-0004: Integração Go ACE → C++ MTE (mesmo padrão CLI + `os/exec`; este ADR estende o padrão para o LLM).
- ADR-0001: Boundary C++ ↔ Go — gRPC planejado para produção; CLI é fase intermediária.
- Skills: `llama-cpp`, `llm-prompting`, `model-safety`, `input-validation`.
- Roadmap: Phase 3 — CLI MVP com fallback rule-based; Phase N — gRPC persistente e suporte a múltiplos modelos.
