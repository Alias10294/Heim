# `heim::unique_allocated_ptr` — tests

This is an isolated, feature-specific test suite. It includes the regular public
header `<heim/common/unique_allocated_ptr.hpp>` and does not use amalgamations.

## Scenarios

| File | Coverage goals |
|---|---|
| `lifetime.cpp` | Original smoke tests, including the previously disabled array-destruction regression (now enabled) |
| `traits.cpp` | Type and deleter constraints, bounded-array rejection, null deletion, compile-time properties |
| `single.cpp` | Argument forwarding, immovable/move-only objects, ownership, move, swap, reset, release, overwrite |
| `arrays.cpp` | Zero/one/many elements, value initialization, reverse destruction, array extents, ownership operations |
| `allocators.cpp` | Stateful allocator ownership, matching allocation/deallocation, `allocator_traits` hooks, swaps/moves |
| `exceptions.cpp` | Allocation failures, constructor failures, allocator-hook failures, partial-array rollback at every element |
| `deleters.cpp` | Direct invocation, null behavior, normal vs overwrite hooks, copied/moved deleters and extents |
| `fancy_pointer.cpp` | Custom nullable allocator pointer type in single/array and normal/overwrite operations |
| `memory_resource.cpp` | `std::pmr::polymorphic_allocator`, byte sizes, alignment, zero-length allocation behavior |
| `stress.cpp` | Fixed-seed sequences of ownership operations and allocator/lifetime invariants |
| `constexpr.cpp` | Constant evaluation of ordinary single/array allocation |
| `alignment.cpp` | Over-aligned class types in all four factory overload families |
| `support.hpp` | Local test fixtures: tracking allocator, lifetime and failure injection |

The suite deliberately *does not* read the indeterminate values of scalar
objects returned by `allocate_unique_for_overwrite` before assigning them.

## Run

After unpacking at the repository root:

```powershell
meson setup build --reconfigure
meson compile -C build
meson test -C build unit-common-unique-allocated-ptr --print-errorlogs
```

For all tests of `common`:

```powershell
meson test -C build --suite heim-common --print-errorlogs
```

To select the Catch2 tests directly, run `build/test/heim/common/heim-test-common-unique-allocated-ptr.exe`
with a Catch2 test-name or tag filter (e.g. `[exception]`, `[array]`, `[stress]`).
The executable's precise path may differ depending on Meson/Ninja output naming.

## Limits

Passing tests, including the ASan/UBSan tests, is not a measured claim of
100% statement or branch coverage. Measure coverage on the installed GCC 16
build to find any remaining reachable branches. Pathological allocator
operations that throw from a `noexcept` deleter, allocation sizes approaching
`max_size()`, and allocator copies/moves that throw are not executed as ordinary
in-process unit tests because some lead to `std::terminate` or impractical
allocations; these require separate death/process-level tests or an explicit API
contract decision.
