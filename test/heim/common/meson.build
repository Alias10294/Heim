# One Catch2 executable per cohesive feature. Additional files can be added to
# a feature's source list without changing how its test target is registered.
common_test_features = {
  'sparse-index':         ['sparse_index/storage.cpp'],
  'sparse-set':           ['sparse_set/operations.cpp'],
  'sparse-map':           ['sparse_map/operations.cpp'],
  'unique-allocated-ptr': [
    'unique_allocated_ptr/lifetime.cpp',
    'unique_allocated_ptr/traits.cpp',
    'unique_allocated_ptr/single.cpp',
    'unique_allocated_ptr/arrays.cpp',
    'unique_allocated_ptr/allocators.cpp',
    'unique_allocated_ptr/exceptions.cpp',
    'unique_allocated_ptr/deleters.cpp',
    'unique_allocated_ptr/fancy_pointer.cpp',
    'unique_allocated_ptr/memory_resource.cpp',
    'unique_allocated_ptr/stress.cpp',
    'unique_allocated_ptr/constexpr.cpp',
    'unique_allocated_ptr/alignment.cpp',
  ],
}

foreach feature, sources : common_test_features
  exe = executable('heim-test-common-' + feature, sources,
    dependencies: [heim_dep, catch2_main_dep])
  test('unit-common-' + feature, exe, suite: ['unit', 'heim-common'])
endforeach
