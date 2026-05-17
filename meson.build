project('heim', 'cpp',
        version : '1.0.0',
        default_options : ['warning_level=3', 'cpp_std=c++23'])

heim_inc = include_directories('include')

heim_dep = declare_dependency(include_directories: heim_inc)
meson.override_dependency('heim', heim_dep)

heim_example_src = files('example/readme.cpp')
heim_example_exe = executable('heim_example', heim_example_src, include_directories: heim_inc)

heim_test_src = files('test/main.cpp')
heim_test_exe = executable('heim_test', heim_test_src, include_directories: heim_inc)