---
name: golden-tests
description: Golden files - saidas MIDI (e proto MSG) persistidas; cada alteracao de Music Theory Engine deve produzir o mesmo golden dada seed; atualizar golden requerre revisão musical.
---

# golden-tests

## Quando usar

- Music Theory Engine: dada seed + blueprint, MIDI esperado.
- Prompt Interpreter: dado prompt, JSON esperado (tolerância model variance).
- SMF writer: saida binaria dada entrada canonica.

## Padrao C++

```cpp
TEST_F(HarmonyGolden, II_V_I_C_seed42) {
    auto bytes = render(...);
    ASSERT_TRUE(bytes_match_or_die(golden("II_V_I_C_seed42.mid"), bytes));
}
```

- Helper `golden("path")` carrega de `tests/golden/`.
- Helper de comparacao tolerente: norm-duration diff <= 1 tick para humanisação.
- Atualizar golden: `-DUPDATE_GOLDENS=ON` no CMake + revisão manual do MIDI.

## Outras saidas

- `.proto` text -> geração texto; golden compare; diff revision.
- JSON blueprint -> golden JSON com diff tolerante em ordering de arrays.
