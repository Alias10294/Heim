# Heim: Organize Your Data The Right Way

## Requirements
To use Heim, users must acquire a compiler that supports at least C++23 as well as the Meson build system.

## Installation
### Use as a cloned repository
To install Heim as a cloned repository, follow these commands :
```
> cd [directory of your choice]
> git clone https://github.com/Alias10294/Heim.git
> meson setup build 
```

Once the project is set up, the tests' executable can be compiled using this command:
```
> meson compile -C build
```
To use the executable, execute this command:
```
> ./build/heim_test
```

## Introduction
Heim is a header-only entity-component-system library that lets you organize your games and simulations in a both 
user-friendly and data-oriented manner. It focuses in providing an elegant API, allowing for extensive customizability 
and on delivering highly-performant code.

## Code Example 
```c++
#include <heim/heim.hpp>


struct position { int x, y; };
struct velocity { int x, y; };
struct tag      { };

using registry
= heim::ecs::sparse::auto_registry;

using expression
= heim::ecs::conjunction<position, velocity, heim::ecs::negation<tag>>;


int main()
{
  registry   reg{};
  auto const id0{reg.make()};
  auto const id1{reg.make()};

  reg.emplace<position>(id0, 0, 0);
  reg.emplace<velocity>(id0, 1, 0);

  reg.emplace<position>(id1, 0, 1);
  reg.emplace<tag     >(id1);

  for (auto const id : reg | heim::ecs::views::match<expression>)
  {
    auto       &[px, py]{reg.get<position>(id)};
    auto const &[vx, vy]{reg.get<velocity>(id)};

    px += vx;
    py += vy;
  }

  reg.destroy(id0);
  reg.destroy(id1);
}
```