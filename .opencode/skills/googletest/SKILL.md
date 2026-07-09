---
name: googletest
description: GoogleTest no C++ - TEST/TEST_F, EXPECT vs ASSERT, fixtures com Setup/TearDown, mocks via gmock; golden files binary compare com tolerancia.
---

# googletest

## Padrao

```cpp
TEST(HarmonyEngineTest, II_V_I_in_C) {
    HarmonyEngine eng(default_config);
    auto prog = eng.generate(C_major, kSeed42, /*bars=*/4);
    ASSERT_THAT(prog.chords(), ElementsAre(...));
}
```

- TEST_F para fixtures (configuraao repetitiva).
- EXPECT_* quando nao bloqueia o restante; ASSERT_* quando invalida o continuar.
- gmock para mocks de I... interfaces.

## Golden tests

```
auto bytes = sermon::render(composition);
EXPECT_EQ(bytes, readFileBytes("tests/golden/II_V_I_C_seed42.mid"));
```

- Reproduzivel byte-a-byte graças á seed.
- Tolerância parametrizável para humanizaç (quando appk).

## CI

- `ctest --output-on-failure` em cada plataforma; coleta gcov/lcov.
