# Performance — AI MIDI Composer

> Toda mudança em hot path requer benchmark. Sem benchmark = PR rejeitado.

## Categorias de caminho

| Categoria     | Latência alvo (p99)  | Onde                             |
|---------------|----------------------|-----------------------------------|
| Audio Thread   | < 1 ms por block     | `processBlock`                    |
| UI/Message     | < 16 ms (60Hz)       | repaint, dispatcher de UI         |
| Workflow total | depende (UX)         | tempo de geração completo         |
| MIDI render   | < 50 ms para 1k comps| `MidiRenderer::render`            |

## Métricas padrão

- **Throughput**: `x ops/s` ou `x bytes/s`.
- **p50 / p99 / p99.9 latência**.
- **Cache-miss/1k inst** (perf stat).
- **Allocs/avgRunLoop** (Tracy memory) para region de hot path.
- **Memória peak**.

## Obrigatório em PR de hot path

1. **Antes**: capturar metricas no branch base.
2. **Depois**: capturar mesmas metricas com a mudança.
3. **Documento** em `docs/benchmarks/<feature>.md` com:
   - Setup (HW/SW/version).
   - Tabela das métricas.
   - Conclusão (regrediu? melhorou? equivalente?).
   - Comando para reproduzir.

## Gates CI

- Nenhum regressão de > 5% em throughput sem justificative ADR.
- Latência de audio p99 deve permanecer < 1ms (run-size 1024).
- Memória peak nao deve crescer > 10%.

## Ferramentas

- Tracy (C++) · pprof (Go) · hyperfine (CLI) · perf record/report.
- Coleta automatica em CI: job `benchmarks-release` roda em runner dedicado.
