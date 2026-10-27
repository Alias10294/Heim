test_sources = files(
    'advanced_regressions.cpp',
    'allocator_modes.cpp',
    'allocators.cpp',
    'basic.cpp',
    'constexpr.cpp',
    'construction.cpp',
    'copy_exceptions.cpp',
    'erasure.cpp',
    'exceptions.cpp',
    'insertion.cpp',
    'iterators.cpp',
    'lifecycle.cpp',
    'limits.cpp',
    'model.cpp',
    'moved_source.cpp',
    'pmr.cpp',
    'traits.cpp'
)

test_executable = executable(
    'heim-test-common-sparse-set',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'common-sparse-set',
    test_executable,
    suite: ['unit', 'heim-common'])
