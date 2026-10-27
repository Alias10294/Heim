# Tests for `heim::detail::sparse_index`

Run with `meson test -C build unit-common-sparse-index --print-errorlogs`.

The suite exercises paged and contiguous modes; initialization and sentinel
semantics; page boundaries, holes and distant keys; deep copy and move; ranges;
allocator ownership, propagation, and exception injection; self-assignment;
and deterministic model tests.

`advanced_regressions.cpp` injects *later* page-copy failures both with and
without POCCA. It does not impose a strong exception guarantee: after a failed
copy the destination may retain old or partial new mappings, but must remain
valid, destructible and reusable; the source remains intact. It also tests
reusing the source after unequal-allocator move assignment.

The sentinel is the largest representable `value_type` and is not a valid
stored index, although it remains a valid sparse **key**. `get` on missing
keys returns a null pointer; tests never dereference it.

Quantitative code coverage must be measured with compiler instrumentation.
