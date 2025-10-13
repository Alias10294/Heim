project('heim', 'cpp',
        version : '1.0.0',
        default_options : ['warning_level=3', 'cpp_std=c++23'])

heim_inc = include_directories('include')

heim_dep = declare_dependency(
    include_directories: heim_inc)
meson.override_dependency('heim', heim_dep)

heim_test_src = files(
    'test/test_config.cpp',
    'test/test_index_map.cpp')

heim_test_exe = executable('heim_test',
    heim_test_src,
    include_directories: heim_inc)

test('heim_test', heim_test_exe)