Heim — adjacency_list: final focused test additions

Base commit reviewed: 53ee7ca63c2bada91d973305c465beaa1ff2c4fa

Install: extract this archive at the root of the Heim repository, preserving paths.
The zip only adds copy_selection_support.hpp, allocator_selection.cpp and
construction_reference.cpp, plus the updated adjacency_list/meson.build.
No production header is changed. If Meson has changed since the reference
commit, use the included Meson patch rather than overwriting the build file.

Tests cover nontrivial select_on_container_copy_construction (SOCCC),
allocator identity and nested incident-storage rebinding, and constructors
compared with an independent edge-sequence oracle.

The tests intentionally do NOT assume an undocumented iterator/view
invalidation rule or define behavior at unsigned identifier overflow.

Run:
  meson setup build -Dheim_tests=true --reconfigure
  meson compile -C build
  meson test -C build --suite heim-graph --print-errorlogs
