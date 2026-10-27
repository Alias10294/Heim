# Heim test layout

`test/` is rebuilt incrementally. **Only the tests under `test/heim/common/`
are registered at present.** No old tests for `meta`, `property`, `graph`,
or `ecs` are expected to exist.

## Organization

```text
test/
├── meson.build              # Catch2 dependency and entry points
├── heim/
│   ├── meson.build          # Library-level dispatcher
│   └── common/
│       ├── meson.build      # One Catch2 executable per feature
│       ├── sparse_index/storage.cpp
│       ├── sparse_set/operations.cpp
│       ├── sparse_map/operations.cpp
│       └── unique_allocated_ptr/lifetime.cpp
└── distribution/
    └── meson.build          # Empty for now; future packaging checks
```

Functional tests include the **regular library headers** (`<heim/...>`)
through `heim_dep`. They do not include files from `single_include/`.
The `distribution` directory intentionally has **no tests** yet.

Each feature is registered separately through `test/heim/common/meson.build`,
under the `heim-common` (and `unit`) Meson suites. The feature test code has
not been changed by this Meson-only correction.

## Commands

If the build directory was already configured with `-Dheim_tests=true`:

```sh
meson setup build --reconfigure
meson compile -C build
meson test -C build --suite heim-common --print-errorlogs
meson test -C build --print-errorlogs
```

If `heim_tests` is now **off** in the existing build configuration, enable it:

```sh
meson configure build -Dheim_tests=true
meson compile -C build
meson test -C build --suite heim-common --print-errorlogs
```

The `subprojects/catch2.wrap` dependency and the top-level `heim_tests`
option are unchanged. When tests are off, Catch2 is not requested.

## Planned workflow

Expand a single feature's test suite, run it and the whole suite, then commit
that feature's passing tests. Add the next library to `test/heim/meson.build`
**only once its test sources exist**. Distribution tests will be introduced
separately, rather than being required for these initial unit-test commits.

The previously identified array-deletion regression in
`unique_allocated_ptr/lifetime.cpp` is intentionally hidden pending an
implementation fix; passing the enabled tests does not resolve that issue.
