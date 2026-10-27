# `heim::detail::sparse_index` — tests

This directory tests the internal `sparse_index` implementation from the **ordinary public/source include tree** (`<heim/common/sparse.hpp>`). It does **not** use `single_include` and does not change the implementation.

## Scope

- API / compile-time properties, `page_size`, `is_paged`, unsigned value types, allocator traits, sentinel and `max_size`.
- Basic lookup and mutable/const access; sentinel transitions, repeated writes and clears.
- Page sizes 1, 2, 3, 7, 8, 16, 31, 64 and the project default; page boundaries, holes and distant keys.
- Lazy page allocation, unchanged per-page addresses on page-table growth, and release of page storage.
- Contiguous storage growth and missing-value initialization.
- Range construction/reset, duplicates (last position wins), empty, forward and single-pass input ranges.
- Copy/move construction and assignment, including self-assignment, independent copies, swap, explicit allocator arguments, POCCA / POCMA / POCS.
- A rebind-aware stateful allocator checks allocation/deallocation balance, ownership and page count.
- Injected allocation failures, preserving preexisting entries where applicable, subsequent recovery, and no leaks.
- Fixed-seed reference-model tests over thousands of mixed operations for different index widths and page sizes.

**Important semantics:** the maximum representable `value_type` is a *missing-value marker*, not an ordinary accessible stored value. The maximal **key** can still be used with a smaller stored value. Tests do not dereference a pointer returned by `get` for a missing key.

## Meson

The existing `heim-common` suite retains **one executable for this feature** (`heim-test-common-sparse-index`). The executable is built from the `.cpp` files listed in `test/heim/common/meson.build`.

```powershell
meson setup build --reconfigure
meson compile -C build
meson test -C build unit-common-sparse-index --print-errorlogs
meson test -C build --suite heim-common --print-errorlogs
```

If your previous feature implementation still exists in `test/heim/common/sparse_index/storage.cpp`, remove that old file from the repository: it is intentionally **not** part of the new target.

```powershell
git rm -- test/heim/common/sparse_index/storage.cpp
```

## Validation caveat

The tests were syntax-checked and exercised locally under GCC 14 and Clang 17, including ASan/UBSan, against a **minimal API-compatible test stand-in**, because this environment does not provide GCC 16 with `<contracts>`. This **does not replace** compiling/running them against the actual Heim source with your GCC 16.2 / Meson / Catch2 setup.

101 Catch2 `TEST_CASE`s are provided. That is not a statement of 100% source/branch coverage. For measured coverage, run a compiler-supported coverage build of the *real* implementation, and assess unexecuted branches. Keep any implementation regressions as new permanent test cases.
