---
name: go-testing
description: testing em Go - tabela-driven tests, t.Parallel, helpers, sem globals; golden JSON checks com cmp; benchmark com b.ReportMetric.
---

# go-testing

## Padrao

```go
func TestSomething(t *testing.T) {
    t.Parallel()
    tests := []struct{
        name string
        in   Input
        want Output
    }{
        {"x", ..., ...},
    }
    for _, tc := range tests {
        tc := tc
        t.Run(tc.name, func(t *testing.T) {
            t.Parallel()
            got := Run(tc.in)
            if diff := cmp.Diff(tc.want, got); diff != "" {
                t.Errorf("mismatch: %s", diff)
            }
        })
    }
}
```

- Use `google/go-cmp/cmp` para diffs legiveis.
- `t.Helper()` em helpers de assert.
- Evite state mutante entre tests.
- Benchmark: `b.ReportMetric(nsPerOp, "ns/op")`.
