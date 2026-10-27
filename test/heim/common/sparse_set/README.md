# Tests for `heim::sparse_set`

Run with `meson test -C build unit-common-sparse-set --print-errorlogs`.

The suite covers construction, insertion, duplicate keys, erasure, iteration,
limits and pages, memory-resource and allocator propagation, copy/move, sparse
index validity, replacement, stress/model tests and exceptions.

`advanced_regressions.cpp` adds checks that extracting the dense storage leaves
a usable empty source; move assignment with unequal nonpropagating allocators
leaves a coherent source; and multiple failure points in copy assignment keep
all sparse/dense invariants. It exercises allocator-free containers where the
standard library provides `std::inplace_vector`.

`allocator_modes.cpp` tests independent POCCA, POCMA and POCS configurations,
and allocator selection on copy construction, on both sparse containers.

The `heim::unique` tag requires unique input. `emplace` is called only for
absent keys, and direct `erase` only for present keys. Iterator invalidation
follows the dense container's requirements.

For failing copy assignment and `replace`, the suite follows the `std::flat_set`
exception-policy model: invariants are restored, but the destination's previous
contents need not survive. Tests derive an oracle from the observed dense keys,
then independently check uniqueness and every sparse lookup. They check source
preservation on copy, destination reuse and allocator ownership. No copy-and-swap
or strong exception guarantee is assumed.

`constexpr` mutations are runtime-tested under GCC 16 until GCC PR125459 is
resolved; counting assertions does not establish line/branch coverage.
