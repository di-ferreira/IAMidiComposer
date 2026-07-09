# Realtime Audio — AI MIDI Composer

> Documento inviolável. Tudo que viola é automaticamente rejeitado pela QA.

## Regras absolutas na Audio Thread

### 1. Nunca aloque na heap

- `new`, `malloc`, `std::vector::push_back`, `std::string` (de tamanho variável),
  `std::shared_ptr` (refcount atomic), `std::function` (chamada virtual) — todos
  proibidos.
- Use `std::array`, `std::span`, arena pre-alocada, object pool, lock-free FIFO.

### 2. Nunca utilize mutex

- `std::mutex`, `std::recursive_mutex`, `std::shared_mutex`, `std::condition_variable`:
  bloqueiam latência.
- Use `std::atomic` (`release`/`acquire`/`relaxed` conforme necessidade) e
  lock-free SPSC ring.

### 3. Nunca realize IO

- Disk (files), rede (gRPC), logs (`std::cout`, spdlog), DB (`sqlite`) — todos
  em message thread ou worker.

### 4. Nunca chame IA

- Nenhuma chamada a llama.cpp / ONNX em audio thread. A IA roda na ACE, em worker
  threads separadas; resultado chega via FIFO.

### 5. Nunca aguarde future

- `future::get()`, `condition_variable::wait()`, lock-free atomic busy spin long)
  em audio thread.

## Politica de conformidade

- Code review por **Performance Engineer** PRs que tocam `processBlock`.
- CI: probe de glitch detect (`AudioProcessor`) com buffer random em 1k iter;
  tempo worst case < 5ms por block.
- Tracy: marcar `ZoneScopedN` em cada ramo de processBlock; consultants com zone
  > 1ms merece analysis.

## Limites práticos (Plugin)

- Buffer block 32-1024 samples com buffer size configurável.
- Parâmetros atomic `std::atomic<float>`; APVTS handles.
- MIDI out: SPSC FIFO pre-alocada, drenada em processBlock.

## Encadeamentos permitidos

```
Message Thread → atomic / ValueTree → Audio Thread
Audio Thread → SPSC FIFO → Message Thread (MIDI events out, status pingback)
```
