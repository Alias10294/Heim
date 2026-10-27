# One Catch2 executable per cohesive feature. Additional files can be added to
# a feature's source list without changing how its test target is registered.
common_test_features = {
  'sparse-index': [
    'sparse_index/traits.cpp',
    'sparse_index/basic.cpp',
    'sparse_index/pages.cpp',
    'sparse_index/contiguous.cpp',
    'sparse_index/ranges.cpp',
    'sparse_index/copy_move.cpp',
    'sparse_index/allocators.cpp',
    'sparse_index/exceptions.cpp',
    'sparse_index/limits.cpp',
    'sparse_index/stress.cpp',
  ],
  'sparse-set': [
    'sparse_set/traits.cpp',
    'sparse_set/basic.cpp',
    'sparse_set/construction.cpp',
    'sparse_set/insertion.cpp',
    'sparse_set/erasure.cpp',
    'sparse_set/iterators.cpp',
    'sparse_set/lifecycle.cpp',
    'sparse_set/allocators.cpp',
    'sparse_set/pmr.cpp',
    'sparse_set/exceptions.cpp',
    'sparse_set/limits.cpp',
    'sparse_set/model.cpp',
    'sparse_set/constexpr.cpp',
  ],
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
