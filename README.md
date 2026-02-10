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

Once the project is set up, the tests' executable can be compiled and executed using these commands :
```
> meson compile -C build
> ./build/heim_test
```

## Introduction
Heim is a header-only entity-component system library that focuses on usability, customizability and performance, while 
providing an elegant API.

## Code Example 
```c++
#include <heim/heim.hpp>

struct position { float x, y, z; };
struct velocity { float x, y, z; };
struct health   { int hp; };


using registry
= heim::registry<heim::sparse_set_based::storage<>
    ::component<position>
    ::component<velocity>
    ::component<health  >>;

using query_expression
= heim::query_expression<>
    ::include<position, velocity const>
    ::exclude<health>;

int main()
{
  registry r;

  auto const e0 = r.create();
  r.emplace<position>(e0, 0.f, 0.f, 0.f);
  r.emplace<velocity>(e0, 1.f, 0.f, 0.f);

  auto const e1 = r.create();
  r.emplace<position>(e1, 0.f, 1.f, 0.f);
  r.emplace<health  >(e1, 10);

  auto q = r.query<query_expression>();

  for (auto &&[e, pos, vel] : q)
  {
    pos.x += vel.x;
    pos.y += vel.y;
    pos.z += vel.z;
  }

  r.destroy(e0);
  r.destroy(e1);
}
```