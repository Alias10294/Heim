# Heim sparse tests: exception policy

These tests model the invariant-restoration policy used by `std::flat_set`
and `std::flat_map` (C++ draft: [flat.set.overview] and [flat.map.overview]).

On success, exact effects are asserted. On failure, the destination may be
unchanged, empty, or partially changed **provided it is valid**. There is no
implicit strong exception guarantee for copy assignment or `replace`.
For copy assignment, the source must remain unchanged.

- `sparse_index`: each *observable* copied index entry has the expected value;
  independent old entries are allowed; `clear` and fresh writes must work.
- `sparse_set`: model the currently observable dense keys and cross-check
  `size`, uniqueness, index membership, `find`, and absent key probes.
- `sparse_map`: model currently observable key/value pairs and cross-check
  dense iteration, exact key/value association, index membership, `find`,
  `at`, uniqueness, and probes. Inconsistent key/value sizes must fail, not
  be treated as an acceptable empty result.
- Every test resets the allocation-failure injection *before* checking
  the container, clears it, reinserts, and checks correct release of pages.

These are test-only edits. Do not execute the production `apply_hardening.py`
from an earlier overlay if `include/heim/common/sparse.hpp` was modified
manually.

The current repository version of production `sparse.hpp` cannot be inferred
from this archive; run the test suite against the user's local implementation.

Standard references:
- https://eel.is/c++draft/flat.set.overview
- https://eel.is/c++draft/flat.map.overview
