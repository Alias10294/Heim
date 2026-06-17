# Heim: Organize Your Data The Right Way
`Heim` is a zero-dependency, header-only library dedicated to providing generic and highly performant solutions for 
simulation and game programming.<br>
Here are some of the library's features:
- Multiple compatible generic implementations of the Entity-Component-System (ECS) pattern
- Powerful template metaprogramming objects such as the `type_sequence`
- [WIP] A modern graph library inspired by the famous [Boost Graph Library](https://www.boost.org/doc/libs/latest/libs/graph/doc/index.html) (BGL)

## Table of contents
- [Introduction](#introduction)
  - [Code Example](#code-example)

## Introduction
### Code example
```c++
#include <heim/heim.hpp>

struct position { float x, y, z; };
struct velocity { float x, y, z; };
struct tag      { };


using registry
=        heim::ecs::sparse   ::auto_registry;
// [WIP] heim::ecs::archetype::auto_registry;
// [WIP] heim::ecs::hibit    ::auto_registry;
//       heim::ecs::sparse   ::static_registry::with<position>::with<velocity>::with<tag>::type;
// [WIP] heim::ecs::archetype::static_registry::with<position>::with<velocity>::with<tag>::type;
// [WIP] heim::ecs::hibit    ::static_registry::with<position>::with<velocity>::with<tag>::type;

int main()
{
  registry   reg;
  auto const id0 = reg.make();
  auto const id1 = reg.make();
    
  reg.emplace<position>(id0, 0.f, 0.f, 0.f);
  reg.emplace<velocity>(id0, 1.f, 0.f, 0.f);
    
  reg.emplace<position>(id1, 0.f, 1.f, 0.f);
  reg.emplace<tag     >(id1);
    
  auto v = reg
         | heim::ecs::views::match<heim::ecs::conjunction<position, velocity>>
  //     | std::views:: ...
  
  for (auto const id : v)
  {
    // ...
  }
    
  reg.destroy(id0);
  reg.destroy(id1);
}
```
