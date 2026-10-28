test_sources = files(
    'direct_include.cpp',
    'external.cpp',
    'owned.cpp',
)

test_executable = executable(
    'heim-test-graph-algorithm-result-storage',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'graph-algorithm-result-storage',
    test_executable,
    suite: ['unit', 'heim-graph'])
