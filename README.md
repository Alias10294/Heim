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
Heim is a header-only entity-component system library that focuses on intuitive usability, extensive customizability 
and outstanding performance. 

## Code Example 
```c++
#include <heim/heim.hpp>

struct position { float x, y, z; };
struct velocity { float x, y, z; };

using registry_t
= heim::registry<heim::sparse_set_based::storage<>
    ::component<position>
    ::component<velocity>
    ::group<position, velocity>>;
/* 
using registry_t 
= heim::registry<heim::archetype_based::storage<>
    ::component<position>
    ::component<velocity>
    ::cached_query<position, velocity>>;
*/

using query_t 
= registry_t::query
    ::include<position, velocity const>;
//  ::exclude<some_other_type, ...>;

int main()
{
  registry_t r;

  auto e0 = r.create();
  e0.emplace<position>(0.f, 0.f, 0.f);
  e0.emplace<velocity>(1.f, 0.f, 0.f);

  auto e1 = r.create();
  e1.emplace<position>(0.f, 1.f, 0.f);

  auto  q0 = r.query<query_t>();
  float ms = 16.f;

  for (auto [e, pos, vel] : q0)
  {
    pos.x += vel.x * ms;
    pos.y += vel.y * ms;
    pos.z += vel.z * ms;
  }
  
  r.destroy(e0);
  r.destroy(e1);
}
```
As Heim has yet to reach a functional implementation stage, this example is only suggestive, and only serves as an 
illustratory example.