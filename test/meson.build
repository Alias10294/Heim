# Catch2 is needed only when the root project enables Heim tests.
# Keep the test dependency out of heim_dep and out of installed headers.
catch2_project = subproject('catch2', default_options: [
  'tests=false',
  'install=false',
  'cpp_std=c++17',
])
catch2_main_dep = catch2_project.get_variable('catch2_with_main_dep')

# Tests against the regular public headers (include/heim), not amalgamations.
subdir('heim')

# Independent distribution/release checks will be added progressively.
subdir('distribution')
