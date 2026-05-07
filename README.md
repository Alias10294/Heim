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

Once the project is set up, the tests' and benchmark's executables can be compiled using this command:
```
> meson compile -C build
```
To use either executable, use these commands:
```
> ./build/heim_test
> ./build/heim_benchmark
```

## Introduction
Heim is a header-only entity-component-system library that lets you organize your games and simulations in a both 
user-friendly and data-oriented manner. It focuses in providing an elegant API, allowing for extensive customizability 
and on delivering highly-performant code.

## Code Example 
```c++
#include <heim/heim.hpp>

struct position { float x, y, z; };
struct velocity { float x, y, z; };
struct health   { int hp; };


using registry
= heim::sparse::registry<>
    ::with<position>
    ::with<velocity>
    ::with<health  >;

using query_expression
= heim::conjunction<position, velocity, negation<health>>;

using entity 
= heim::entity<registry>;

int main()
{
  registry r;

  auto const id0 = r.create();
  r.emplace<position>(id, 0.f, 0.f, 0.f);
  r.emplace<velocity>(id, 1.f, 0.f, 0.f);

  entity e0(r);
  e0.emplace<position>(0.f, 1.f, 0.f);
  e0.emplace<health  >(10);

  auto q = r.query<query_expression>();

  for (auto e : q)
  {
    auto       &pos = e.get<position>();
    auto const &vel = e.get<velocity>();  
      
    pos.x += vel.x;
    pos.y += vel.y;
    pos.z += vel.z;
  }

  r.destroy(id0);
  e0.destroy();
}
```