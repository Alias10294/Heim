# Heim: Organize Your Data The Right Way
<b>Heim</b> is a zero-dependency, header-only C++23 library for data-oriented game and simulation programming. Here are 
its main features:
- A hypermodern C++23 API
- A generic take on the entity-component-system (ECS) pattern
- Powerful template metaprogramming utilities such as the `type_sequence`
- [WIP] A modern graph library inspired by the famous <b>Boost Graph Library</b> (BGL)

## Code Example
```c++
#include <heim/heim.hpp>

struct position { float x, y, z; };
struct velocity { float x, y, z; };
struct tag      { };


using registry
= heim::ecs::sparse::auto_registry;

int main()
{
  registry   reg;
  auto const e0  = reg.make();
  auto const e1  = reg.make();

  reg.emplace<position>(e0, 0.f, 0.f, 0.f);
  reg.emplace<velocity>(e0, 1.f, 0.f, 0.f);

  reg.emplace<position>(e1, 0.f, 1.f, 0.f);
  reg.emplace<tag>     (e1);

  auto view = reg
            | heim::ecs::views::match<heim::ecs::conjunction<position, velocity>>;

  for (auto const e : view)
  {
    auto       &[px, py, pz] = reg.get<position>(id);
    auto const &[vx, vy, vz] = reg.get<velocity>(id);

    px += vx;
    py += vy;
    pz += vz;
  }

  reg.destroy(e0);
  reg.destroy(e1);
}
```

# Table of contents
- [Introduction](#introduction)
- [Entity-Component-System pattern](#entity-component-system-pattern)
- [Metaprogramming utilities](#metaprogramming-utilities)
- [Graphs](#graphs)
- [Installation](#installation)
- [License](#license)

# Introduction
...

# Entity-Component-System pattern
...

# Metaprogramming utilities
...

# Graphs
...

# Installation
...

# License
...