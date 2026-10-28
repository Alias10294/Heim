test_sources = files(
    'allocators.cpp',
    'assignment.cpp',
    'basic.cpp',
    'configurations.cpp',
    'constexpr.cpp',
    'construction.cpp',
    'descriptors.cpp',
    'direct_include.cpp',
    'edges.cpp',
    'equality.cpp',
    'erasure.cpp',
    'exceptions.cpp',
    'exhaustive.cpp',
    'incidence.cpp',
    'integration.cpp',
    'limits.cpp',
    'model.cpp',
    'regressions.cpp',
    'swap.cpp',
    'traits.cpp',
    'vertices.cpp',
)

test_executable = executable(
    'heim-test-graph-adjacency-list',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'graph-adjacency-list',
    test_executable,
    suite: ['unit', 'heim-graph'])
