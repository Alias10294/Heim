# `heim::type_sequence` tests

All functional test files include the regular `<heim/meta/type_sequence.hpp>` header.
The primary checks are global `static_assert` expressions, deliberately separated by
operation family so a failing instantiation is attributable to one source file.
`smoke.cpp` adds Catch2 runtime checks for a few public value traits and provides
an executable test binary registered in the `heim-meta` Meson suite.

The suite covers trait values, exact result types, exposed member aliases, empty and
singleton boundaries, repeated/cv/ref/incomplete types, order preservation,
composition identities and deliberately unsupported operations detected with C++20
`requires` where practical. Large combinatorial expansions are bounded so CI
compile time stays predictable.

## Run

```sh
meson setup build --reconfigure
meson compile -C build
meson test -C build unit-meta-type-sequence --print-errorlogs
meson test -C build --suite heim-meta --print-errorlogs
```

No amalgamated headers are involved. A successful runtime exit is **not** sufficient
on its own: all compile-time assertions must first compile and link. This suite
checks semantics rather than line/branch coverage, which is not directly useful
for purely template metaprogramming.
