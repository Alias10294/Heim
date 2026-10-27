# Tests de `heim::sparse_set`

Battery of **121 independent Catch2 `TEST_CASE`s** (ordinary `<heim/common/sparse.hpp>` include, no amalgamated headers). Each test belongs to the `heim-test-common-sparse-set` executable and `[common][sparse_set]` tag group.

## Coverage

| File | Focus |
| --- | --- |
| `traits.cpp` | Public aliases, iterator/range concepts, compile-time interface, page-size constants |
| `basic.cpp` | Empty state, sparse keys, const lookup, repeated clear/reinsertion, paged vs contiguous |
| `construction.cpp` | Initializer lists, vectors, `unique_t`, `std::from_range`, iterators, one-pass streams |
| `insertion.cpp` | `insert`, `try_emplace`, `emplace`, hints, duplicates, input ranges, many keys |
| `erasure.cpp` | Key/iterator/range erase, returned iterators, swap-with-back bookkeeping, boundary keys |
| `iterators.cpp` | Reverse dense traversal, const/nonconst, random-access behavior, views, `extract` |
| `lifecycle.cpp` | Copy, move, self-assignment, swap, equality, `replace` and `replace(unique, ...)` |
| `allocators.cpp` | Owner-checked rebind allocators, page lifetimes, allocator propagation (POCCA/POCMA/POCS), replacement |
| `pmr.cpp` | `polymorphic_allocator`, independent PMR resources, release of memory and replacement |
| `exceptions.cpp` | Injected failures in sparse page, dense vector, contiguous index, construction and replacement; recovery and leak checks |
| `limits.cpp` | `uint8_t`, `uint16_t`, page sizes 0/1/7/8/64, sentinel/max-size boundaries |
| `model.cpp` | Multiple fixed-seed independent `std::unordered_set` models, deduplication, replacement, churn and randomized operations |
| `constexpr.cpp` | Compile-time functionality for paged and contiguous storage |

`support.hpp` keeps feature-specific utilities and allocator diagnostics local to this feature. Validation checks exact dense membership, uniqueness, both iterator directions, `find` and `contains`; it also probes missing keys to detect stale sparse/dense mappings.

## Contracts and interpretation

- The public `insert`/`try_emplace` functions are used for duplicate keys. The direct `emplace` and `emplace_hint` implementation requires the inserted key to be absent; tests respect that precondition.
- `erase(key)` is called only for present keys. `try_erase(key)` covers missing keys.
- The `unique_t` overload is supplied *only with genuinely unique input*; it is not a deduplication request.
- Iterator erase is tested with valid iterators, including `erase(begin(), end())`; tests do not retain invalidated iterators after mutating the vector.
- `std::vector` self-move has an unspecified value, but the resulting `sparse_set` must retain a valid internal index. The self-move test checks consistency, not retained membership.
- `uint8_t::max` is valid as a **key**. The sparse index uses that same value as a sentinel for an **index entry**, so only `max_size()` keys can be present at once.
- No test claims that failed copy assignment provides the strong exception guarantee. Failure tests focus on existing-key survival for failed insertion/replacement and leak-free unwinding of constructors.
- Tests of nonpropagating allocators **never swap incompatible allocator owners**, which would otherwise violate the underlying container's allocator constraints.

## Running only this feature

```powershell
meson setup build --reconfigure
meson compile -C build
meson test -C build unit-common-sparse-set --print-errorlogs
```

Or the entire `heim-common` group:

```powershell
meson test -C build --suite heim-common --print-errorlogs
```

Once the archive has been extracted, remove the **old** file, which is no longer included in the Meson sources:

```powershell
git rm -- test/heim/common/sparse_set/operations.cpp
```

## Validation limits

These test translation units have been syntax-checked with GCC 14 and Clang 17 in C++23 mode against a **local API-compatible stand-in**. A core subset (88 tests) was exercised against that stand-in. This verifies the test code's structure and its independent reference model, **not the actual Heim implementation**. The real library requires GCC 16 and `<contracts>`, unavailable in this environment. You must compile and run this suite against your real checkout. The 121 cases do not imply 100% line/branch coverage without instrumentation.

If any test fails against Heim, keep its independent oracle or invariant intact until the behavior has been diagnosed. Treat an implementation bug differently from a wrong test assumption; retain reproducible cases as regression tests.
