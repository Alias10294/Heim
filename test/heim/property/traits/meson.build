test_sources = files('direct_include.cpp', 'readable.cpp', 'writable.cpp', 'map.cpp', 'reflective.cpp')

test_executable = executable(
    'heim-test-property-traits',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'property-traits',
    test_executable,
    suite: ['unit', 'heim-property'])
