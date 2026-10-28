test_sources = files(
    'direct_include.cpp',
    'flags.cpp',
    'types.cpp'
)

test_executable = executable(
    'heim-test-graph-interface-traits',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'graph-interface-traits',
    test_executable,
    suite: ['unit', 'heim-graph'])
