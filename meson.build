project('heim', 'cpp', default_options: ['cpp_std=c++26'])

stdcxxexp_dep = meson.get_compiler('cpp').find_library('stdc++exp', required: true)
compiler_dep  = declare_dependency(
    compile_args: ['-fcontracts'],
    dependencies: [stdcxxexp_dep])

add_project_dependencies(compiler_dep, language: 'cpp')


heim_inc = include_directories('include')

heim_test_files = files(
    'test/test.cpp')
heim_test_exe   = executable(
    'heim_test',
    heim_test_files,
    include_directories: heim_inc)