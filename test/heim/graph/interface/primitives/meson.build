test_sources = files(
    'counts.cpp',
    'direct_include.cpp',
    'dispatch.cpp',
    'endpoints.cpp',
    'incidence.cpp',
    'mutation.cpp',
    'ranges.cpp',
    'specializations.cpp'
)

test_executable = executable(
    'heim-test-graph-interface-primitives',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'graph-interface-primitives',
    test_executable,
    suite: ['unit', 'heim-graph'])
