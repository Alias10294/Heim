test_sources = files(
    'access.cpp',
    'associative.cpp',
    'composition.cpp',
    'construction.cpp',
    'direct_include.cpp',
    'factories.cpp',
    'integration.cpp',
    'invocable.cpp',
    'member_pointer.cpp',
    'random_access.cpp',
    'simple_adapters.cpp'
)

test_executable = executable(
    'heim-test-property-property-map',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'property-property-map',
    test_executable,
    suite: ['unit', 'heim-property'])
