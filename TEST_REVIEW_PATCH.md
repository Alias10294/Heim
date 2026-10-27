# Heim test-audit corrective overlay

Apply the ZIP at the repository root. No production `include/heim/` header is
modified. The patch is deliberately an **overlay**: only changed or new test
files and CI configuration are included.

## Main improvements

1. Dense/sparse stale-index probes cover the modeled key domain and can also
   accept explicit keys visited by a scenario; no expensive 64-bit full scan.
2. Model tests report the random seed, operation and key. Replacement models
   derive the oracle from the input before moving the input.
3. `sparse_map` allocators log their four rebound storage categories separately,
   and clean up real allocations if recording them throws. Likewise for the
   `unique_allocated_ptr` allocation log.
4. Fresh insertion injects failures into every observed allocation point.
   Failed `replace` with and without `unique` is verified separately.
5. Moved-from objects are checked for consistency and reuse after construction,
   including distinct allocator IDs.
6. `type_sequence` now tests public **member** aliases for empty, singleton,
   multiple and repeated sequences. This catches the previously reported
   `type_sequence<>::reverse` mutation.
7. Old inaccurate docs and missing feature tags are fixed.
8. Adds independent Windows/MSYS2 GCC unit-test CI.

## Not claimed

- This does **not** measure branch coverage. Use a coverage build for a number.
- Exception guarantees for copy assignment and throwing mapped-value move
  assignment have not been strengthened by changing the library code. Such
  cases may expose genuine container invariant defects and deserve explicit
  API-guarantee decisions before enforcing a particular post-exception state.
- MSYS2 CI depends on its current GCC providing C++26 Contracts support.

## Remove obsolete tracked test files

ZIP extraction cannot delete them. From the Git repository root:

```sh
git rm -- test/heim/common/sparse_set/operations.cpp test/heim/common/sparse_map/operations.cpp
```

If Git reports a file is not tracked, simply check the path and delete any
stale local copy manually.

## Validate before committing

```sh
meson setup build --reconfigure -Dheim_tests=true
meson compile -C build
meson test -C build --print-errorlogs
```

Only commit after the suites pass. Investigate a failing moved-source or
exception-injection test as a potentially real invariant defect; do not silently
weaken assertions to make tests pass.
