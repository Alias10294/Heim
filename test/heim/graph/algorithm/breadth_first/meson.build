test_sources = files(
    'audit_regressions.cpp',
    'custom_graph.cpp',
    'direct_include_search.cpp',
    'direct_include_search_to.cpp',
    'direct_include_visit.cpp',
    'exception.cpp',
    'property_maps.cpp',
    'queue.cpp',
    'search.cpp',
    'search_model.cpp',
    'search_to.cpp',
    'traits.cpp',
    'visit.cpp',
    'visit_control.cpp',
    'visit_events.cpp',
    'result/construction.cpp',
    'result/external_maps.cpp',
    'result/observers.cpp',
    'result/paths.cpp',
)

test_executable = executable(
    'heim-test-graph-algorithm-breadth-first',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'graph-algorithm-breadth-first',
    test_executable,
    suite: ['unit', 'heim-graph'])
