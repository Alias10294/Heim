test_sources = files(
    'aggregate.cpp'
)

test_executable = executable(
    'heim-test-meta-header',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'meta-header',
    test_executable,
    suite: ['unit', 'heim-meta'])
