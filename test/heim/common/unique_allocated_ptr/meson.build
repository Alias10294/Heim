test_sources = files(
    'alignment.cpp',
    'allocators.cpp',
    'arrays.cpp',
    'constexpr.cpp',
    'deleters.cpp',
    'exceptions.cpp',
    'fancy_pointer.cpp',
    'lifetime.cpp',
    'memory_resource.cpp',
    'single.cpp',
    'stress.cpp',
    'traits.cpp'
)

test_executable = executable(
    'heim-test-common-unique-allocated-ptr',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'common-unique-allocated-ptr',
    test_executable,
    suite: ['unit', 'heim-common'])
