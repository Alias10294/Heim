# Heim
Heim is a C++26 collection of header-only libraries that provide useful and generic abstractions for 
*data-oriented design* (*DOD*).

## Libraries
### Properties
Represent associations between objects and their properties through a common read/write interface, independently of how
those associations are implemented. Property maps can adapt existing representations, including containers, members and
function objects, and can be composed to express richer mappings.
```cpp
#include <print>
#include <unordered_map>
#include <vector>
#include <heim/property.hpp>

struct entity { int hp; }

int main()
{
  std::unordered_map<int, int> map{{0, 100}, {1, 80}, {2, 60}};
  std::vector<entity>          vec{{100}, {80}, {60}};
  
  auto map_pm{heim::make_property_map(map)};
  auto vec_pm{heim::make_property_map(
      heim::composable_arg,
      heim::make_property_map(heim::member_arg, &entity::hp),
      heim::make_property_map(vec))};
  
  std::print("{} ", heim::properties::get(map_pm, 1));
  std::print("{} ", heim::properties::get(vec_pm, 1));

  heim::properties::set(map_pm, 1, 20);
  heim::properties::set(vec_pm, 1, 20);

  std::print("{} ", heim::properties::get(map_pm, 1));
  std::print("{} ", heim::properties::get(vec_pm, 1));
}
```
```text
80 80 20 20 
```

### Graphs
Represent graph structures and run algorithms through generic, requirement-based interfaces. Heim provides ready-to-use
graph types while allowing compatible user-defined representations to participate in the same algorithms.
```cpp
#include <print>
#include <heim/graph.hpp>

using graph_type  = heim::adjacency_list<>;
using vertex_type = heim::graphs::vertex_t<graph_type>;

int main()
{
  graph_type g{};
  
  vertex_type v0{heim::graphs::place_vertex(g)};
  vertex_type v1{heim::graphs::place_vertex(g)};
  vertex_type v2{heim::graphs::place_vertex(g)};
  vertex_type v3{heim::graphs::place_vertex(g)};
  
  auto [e0, placed]{heim::graphs::place_edge(g, v0, v1)};
  auto [e1, _     ]{heim::graphs::place_edge(g, v1, v2)};
  auto [e2, _     ]{heim::graphs::place_edge(g, v0, v2)};
  auto [e3, _     ]{heim::graphs::place_edge(g, v2, v3)};
  
  auto res{heim::graphs::breadth_first_search(g, v0)};
  if (res.reached(v3))
    std::print("{}", res.path_from(v3));
}
```
```text
[3, 2, 0]
```

## Why Use Heim ?
### Keep control over your data
Every concept formalized in Heim's libraries explicitly avoids imposing a particular representation for your data. How
it is structured, stored and organized remains your complete choice.

### Leverage genericity
Interact with common interfaces rather than concrete types. By expressing abstractions only in terms of their
requirements, Heim lets any suited representation benefit from the same code.

### Stay free to specialize
Genericity in Heim does not come at the cost of specialization, it *includes* it. Common interfaces remain open to
specialized behavior whenever a particular representation has more to offer.

### Compose richer abstractions
Heim's abstractions are intended to be tools at the service of your project, and can be freely combined to build more
expressive ones around your own needs.

### Maintain uncompromised performance
Heim fully embraces C++'s core motivation of *zero-cost abstraction*. In the same manner as the standard library,
abstractions do not take away from performance and can even fully disappear through compilation.

### Harness C++ modernity
Sitting on top of the best modern C++ has to offer, Heim makes the most of recent language and standard library
features such as *concepts*, *ranges* and *customization point objects*.

## Requirements
To be used, Heim requires a C++26 compiler with Contracts support.

Currently, only GCC 16 and later meets these requirements. Check out
[this page](https://en.cppreference.com/cpp/compiler_support/26) to stay informed about compiler support for C++26.

## Installation
### Single-header distribution
The most straightforward way to have access to all of Heim's libraries is to copy the `heim.hpp` from the
`single_include/` directory. This directory also provides individual files for each individual library, to be copied in
the same way.

### Using Meson
#### Make Heim available
##### As a subproject
The most natural way to use Heim with Meson is to have it as a Meson subproject. That is attained by placing the Heim
repository in your project's `subprojects/` directory, either using Git
```text
git clone https://github.com/Alias10294/Heim.git subprojects/heim
```
or as a Git submodule.
```text
git submodule add https://github.com/Alias10294/Heim.git subprojects/heim
```

Your project should look like this:
```text
your-project/
├── ...
├── subprojects/
│   └── heim/
│       ├── include/
│       ├── ...
│       └── meson.build
└── meson.build
```

##### As a system dependency
Heim can also be installed independently anywhere in your machine and later found through `pkg-config`.
```text 
git clone https://github.com/Alias10294/Heim.git
cd heim
meson setup build
meson install -C build
```

#### Add it as dependency
Once Heim has been made available, the only action left is to declare it in your `meson.build` file.
```meson
heim = dependency('heim')
```
The dependency can the passed to any candidate target that needs Heim or one of its libraries.
```meson
executable(
    ...,
    dependencies: heim)
```

## Documentation
Documentation for the entire project can be found [here](https://alias10294.github.io/Heim/).

## License
Heim is available under the [MIT license](LICENSE).