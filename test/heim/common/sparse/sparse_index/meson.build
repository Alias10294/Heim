test_sources = files(
    'advanced_regressions.cpp',
    'allocators.cpp',
    'basic.cpp',
    'contiguous.cpp',
    'copy_move.cpp',
    'exceptions.cpp',
    'limits.cpp',
    'pages.cpp',
    'ranges.cpp',
    'stress.cpp',
    'traits.cpp'
)

test_executable = executable(
    'heim-test-common-sparse-index',
    test_sources,
    dependencies: [heim_dep, catch2_main_dep])

test(
    'common-sparse-index',
    test_executable,
    suite: ['unit', 'heim-common'])
