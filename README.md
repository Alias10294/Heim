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
```cpp
#include "heim/heim.hpp"

struct position 
{
  float x, y, z;
};

struct velocity 
{
  float x, y, z;
};

struct health 
{
  int hp, regen;
};

using registry 
= heim::registry<>
    ::component<position>
    ::component<velocity>
    ::component<health  >
    ::sync<position, velocity>;

int main()
{
  registry r;
  
  r.reserve<position>(2);
  r.reserve<velocity>(2);
  r.reserve<health  >(1);
  
  auto const e0 = r.entity();
  auto const e1 = r.entity();
  
  r.add<position>(e0, .0f, .0f, .0f);
  r.add<velocity>(e0, .0f, .0f, .0f);
  r.add<health  >(e0, 100, 10);
  
  r.add<position>(e1, 1.0f, .0f, .0f);
  r.add<velocity>(e1, 1.0f, .0f, .0f);
  
  auto  view = r.view<position, velocity const>();
  float dt   = 16.0f;
  for (auto const e : view)
  {
    position       &pos = view.get<position>(e);
    velocity const &vel = view.get<velocity>(e);

    pos.x += vel.x * dt;
    pos.y += vel.y * dt;
    pos.z += vel.z * dt;
  }
  
  return 0;
}
```
As Heim has yet to reach a functional implementation stage, this example is only suggestive, and only serves as an 
illustratory example.