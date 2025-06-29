project('heim', 'cpp',
        version: '1.0.0',
        default_options: ['warning_level=3', 'cpp_std=c++20']
)

heim_inc = include_directories('include')
heim_dep = declare_dependency(include_directories: heim_inc)
meson.override_dependency('heim', heim_dep)


install_headers(
    'include/composer.hpp',
    'include/composition.hpp',
    'include/entity.hpp',
    'include/glimpse.hpp',
    'include/summoner.hpp',
    'include/world.hpp',
)


test_inc = include_directories('tests')
test_sources = files(
    'tests/test_heim.cpp',
    'tests/test_composition.cpp')

test_exe = executable(
    'heim_test',
    test_sources,
    include_directories: [heim_inc, test_inc]
)

test('heim_test', test_exe)
