project(
    'heim',
    'cpp',
    version:         '0.1.0',
    license:         'MIT',
    meson_version:   '>=1.1',
    default_options: ['cpp_std=c++23'])


cpp_compiler      = meson.get_compiler('cpp')
cpp_compiler_args = []
cpp_dependencies  = []


heim_inc = include_directories('include')

heim_dep = declare_dependency(
    include_directories: heim_inc,
    compile_args:        cpp_compiler_args,
    dependencies:        cpp_dependencies)
meson.override_dependency('heim', heim_dep)

install_subdir(
    'include/heim',
    install_dir: get_option('includedir'))


pkgconfig = import('pkgconfig')
pkgconfig.generate(
    name:         'heim',
    filebase:     'heim',
    description:  'C++23 collection of header-only generic libraries',
    version:      meson.project_version(),
    subdirs:      '.',
    libraries:    cpp_dependencies,
    extra_cflags: cpp_compiler_args)


if get_option('heim_tests') and not meson.is_subproject()
  subdir('test')
endif
