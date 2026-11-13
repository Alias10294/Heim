test_sources = files(
    'advanced_regressions.cpp',
    'allocators.cpp',
    'basic.cpp',
    'constexpr.cpp',
    'construction.cpp',
    'erasure.cpp',
    'exceptions.cpp',
    'forward_ranges.cpp',
    'insertion.cpp',
    'iterator_concepts.cpp',
    'iterators.cpp',
    'lifecycle.cpp',
    'limits.cpp',
    'mapped_values.cpp',
    'model.cpp',
    'moved_source.cpp',
    'pmr.cpp',
    'traits.cpp',
    'value_exceptions.cpp'
)

test_executable = executable(
    'heim-test-common-sparse-map',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'common-sparse-map',
    test_executable,
    suite: ['unit', 'heim-common'],
    timeout: 60)
