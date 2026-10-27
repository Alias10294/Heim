project(
    'heim',
    'cpp',
    meson_version: '>=1.1',
    default_options: ['cpp_std=c++26'])


cpp_compiler      = meson.get_compiler('cpp')
cpp_compiler_args = []
cpp_dependencies  = []

if cpp_compiler.get_id() == 'gcc'
  if not cpp_compiler.version().version_compare('>=16')
    error('GCC 16 or later is required for Heim to compile.')
  endif

  cpp_compiler_args += ['-fcontracts']
  cpp_dependencies  += cpp_compiler.find_library('stdc++exp', required: true)
else
  error('A C++26 compiler with Contracts support is required for Heim to compile.')
endif


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
    description:  'C++26 collection of header-only generic libraries',
    version:      meson.project_version(),
    subdirs:      '.',
    libraries:    cpp_dependencies,
    extra_cflags: cpp_compiler_args)


if get_option('heim_tests') and not meson.is_subproject()
  subdir('test')
endif
