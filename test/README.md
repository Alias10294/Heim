# Heim tests

The tests use ordinary public headers from `include/heim/`, and are organized
as five feature-level Catch2 executables:

- `unique_allocated_ptr`, `sparse_index`, `sparse_set`, `sparse_map` under `test/heim/common/`;
- `type_sequence` under `test/heim/meta/`, mostly verified through `static_assert` during compilation.

## Build / run

```sh
meson setup build --reconfigure -Dheim_tests=true
meson compile -C build
meson test -C build --print-errorlogs
meson test -C build --suite heim-common --print-errorlogs
meson test -C build --suite heim-meta --print-errorlogs
```

`test/heim/common/meson.build` lists every source for each feature, including
the regression tests. `test/distribution/` is currently reserved for further
packaging tests; GitHub Actions separately checks generation and syntax-compiles
the amalgamated public header.

## What the hardening regressions cover

- Invariants and reuse after move construction, move assignment and `extract()`.
- Copy assignment under allocation failures, including allocation failures
  after more than one allocation has succeeded.
- `sparse_index` copy assignment with propagating and nonpropagating allocators.
- Distinct propagation traits (`POCCA`, `POCMA`, `POCS`) and allocator
  `select_on_container_copy_construction`.
- Containers without `allocator_type`, via `std::inplace_vector` on standard
  libraries providing `__cpp_lib_inplace_vector >= 202406L`.
- A mapped-value copy exception while reallocating `sparse_map`'s dense values.
- Independently generated sparse reference models, range support, iterator
  concepts and static metaprogramming member aliases.

## Exceptional states

With a nonpropagating allocator, `sparse_set`/`sparse_map` copy assignment
stages a replacement before exchanging it with the destination. With a
propagating allocator, copy assignment preserves the source and leaves the
destination **empty and reusable** if an intermediate operation throws. Move
assignment also clears both objects if a member operation throws. These are
explicit Heim policies tested by the regression suite, not claims about all
STL containers.

`extract() &&` consumes the storage and leaves the source empty and reusable.
The moved-out dense buffers are returned intact. The `noexcept` specification
of extraction is conditional on the dense container's move construction.

## Further validation

Counts of Catch2 assertions are *not* source-coverage percentages. If a
supported GCC 16+ and coverage tools are installed, use a separate build:

```sh
meson setup build-coverage -Dheim_tests=true -Db_coverage=true
meson compile -C build-coverage
meson test -C build-coverage --print-errorlogs
ninja -C build-coverage coverage-html
```

For memory instrumentation, use another compiler/toolchain providing Contracts
and ASan/UBSan; do not assume MinGW has usable sanitizers by default. Example:

```sh
meson setup build-sanitize -Dheim_tests=true -Db_sanitize=address,undefined
meson compile -C build-sanitize
meson test -C build-sanitize --print-errorlogs
```

Some mutation `constexpr` static assertions in sparse containers remain
conditional on GCC >= 17 because of GCC PR125459 involving Contracts. Their
runtime checks remain enabled under GCC 16.
