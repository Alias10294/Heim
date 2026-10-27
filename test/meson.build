# Keep all test dependencies out of the installed Heim interface.
catch2_project = subproject('catch2', default_options: [
  'tests=false', 'install=false', 'cpp_std=c++17',
])
catch2_main_dep = catch2_project.get_variable('catch2_with_main_dep')

test_modules = {
  'property': ['property/property_map.cpp'],
  'graph':    ['graph/graph.cpp'],
  'meta':     ['meta/type_sequence.cpp'],

  'common-sparse-index': ['common/sparse_index.cpp'],
  'common-sparse-set':   ['common/sparse_set.cpp'],
  'common-sparse-map':   ['common/sparse_map.cpp'],
  'common-allocator':    ['common/allocated_ptr.cpp'],
  'property-extended':   ['property/cpo.cpp', 'property/mappings.cpp'],
  'meta-extended':       ['meta/type_sequence_extended.cpp', 'meta/type_sequence_combinatorics.cpp'],
  'graph-adjacency':     ['graph/adjacency_list.cpp', 'graph/adjacency_list_model.cpp', 'graph/variants.cpp'],
  'graph-bfs':           ['graph/breadth_first.cpp', 'graph/breadth_first_model.cpp'],
  'graph-dfs':           ['graph/depth_first.cpp'],
  'graph-dijkstra':      ['graph/dijkstra.cpp'],
  'graph-topological':   ['graph/topological_sort.cpp'],
  'graph-components':    ['graph/components.cpp'],
  'graph-heap':          ['graph/indirect_heap.cpp'],
  'graph-visitors':      ['graph/visitors.cpp'],
}
foreach name, sources : test_modules
  exe = executable('heim-test-' + name, sources,
    dependencies: [heim_dep, catch2_main_dep])
  test('unit-' + name, exe, suite: 'unit')
endforeach

# Build a separate translation unit for each public amalgamated header.
# Crucially, heim_dep is NOT used: -Iinclude would mask a broken amalgamation.
amalgamated_inc = include_directories('../single_include')
foreach name : ['heim', 'graph', 'property', 'meta', 'ecs']
  exe = executable('heim-amalgamated-' + name,
    'single_include/' + name + '.cpp',
    include_directories: [amalgamated_inc, include_directories('../include')],
    cpp_args: cpp_compiler_args,
    dependencies: cpp_dependencies)
  test('amalgamated-' + name, exe, suite: 'distribution')
endforeach

# A single-header build must link cleanly from independent TUs (ODR).
odr_exe = executable('heim-amalgamated-odr',
  ['single_include/odr_a.cpp', 'single_include/odr_b.cpp', 'single_include/odr_main.cpp'],
  include_directories: amalgamated_inc,
  cpp_args: cpp_compiler_args,
  dependencies: cpp_dependencies)
test('amalgamated-odr', odr_exe, suite: 'distribution')

py = import('python').find_installation('python3')
generator_script = meson.project_source_root() / 'scripts' / 'generate_single_include.py'
test('amalgamation-freshness', py,
  args: [generator_script, '--check'],
  suite: 'distribution')
test('amalgamation-regressions', py,
  args: ['-m', 'unittest', 'discover', '-s',
         meson.project_source_root() / 'scripts' / 'tests', '-p', 'test_*.py'],
  suite: 'distribution')
