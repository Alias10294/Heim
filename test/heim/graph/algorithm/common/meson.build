test_sources = files(
    'color.cpp',
    'direct_include.cpp',
    'visit.cpp',
)

test_executable = executable(
    'heim-test-graph-algorithm-common',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'graph-algorithm-common',
    test_executable,
    suite: ['unit', 'heim-graph'])
