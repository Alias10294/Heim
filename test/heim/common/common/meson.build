test_sources = files(
    'aggregate.cpp'
)

test_executable = executable(
    'heim-test-common-header',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'common-header',
    test_executable,
    suite: ['unit', 'heim-common'])
