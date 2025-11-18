# Heim: Organize Your Data The Right Way

## Requirements
To use Heim, users must acquire a compiler that supports at least C++23 as well as the build system Meson.

## Installation
### Use as a cloned repository
To install Heim, to import Heim as a cloned repository, follow these commands :
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

But what is an entity-component system ?

### Definition
The entity-component system (ECS) describes a way of organizing data such that it promotes reusability and efficiency 
when creating some kind of objects. It involves two concepts : the entities and the components. 

The components are a part of an object, they can contain behaviour and methods, but *traditionally* they are just plain 
data and behaviour is handled separately.

The entities are the identifier to each object. Components that are identified by the same entity are parts of the same 
object. 

Now building objects following these concepts is simple: create your entities, and associate each component you want to 
it. This way, you can create objects with any kind of combination of components, without even using once an inheritance 
hierarchy. Also, objects can receive and detach any component at any time, which allows objects to be even more dynamic 
in their composition.

### Heim's design
In Heim, both entities and components are managed through one central object: the registry, which both stores and 
manages entities as well as their associated components.

Unlike most ECSs out there, Heim's registry can only accepts new component types at compile-time. 

I for a long time questioned myself a lot about this decision, as while it comes with nice perks, not having the 
flexibility of managing new component types at runtime is a big disadvantage, especially in game engines and editors 
where it is all about the user creating its own components and building their game out of it. 

In the end I realised that they are already plenty of great ECSs that already do a great job of providing both the 
flexibility and the performance required to create even the most demanding of games and engines, and yet another ECS 
with the same characteristics and goals seems redundant. That is why I focused on making the most out of the benefits 
that my direction with component management could offer me. 

A lot of what this project came out to be is inspired by all the existing ECS projects, especially 
[EnTT](https://github.com/skypjack/entt) which is certainly one of, if not the most used and praised entity-component 
system out there. A lot of the design decisions in Heim had the goal to perfect even more, not so much in terms of 
performance but more in terms of the practices and elegance portrayed in the code itself, the decisions made in EnTT.

### A learning experience 
I started using C++ in fall 2024, with the idea in mind to make a complete video game by myself as a passion project. 
At first, it was difficult. I had no idea how a game was supposed to be structured and organized, so I made a lot of 
research, but more so a lot of design errors. Especially, I quickly realised that working with deep inheritance 
hierarchies was not going to work for me, and dependency injection while helpful was going to end up causing just as 
many problems.

That is when I discovered the entity-component system, and immediately I understood that it was what I wanted to adopt.
I could have used already existing libraries to use it, but by then I had already fallen in love with the language and 
wanted to just code myself as much of the game as possible. The next months up until now I spent learning everything 
about the language and about entity-component systems, and the result comes of all of this learning time is this first 
project, Heim.

## Code Example
As Heim has not reached a functional stage yet, the code example below is only a suggestion of what using Heim can look 
like.

```cpp
#include "heim/registry.hpp"

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
  int points, regeneration;
};

// Define the type you prefer to use 
using entity = unsigned int;

using registry = heim::registry<entity>
    // declare components
    ::component<position>
    // declare components with specified parameters such as allocators
    ::component<velocity, std::allocator<velocity>>
    // sync components together to optimize common iteration
    ::sync<position, velocity>
    // and all of this in any order
    ::component<health>;
  
int main()
{
  registry r;
  
  // Preallocate as much memory as needed for both your entities and components.
  r.view<entity  >().reserve(2);
  r.view<position>().reserve(2);
  r.view<velocity>().reserve(2);
  r.view<health  >().reserve(1);
  
  // Declare a new entity
  entity e0 = r.entity();
  // Associate some components with it
  r.emplace<position>(e0, 0., 0., 0.);
  r.emplace<velocity>(e0, 0., 0., 0.);
  r.emplace<health  >(e0, 100, 10);
  
  entity e1 = r.entity();
  r.emplace<position>(e1, 1., 0., 0.);
  r.emplace<velocity>(e1, 0., 0., 0.);
  
  float dt = 16.;
  
  // Iterate through entities and components alike 
  for (auto [e, pos, vel] : r.view<position, velocity const>())
  {
    pos.x += vel.x * dt;
    pos.y += vel.y * dt;
    pos.z += vel.z * dt;
  }
  
  for (auto [e, h] : r.view<health>())
  {
    h.points += h.regeneration;
  }
}
```
A lot of other functionalities are coming and more customization is possible, but as a suggested demonstration and not 
yet functional, this serves its purpose well.