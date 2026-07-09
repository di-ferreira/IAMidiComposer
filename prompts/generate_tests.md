# Prompt: Gerar Testes

> Use para adicionar tests a um modulo existente que nao tem cobertura suficiente.

---

## Etapas

1. Leia o modulo alvo (`<file>.cpp/.go`).
2. Liste funções/paths em falta de testes.
3. Para cada um:
   - Identifique comportamento (nao internals)
   - Escrever teste unit cub tabumasual se possivel
   - Adicionar golden se saida é MIDI/JSON/proto
   - Adicionar bench se hot path
4. Rode testes para validar.

## Regras

- **Apenas testes** (nao mudar implementacao; se bug encontrado, abrir outro PR).
- Use fixtures (TEST_F em C++; table-driven em Go) quando repetitive.
- Nome de teste descreve cenario, nao internals.
- Não mock exagerado; prefira real cadastra objectos; criar ad-hoc hanya quando custoso.
- Inclua casos de erro: empty input, malformed, boundary cases.

## Golden MIDI

```cpp
TEST_F(EngineGolden, GeneratesII_V_I_in_C_with_seed_42) {
    auto blueprint = make_blueprint(C, Major, II_V_I, ...);
    auto events = engine.generate(blueprint, /*seed=*/42);
    auto bytes = renderer::smf1(events);
    EXPECT_TRUE(golden::matches("II_V_I_C_seed42.mid", bytes));
}
```

- Atualizar golden: PR com `-DUPDATE_GOLDENS=ON` + reproducible.

## Cobertura靶

- Apos gerar, rodar coverage e anexar print/diff.
- Se ainda < minima (ver `standards/testing.md`), documente onde falta.

## PR title

`test(<scope>): adiciona cobertura de <comportamento>`
