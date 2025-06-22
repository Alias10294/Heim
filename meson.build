project('heim', 'cpp',
        version: '1.0.0',
        default_options: ['warning_level=3', 'cpp_std=c++20']
)

heim_inc = include_directories('include')
heim_dep = declare_dependency(include_directories: heim_inc)
meson.override_dependency('heim', heim_dep)


install_headers(
    'include/heim/composer.hpp',
    'include/heim/composition.hpp',
    'include/heim/entity.hpp',
    'include/heim/glimpse.hpp',
    'include/heim/summoner.hpp',
    'include/heim/world.hpp',
    subdir: 'heim'
)


test_inc = include_directories('tests')
test_sources = files(
    'tests/test_heim.cpp',
    'tests/test_summoner.cpp',
    'tests/test_composition.cpp')

test_exe = executable(
    'heim_test',
    test_sources,
    include_directories: [heim_inc, test_inc]
)

test('heim_test', test_exe)
