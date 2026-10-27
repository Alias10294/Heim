# Heim tests

The functional suites compile the **regular** public headers under `include/heim/`.
They do not depend on `single_include` artifacts. The release/distribution suite
is independent and can be added separately.

## Layout

- `test/heim/common/`: `unique_allocated_ptr`, `sparse_index`, `sparse_set`, `sparse_map`.
- `test/heim/meta/type_sequence/`: compile-time type/value assertions and runtime smoke checks.
- `test/distribution/`: a separate Meson entry point (currently empty).

Each feature has one Catch2 executable, selected by its own Meson test name.

```sh
meson setup build --reconfigure -Dheim_tests=true
meson compile -C build
meson test -C build --print-errorlogs
meson test -C build --suite heim-common --print-errorlogs
meson test -C build --suite heim-meta --print-errorlogs
```

Most meta checks are `static_assert`s; they run **at compilation**, not as
separately enumerated Catch2 test cases.

An older GCC 16 Contracts constant-evaluation issue may prevent some mutation
`constexpr` assertions from compiling. Those assertions remain conditional,
with equivalent runtime tests. See the per-feature README and GCC PR125459.

## Maintenance

The old `test/heim/common/sparse_set/operations.cpp` and
`test/heim/common/sparse_map/operations.cpp` are historical leftovers and are
**not registered in Meson**. Remove them explicitly from Git; extracting a ZIP
cannot delete existing files:

```sh
git rm -- test/heim/common/sparse_set/operations.cpp test/heim/common/sparse_map/operations.cpp
```

For statistical coverage, build with `-Db_coverage=true` and inspect a coverage
report. Test counts alone are not a coverage percentage. Keep randomized models
reproducible and include seed, step, operation and key in diagnostics.
