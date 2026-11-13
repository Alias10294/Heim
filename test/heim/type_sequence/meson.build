test_sources = files(
    'access.cpp',
    'boundaries.cpp',
    'combinatorics.cpp',
    'composition.cpp',
    'conversion.cpp',
    'folding.cpp',
    'invariants.cpp',
    'members.cpp',
    'mutation.cpp',
    'partition.cpp',
    'smoke.cpp',
    'sorting.cpp',
    'traits.cpp',
    'transformation.cpp'
)

test_executable = executable(
    'heim-test-type-sequence',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'type-sequence',
    test_executable,
    suite: ['unit', 'heim-type-sequence'])
