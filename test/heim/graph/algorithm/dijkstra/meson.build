test_sources = files(
    'audit_regressions.cpp',
    'basics.cpp',
    'custom_graph.cpp',
    'customization.cpp',
    'direct_include_search.cpp',
    'direct_include_search_to.cpp',
    'direct_include_visit.cpp',
    'exception.cpp',
    'model.cpp',
    'property_maps.cpp',
    'result/construction.cpp',
    'result/external_maps.cpp',
    'result/observers.cpp',
    'result/paths.cpp',
    'search_to.cpp',
    'traits.cpp',
    'visit.cpp',
    'visit_control.cpp',
    'visit_events.cpp'
)

test_executable = executable(
    'heim-test-graph-algorithm-dijkstra',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'graph-algorithm-dijkstra',
    test_executable,
    suite: ['unit', 'heim-graph'])
