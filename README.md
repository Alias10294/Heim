# Heim
Heim is a C++26 collection of header-only libraries that provide useful and generic abstractions for 
*data-oriented design* (*DOD*).

It currently features:
+ a [graph](docs/graph/index.md) library inspired by the 
  [Boost Graph Library (BGL)](https://www.boost.org/doc/libs/latest/libs/graph/doc/html/graph/index.html),
+ a [module](docs/property/index.md) library that abstracts the concepts of object properties and association of 
  objects.

## Code examples
### Graphs
```cpp
#include <print>
#include <heim/graph.hpp>

using graph_type = heim::adjacency_list<>;

int main()
{
  graph_type g;

  auto v0 = heim::graphs::place_vertex();
  auto v1 = heim::graphs::place_vertex();
  auto v2 = heim::graphs::place_vertex();
  auto v3 = heim::graphs::place_vertex();

  auto [e0, placed] = heim::graphs::place_edge(g, v0, v1);
  auto [e1, _     ] = heim::graphs::place_edge(g, v1, v2);
  auto [e2, _     ] = heim::graphs::place_edge(g, v0, v2);
  auto [e3, _     ] = heim::graphs::place_edge(g, v2, v3);

  auto res = heim::graphs::breadth_first_search(g, v0);
  if (res.reached(v3))
    std::println("{}", res.path_from(v3));  
}
```

### Properties
```cpp
#include <print>
#include <unordered_map>
#include <vector>
#include <heim/property.hpp>

struct entity { int hp; };

int main()
{
  std::unordered_map<int, int> map{{0, 100}, {1, 80}, {2, 60}};
  std::vector<entity>          vec{{100}, {80}, {60}};
  
  auto map_pm{heim::make_property_map(map)};
  auto vec_pm{heim::make_property_map(
      heim::composable_arg,
      heim::make_property_map(heim::member_arg, &entity::hp),
      heim::make_property_map(vec)};
      
  std::print("{} ", heim::properties::get(map_pm, 1));
  std::print("{} ", heim::properties::get(vec_pm, 1));
  
  heim::properties::set(map_pm, 1, 20);
  heim::properties::set(vec_pm, 1, 20);
  
  std::print("{} ", heim::properties::get(map_pm, 1));
  std::print("{} ", heim::properties::get(vec_pm, 1));
}
```

## Getting Started
### Requirements
Heim only requires a C++26 compiler to be used, no other dependency whatsoever.

### Installation
#### Single-header distribution
The most straightforward way to have access to all of Heim's libraries is to copy the `heim.hpp` from the
`single_include/` directory. This directory also provides individual files for each individual library, to be copied in
the same way.

#### Vendoring the repository
Heim can easily be included directly in a project, for example as a Git submodule
```console
git submodule add https://github.com/Alias10294/Heim external/heim
```
and then be added to the compiler's include paths, either manually or with any build system.

#### Using Meson
To use Heim with Meson, you can copy the repository in the `subprojects/` directory then obtain it as a dependency.
```meson
heim_dep = dependency(
    'heim',
    fallback: ['heim', 'heim_dep']
)
```

## Documentation
Each library has its own documentation.

See the [Graphs documentation](docs/graph/index.md).\
See the [Properties documentation](docs/property/index.md).

## License
Heim is available under the [MIT license](LICENSE).