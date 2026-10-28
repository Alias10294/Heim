test_sources = files(
    'basics.cpp',
    'custom_graph.cpp',
    'direct_include.cpp',
    'model.cpp',
    'property_maps.cpp',
    'results.cpp',
    'traits.cpp'
)

test_executable = executable(
    'heim-test-graph-algorithm-component',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'graph-algorithm-component',
    test_executable,
    suite: ['unit', 'heim-graph'])
