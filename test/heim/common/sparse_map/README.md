# `heim::sparse_map` test suite

Feature-scoped Catch2 executable: `unit-common-sparse-map`, Meson suite `heim-common`.
All tests include the normal `<heim/common/sparse.hpp>`; no amalgamated header is involved.

## Coverage map

| File | Focus |
|---|---|
| `traits.cpp` | Exposed types, reference/proxy iterator semantics |
| `basic.cpp` | Empty behavior, `at`, `find`, `[]`, sparse keys |
| `construction.cpp` | Container, iterator, range, unique-tag and allocator overloads |
| `insertion.cpp` | All insertion/assignment API families; duplicate behavior |
| `erasure.cpp` | Key, iterator and interval erasure; swap-and-pop invariant |
| `iterators.cpp` | Const/mutable, reverse and random-access navigation |
| `lifecycle.cpp` | Copy/move/assignment/swap/equality/replace/extract, self-move |
| `mapped_values.cpp` | Strings, move-only/non-default constructible types, destruction |
| `allocators.cpp` | Rebound allocators, propagation, ownership and no leaks |
| `pmr.cpp` | Polymorphic allocator resources and allocation failure |
| `exceptions.cpp` | Allocation/construction failure paths and recovery |
| `limits.cpp` | Index widths, page sizes, full capacity and sentinel values |
| `model.cpp` | Reproducible operation sequences against `unordered_map` |
| `constexpr.cpp` | Empty constexpr; runtime checks until contract compiler fixes |

## Assumptions and expected failures

- `emplace` is called **only for absent keys** (required by contract).
- `erase(key)` is called **only for present keys**; `try_erase` handles missing keys.
- The `heim::unique` tag is only used with already-unique keys and correctly
  aligned key/value arrays.
- After self-move, **contents are unspecified**, but the public index and
  dense storage must agree; the test compares membership with iteration.
  This detected a real bug in `sparse_set` previously and may require the
  analogous guard in `sparse_map` (whose move assignment is currently defaulted).
- `constexpr` mutation `static_assert`s are GCC 17+ only because GCC 16
  with Contracts can report `contract condition is not constant` (PR125459).
  Runtime checks remain active for every supported compiler.
- Running this suite is not equivalent to measured statement/branch coverage;
  enable Meson coverage instrumentation for quantitative metrics.

## Commands

```sh
meson setup build --reconfigure
meson compile -C build
meson test -C build unit-common-sparse-map --print-errorlogs
meson test -C build --suite heim-common --print-errorlogs
```

Keep tests deterministic; report a random seed and operation step in new
model-based tests when adding reproduction cases.
