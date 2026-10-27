# Heim final common/meta hardening overlay

Based on `Alias10294/Heim`, commit `72c8eb47b60251b5fe41318ddb66b516db02c270`.

## Applying the archive

1. Extract the archive into the **root of the repository** and allow it to
   replace test/CI/README files.
2. Because this archive intentionally does not overwrite the full 53 kB
   production `sparse.hpp` without verifying its current version, run:

   ```powershell
   py scripts/apply_hardening.py
   ```

   This checks each code anchor before changing `include/heim/common/sparse.hpp`;
   it is idempotent. It **automatically regenerates `single_include` headers**
   through the repository's generator when present. You can preview compatibility
   without changing files using `py scripts/apply_hardening.py --check`.
3. Build and execute tests:

   ```powershell
   meson setup build --reconfigure -Dheim_tests=true
   meson compile -C build
   meson test -C build --print-errorlogs
   python scripts/generate_single_include.py --check
   ```
4. Review `git diff` before committing; the GitHub connection used to prepare
   this ZIP did not permit remote write operations.

## Corrected test allocator

The initial overlay introduced allocation-budget regressions using `log->remaining`
without extending `sparse_index/support.hpp`. The installer now adds this field
and its guarded decrement and rejection logic. It is a test-support fix, not a Heim API change.

## Production fixes

- `detail::sparse_index`: staged paged copy assignment, no destruction before
  potentially failing copies, source cleared after successful move assignment.
- `sparse_set`/`sparse_map`: copy assignment respects allocator propagation,
  preserves invariants on failure, and move assignment clears the source.
- `extract() &&` consumes dense storage but leaves its source with an empty
  index and valid reusable state; conditional `noexcept`.
- `sparse_set` allocator-extended constructors use `make_obj_using_allocator`,
  enabling containers without `allocator_type`.
- Rebuilds amalgamated distribution headers to avoid generator drift.

## Tests and CI

- Added three advanced regressions and an allocator-mode matrix to the same
  five executable Meson layout.
- Adds separate POCCA/POCMA/POCS configurations and SOCCC.
- Adds `std::inplace_vector` tests when supported.
- Repairs MSYS2 CI dependency (`git`) and syntax-checks `single_include`.
- Updates global and feature README files.

## Limitations

- Not compiled here with GCC 16/Contracts: the local environment supplies GCC
  14 and does not provide the required `<contracts>` header. The real compilation
  and runtime results must be confirmed with the commands above.
- The suite does not imply exhaustive exception coverage or 100% code coverage.
- Mixed *allocator instances* for the different dense storages, and containers
  with exception-throwing swap or a nonstandard allocator model, still require
  separate design decisions about the library's contract.
- Source files that change after the audited commit can cause the patch's
  anchor checks to reject the update deliberately rather than silently applying
  an incorrect transformation.
