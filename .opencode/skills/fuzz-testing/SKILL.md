---
name: fuzz-testing
description: Fuzz testing em C++ (libFuzzer) e Go (native fuzz) - parsers MIDI/projeto recebem fuzzing; corpus em tests/fuzz_corpus/. Acha edge cases de parser.
---

# fuzz-testing

## Targets

- C++ SMF/MIDI parser, parser de projeto JSON/proto.
- Go gRPC request parser; proto decode path.

## Go (native)

```go
func FuzzParseProject(f *testing.F) {
    f.Add([]byte("...seed..."))
    f.Fuzz(func(t *testing.T, in []byte) {
        if _, err := parse(in); err != nil {
            // Esperado: erro bem-comportado, nao panic.
        }
    })
}
```

- Corpus gitignored; seed corpus em `tests/fuzz_corpus/`.

## C++ (libFuzzer)

```cpp
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    aimidi::midi::parser::parse({data, size});  // nao throw, nao crash
    return 0;
}
```

- Build com `-fsanitize=fuzzer,address`; CI job dedicado.
- ASan/UBSan combinados;deaux crashes + falsa overflow.
