# Prompt: Gerar Documentação

> Use para adicionar/atualizar documentação técnica para um módulo, ADR, workflow
> ou benchmark.

## Decisao do agente - quando `generate_documentation` vs criar manualmente

- Modulo novo: **gerar doc tecnica** (`docs/architecture/modules/<domain>-<module>.md`).
- Decisão arquitetural nova: **gerar ADR** (`docs/adr/NNNN-...`).
- Workflow novo: **gerar workflow doc** (`docs/workflows/<name>.md`).
- Bench novo: **documentar procedimento + resultados** (`docs/benchmarks/...`).

## Templates

Todos templates ja seguem `doc-standards` skill. Documentos:

1. **Module doc**
   ```md
   # <Domain>/<Module>
   ## Objetivo
   ## Dependências
   ## Interface públic
   ## Fluxo de execução
   ## Determinismo (seed)
   ## Testes & Golden
   ## Benchmarks
   ## ADRs relacionados
   ```

2. **ADR** - ver `adr-format` skill.

3. **Workflow doc**
   ```md
   # Workflow <N>: <nome>
   ## Objetivo
   ## Entradas (UI & contexto)
   ## Saídas (MIDI/context updates)
   ## Agentes envolvidos (e subagentes)
   ## Fluxo
       1. ...
   ## Seed e reproducibilidade
   ## Casos de borda
   ## Testesgolden
   ## Exemplo de uso
   ```

4. **Benchmark**
   ```md
   # Bench: <nome>
   ## Setup
   ## Comando
   ## Resultados
       ### Antes
       ### Depois
   ## Conclusão
   ```

## Regras

- Nao duplicar conteúdo; referenciar com link.
- Idioma PT-BR (compativel com narrativa do projeto). Para ADRs/code/PR preferimos
  Ehlish (ver `standards/documentation.md`).
- Diagramas em PlantUML, em `docs/diagrams/`.

## PR title

`docs(<scope>): atualiza doc de <modulo|feature>`
