test_sources = files(
    'audit_regressions.cpp',
    'basics.cpp',
    'containers.cpp',
    'custom_graph.cpp',
    'cycles.cpp',
    'direct_include.cpp',
    'iterators.cpp',
    'model.cpp',
    'property_maps.cpp',
    'traits.cpp'
)

test_executable = executable(
    'heim-test-graph-algorithm-topological-sort',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'graph-algorithm-topological-sort',
    test_executable,
    suite: ['unit', 'heim-graph'])
