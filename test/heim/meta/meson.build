# Compile-time assertions live at namespace scope in independent translation units.
# Catch2 provides an executable entry point and runtime smoke checks.
type_sequence_sources = files(
  'type_sequence/traits.cpp',
  'type_sequence/members.cpp',
  'type_sequence/access.cpp',
  'type_sequence/composition.cpp',
  'type_sequence/transformation.cpp',
  'type_sequence/mutation.cpp',
  'type_sequence/partition.cpp',
  'type_sequence/combinatorics.cpp',
  'type_sequence/sorting.cpp',
  'type_sequence/folding.cpp',
  'type_sequence/conversion.cpp',
  'type_sequence/invariants.cpp',
  'type_sequence/boundaries.cpp',
  'type_sequence/smoke.cpp',
)

meta_type_sequence_exe = executable('heim-test-meta-type-sequence',
  type_sequence_sources,
  dependencies: [heim_dep, catch2_main_dep])
test('unit-meta-type-sequence', meta_type_sequence_exe,
  suite: ['unit', 'heim-meta'])
