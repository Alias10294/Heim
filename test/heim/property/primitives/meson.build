test_sources = files('direct_include.cpp', 'get.cpp', 'set.cpp')

test_executable = executable(
    'heim-test-property-primitives',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'property-primitives',
    test_executable,
    suite: ['unit', 'heim-property'])
