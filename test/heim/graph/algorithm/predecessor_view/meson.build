test_sources = files(
    'concepts.cpp',
    'customization.cpp',
    'direct_include.cpp',
    'iterators.cpp',
    'traversal.cpp',
)

test_executable = executable(
    'heim-test-graph-algorithm-predecessor-view',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'graph-algorithm-predecessor-view',
    test_executable,
    suite: ['unit', 'heim-graph'])
