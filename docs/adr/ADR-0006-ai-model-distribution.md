# 0006. AI Model Distribution

Data: 2026-07-16
Status: Aceita

## Contexto

O AI MIDI Composer utiliza modelos de IA locais para interpretação de prompts (LLM via llama.cpp, GGUF) e classificação auxiliar (ONNX classifiers para estilo/mood). Estes modelos possuem tamanhos que inviabilizam seu versionamento no repositório Git:

- Llama-3.2-3B-Instruct Q4_K_M (~2 GB) — LLM principal para interpretação de prompts.
- Classificadores ONNX (~10–50 MB cada) — detecção de estilo, mood, densidade.
- Modelo de análise de áudio (opcional, ~200 MB) — beat tracking, key detection.

Forças em conflito:

- **Tamanho do repositório**: modelos >10 MB comprometem clonagem, CI e práticas Git.
- **100% offline**: o sistema deve funcionar sem internet; modelos precisam estar disponíveis localmente.
- **Reprodutibilidade**: a versão exata do modelo deve ser rastreável para garantir saída determinística.
- **Segurança**: modelos baixados da internet podem ser adulterados; integridade deve ser verificada.
- **Experiência do usuário**: o primeiro uso deve ser simples, sem exigir download manual de arquivos.
- **Customização**: usuários avançados podem querer usar modelos alternativos (maiores, especializados, ou fine-tuned).

## Decisao

Modelos são baixados sob demanda na primeira execução via script de instalação, com verificação de integridade e cache local.

1. **Script de instalação** (`scripts/install_models.sh` / `install_models.ps1`):
   - Baixa os modelos padrão de um registry versionado.
   - Verifica SHA256 checksums antes de salvar em disco.
   - Suporta `--offline` (usa apenas cache existente, falha se ausente).
   - Suporta `--model-path` para diretório personalizado.

2. **Cache directory**:
   - Linux/macOS: `~/.local/share/ai-midi-composer/models/`
   - Windows: `%LOCALAPPDATA%\ai-midi-composer\models\`
   - Estrutura: `<cache>/gguf/`, `<cache>/onnx/`, `<cache>/audio/`
   - Configurável via variável de ambiente `AIMIDI_MODELS_DIR`.

3. **Model registry** (`engine/cpp/data/model_registry.json`):
   - Mapeia nome do modelo → URL + SHA256 + tamanho + formato.
   - Versionado no repositório Git (arquivo pequeno, JSON).
   - Permite adicionar novos modelos sem alterar código.

4. **Default model**: Llama-3.2-3B-Instruct Q4_K_M (~2 GB).
   - Escolha baseada em: tamanho (cabe em 4 GB RAM), qualidade de instrução, licença permissiva (Llama 3.2 Community License).
   - Se o modelo não estiver presente, o sistema usa fallback rule-based (parser de palavras-chave).

5. **Fallback rule-based**:
   - Se nenhum modelo LLM estiver disponível, o PromptInterpreter usa um parser determinístico que extrai intenção musical via correspondência de palavras-chave (tempo, gênero, instrumentos, emoção).
   - Garante que o sistema nunca fique inoperante por falta de modelo.

6. **Custom models via environment variable**:
   - `AIMIDI_MODEL_PATH` — caminho para GGUF alternativo (ignora default).
   - `AIMIDI_ONNX_MODELS_DIR` — diretório com ONNX classifiers customizados.

7. **Offline/air-gapped install**:
   - Usuário baixa os modelos manualmente do release page do GitHub.
   - `install_models.sh --offline --source ./my-models/` copia do diretório local.
   - Checksums são verificados da mesma forma.

## Alternativas consideradas

- **A. Bundlar modelos no repositório Git.**
  - Vantagens: tudo em um lugar, sem download extra, CI tem os modelos.
  - Por que não: repositório passaria de ~100 MB para ~3 GB; `git clone` inviável; LFS adiciona complexidade e nem todos provedores de Git suportam bem; viola boas práticas de versionamento de binários grandes.

- **B. Gerenciador de pacotes (apt, brew, choco) como dependência.**
  - Vantagens: integração com gerenciador de pacotes do sistema, atualizações via package manager.
  - Por que não: não disponível em todas as plataformas de forma consistente; exige empacotamento separado para cada sistema; versão do modelo fica acoplada à versão do pacote; dificulta usuários testarem modelos diferentes.

- **C. Primeiro acesso baixa automaticamente (sem script).**
  - Vantagens: zero ação do usuário, experiência mais fluida.
  - Por que não: menos transparência (o que está sendo baixado?); sem verificação explícita de checksum no fluxo principal; difícil de interromper se a rede estiver lenta; viola princípio de surpresa mínima.

- **D. Docker image com modelos inclusos.**
  - Vantagens: ambiente completamente isolado e reproduzível, modelos já inclusos.
  - Por que não: adiciona dependência de Docker; overhead de ~500 MB+ pela imagem base; não atende usuários que executam o plugin VST3 nativo na DAW (o caso de uso principal); útil apenas para desenvolvimento/CLI.

## Consequencias

- **Positivas:**
  - Repositório Git permanece leve (~100 MB com assets menores, sem modelos).
  - Primeira execução é simples: um comando (`./scripts/install_models.sh`) e o usuário está pronto.
  - SHA256 checksums garantem integridade dos modelos baixados.
  - Fallback rule-based garante que o sistema nunca fique inoperante.
  - Customização via environment variable é simples e documentada.
  - Model registry versionado permite rastrear qual versão do modelo foi usada para cada composição (seed + model hash = reproduzibilidade completa).
  - Offline/air-gapped install tem caminho claro e suportado.

- **Negativas:**
  - Requer internet para o primeiro setup (~2 GB para o modelo padrão).
  - Script de instalação precisa ser mantido atualizado com URLs e checksums.
  - Usuário aguarda download antes do primeiro uso (~5–15 minutos dependendo da conexão).
  - Falha de rede durante download interrompe o setup (com retry e resumable download).

- **Neutras:**
  - Modelo pode ser trocado sem recompilar nada (basta variável de ambiente).
  - CI não testa com modelo real (apenas fallback rule-based), a menos que job específico seja criado.
  - Download pode ser integrado à interface do plugin no futuro (barra de progresso + botão de download).

## Relacionado

- ADR-0005: Integração LLM Local (llama.cpp) — define o mecanismo de chamada ao modelo.
- ADR-0001: Boundary C++ ↔ Go — gRPC boundary que transporta o resultado da inferência.
- Skills: `model-safety`, `llama-cpp`, `onnx-runtime`, `input-validation`.
- Roadmap: Fase 7.3 — release v0.1.0 hardening.
