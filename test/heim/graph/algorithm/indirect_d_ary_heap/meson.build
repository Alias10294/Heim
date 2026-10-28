test_sources = files(
    'basic.cpp',
    'customization.cpp',
    'decrease_key.cpp',
    'direct_include.cpp',
    'model.cpp',
    'ownership.cpp',
)

test_executable = executable(
    'heim-test-graph-algorithm-indirect-heap',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'graph-algorithm-indirect-heap',
    test_executable,
    suite: ['unit', 'heim-graph'])
