# Tests for `heim::sparse_map`

Run with `meson test -C build unit-common-sparse-map --print-errorlogs`.

The suite checks constructors, `std::from_range`, insertion and assignment,
deduplication, erasure, random-access iterators and their C++ concepts,
exceptional mapped-value types, allocator propagation, index-width limits,
model comparisons to `unordered_map`, and randomized deterministic sequences.

`advanced_regressions.cpp` adds:

- coherence and reuse of a source after `extract()`;
- unequal-allocator move assignment, including checks on the source;
- failure at multiple positions during copy assignment;
- propagating allocator copy-assignment failures;
- failure while copying old mapped values during vector growth;
- a mixed allocator-free/key-vector configuration when `std::inplace_vector`
  is implemented by the compiler's standard library.

`../sparse_set/allocator_modes.cpp` tests the allocator propagation trait matrix
for both sparse containers. `value_exceptions.cpp` checks mapped assignments
that throw, including the exception behavior of `erase`.

Preconditions: `heim::unique` is supplied genuinely unique input, direct
`emplace` is only used for absent keys, and direct `erase(key)` uses an existing
key. The key/value containers must maintain alignment. The moved-from object
is empty and reusable after a successful move assignment.

After copy-assignment or `replace` failure, tests apply the `std::flat_map`
exception-policy model: contents are unspecified but key/value alignment,
uniqueness, sparse index consistency and safe reuse must be preserved. A
successful copy still checks exact contents. Tests do not require staging via
swap or preservation of the old destination after an exception.

C++ Contracts GCC PR125459 currently prevents enforcing some mutation
`constexpr` assertions as `static_assert` under GCC 16. Both runtime variants
remain tested. Coverage percentages require compiler instrumentation.
