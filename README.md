# Heim: Organize Your Data The Right Way
`Heim` is a zero-dependency, header-only C++23 library for data-oriented game and simulation programming. Here are its 
main features:
- A hypermodern C++23 API;
- A generic take on the [entity-component-system (ECS)](https://en.wikipedia.org/wiki/Entity_component_system) pattern;
- Powerful template metaprogramming utilities such as the `type_sequence`;
- [WIP] A modern graph library inspired by the famous 
[Boost Graph Library (BGL)](https://www.boost.org/doc/libs/latest/libs/graph/doc/index.html).

## Code Examples
### Entity-component-system pattern
```c++
#include <heim/ecs.hpp>

struct position { float x, y, z; };
struct velocity { float x, y, z; };
struct tag      { };


using registry
= heim::ecs::sparse::auto_registry;

int main()
{
  registry   reg;
  auto const id0 = reg.make();
  auto const id1 = reg.make();

  reg.emplace<position>(id0, 0.f, 0.f, 0.f);
  reg.emplace<velocity>(id0, 1.f, 0.f, 0.f);

  reg.emplace<position>(id1, 0.f, 1.f, 0.f);
  reg.emplace<tag>     (id1);

  auto view = reg
            | heim::ecs::views::match<heim::ecs::conjunction<position, velocity>>;

  for (auto const id : view)
  {
    auto       &[px, py, pz] = reg.get<position>(id);
    auto const &[vx, vy, vz] = reg.get<velocity>(id);

    px += vx;
    py += vy;
    pz += vz;
  }

  reg.destroy(id0);
  reg.destroy(id1);
}
```

### Metaprogramming utilities
```c++
#include <cstdint>
#include <type_traits>
#include <heim/meta.hpp>

using some_type_sequence 
= heim::type_sequence<std::int8_t, std::int16_t, std::int32_t, std::int32_t, std::int64_t>;

using unsigned_type_sequence
= some_type_sequence
    ::transform<std::make_unsigned>
    ::prepend  <bool>
    ::unique;


using some_other_type_sequence
= heim::type_sequence<int, float, float, int, float, float>;

using int_sequence       = some_other_type_sequence::stride<3>;
using other_int_sequence = some_other_type_sequence::filter<std::is_integral>;
```

### Graphs
```c++
#include <print>
#include <unordered_map>
#include <heim/graph.hpp>


using graph_type = heim::graphs::adjacency_list<heim::graphs::undirected_tag>;
using edge_type  = heim::graphs::edge_t<graph_type>;

int main()
{
  graph_type                         g;
  std::unordered_map<edge_type, int> weights;
  
  auto const v0 = heim::graphs::place_vertex(g);
  auto const v1 = heim::graphs::place_vertex(g);
  auto const v2 = heim::graphs::place_vertex(g);
  auto const v3 = heim::graphs::place_vertex(g);
  
  auto weight_map = heim::graphs::make_property_map(weights);
  
  auto const e0 = heim::graphs::place_edge(g, v0, v1); weight_map.set(e0, 1);
  auto const e1 = heim::graphs::place_edge(g, v1, v2); weight_map.set(e1, 2);
  auto const e2 = heim::graphs::place_edge(g, v0, v2); weight_map.set(e2, 10);
  auto const e3 = heim::graphs::place_edge(g, v2, v3); weight_map.set(e3, 1);
  
  auto res = heim::graphs::dijkstra(g, v0, weight_map);
  
  for (auto v : heim::graphs::vertices(g))
    std::println("distance from {0} to {1}: {2}.", v0, v, res.distance_to(v));
}
```

# Table of contents
- [Entity-component-system pattern](#entity-component-system-pattern-1)
  - [Design](#design)
    - [Identifiers](#identifiers)
    - [Registries](#registries)
    - [Matching entities](#matching-entities)
  - [Implementation strategies](#implementation-strategies)
    - [Sparse-set-based strategy](#sparse-set-based-strategy)
    - [Archetype-based strategy](#archetype-based-strategy)
    - [Hibit-tree-based strategy](#hibit-tree-based-strategy)
- [Metaprogramming utilities](#metaprogramming-utilities-1)
- [Graphs](#graphs-1)
  - [Inspiration](#inspiration)
  - [Implementation](#implementation)
    - [Interface](#interface)
    - [Properties](#properties)
    - [Algorithms](#algorithms)
- [Installation](#installation)
- [License](#license)

# Entity-component-system pattern
`Heim`'s main feature is its implementation of the 
[entity-component-system (ECS)](https://en.wikipedia.org/wiki/Entity_component_system) pattern.

## Design
### Identifiers
Most of the time in ECS implementations, entities are not concrete singular objects in code. Rather, their components 
are often stored and organized in a data-oriented manner to optimize for performance.

To keep a given *logical* entity associated with its components, with `Heim` each entity is assigned a unique 
*identifier*.\
This identifier, which can be as simple as an unsigned integer value, serves as a key for all operations made on the 
entity, such as accessing or modifying one of its components, inserting or removing a component, and even destroying 
the entity itself.

### Registries
Genericity in a program or code library is often achieved through the separation of data and behavior. In the ECS 
pattern, systems by definition are responsible for the behavior of entities.\
That is why in `Heim`, entities and their components live separately from systems in a container called a *registry*.

As containers of *logical* entities, registries expose many operations:
- creation / destruction of entities;
- assignment / removal of components to / from entities;
- access to components;
- ...

### Matching entities
Because systems usually want to operate on groups of entities that match certain combinations of components, registries 
expose views (often called queries) that provide these entities as a range.\
To be able to describe complex combination of components, registries in `Heim` use *expressions*, a small compile-time 
language where component types are the "variables". Expressions can be formed using three "keyword" types, and can 
either be:
- A single component type;
- A conjunction of sub-expressions;
- A disjunction of sub-expressions;
- A negation of a sub-expression.

#### Single component type
A single component type used as an expression simply means that entities in the associated vie must possess the 
component type.

#### Conjunction
A conjunction of sub-expressions express the idea that the entities included in the associated view must match all
sub-expressions. It represents the logical operation `AND`, and looks like this in code:
```c++
heim::ecs::conjunction<Expressions ...>
```

#### Disjunction
A disjunction of sub-expression express the idea that the entities in the associated view must match at least one of 
the sub-expressions. It represents the logical operation `OR`, and looks like this in code:
```c++
heim::ecs::disjunction<Expressions ...>
```

#### Negation
A negation of a sub-expression express the idea that the entities in the associated view must not match any of the 
sub-expressions. It represents the logical operation `NOT`, and looks like this in code:
```c++
heim::ecs::negation<Expression>
```

## Implementation strategies
The ECS pattern, by essence, is not necessarily tied to certain implementation strategies (e.g. archetypes or sparse
sets). Each of those strategies have their own strengths and weaknesses, but shares the same core functionalities.\
That is why `Heim` aims to provide multiple interchangeable implementations of the pattern that exhibit the same 
minimal interface.

### Sparse-set-based strategy
This strategy is probably the simplest, but still competitive, approach when implementing the ECS pattern.\
In this approach, each component type in the registry is associated with its own contiguous associative container, 
which are inspired by sparse sets. 

#### Advantages
With a sparse-set-based registry, insertion and removal is the most trivial of all strategies, as you simply need to 
modify the component type's container.

#### Disadvantages
Although constructing the views of a sparse-set-based registry is fast, iterating it can be costly. Because each 
component container is independent, encountered entities are not guaranteed to match the expression, thus verification 
is needed for each one. This is very acceptable when expressions do not involve many components, but can be costly when 
expressions include many component types.\
This cost can be mitigated though by selecting the smallest range of entities able to match the expression, thus 
reducing how many entities are encountered.

### Archetype-based strategy
This implementation strategy rely on storing entities possessing the same combination of component types together 
contiguously in tables called *archetypes*.
> Please note while `Heim` is ready to welcome an archetype-based registry implementation, such implementation has not 
been implemented yet.

#### Advantages
Because entities are grouped by their combination of component types, to construct a view registries just need to find 
which archetypes match its expression, and iteration boils down to traversing the entities of each archetype which is 
very fast. This is especially fast when expression mention a lot of component types, as the number of archetypes that 
match said expression is often low.

#### Disadvantages
When a component is inserted into or removed from an entity, the entity has to move to their new archetype, and the 
departed-from archetype has to reorder its entities to assure contiguity, which can be expensive in certain situations.

### Other strategies
While theses two strategies are today the usual picks when implementing the ECS pattern, other strategies exist.\
For example, entities could be arranged along on a hierarchical inverted bitmap tree which trades some memory 
contiguity for other benefits. Bitsets more generally can be used in many inside an ECS implementation.

# Metaprogramming utilities
...

# Graphs
Whether it is in `Heim` itself or in the projects that use it, chances are that graphs or graph-like structures are 
going to appear. Examples of situations where graphs are useful or even *critical* vary: rendering, physics simulation, 
pathfinding, procedural generation, and *many* more.

Because having the right data structures and algorithms at hand in all those situations goes a long way towards both 
speeding up development and ensuring high performance on often important systems, `Heim` aims to provide *just that*.
> Please note that this section of the library is still underway, and that the information below is *not* guaranteed to 
be complete or final.

## Inspiration
Coming up with a representation of graphs that fits every possible use case is very difficult. One famous library that 
has done that is the [Boost Graph Library (BGL)](https://www.boost.org/doc/libs/latest/libs/graph/doc/index.html). It 
succeeded by:
- Decoupling graph data structures and algorithms to maximize their reusability;
- Defining the expected interface of graph data structures through the use of traits and external functions;
- Dissociating the graphs' descriptive data from their functional properties.

This ensured that every candidate data structure could implement the interface needed to interact with the library's 
algorithms.

## Implementation
While the BGL's ideas represent perhaps the best way to go about implementing a generic graph library, its interface 
does not fully match what modern C++ code tends to look like. It is also somewhat unfriendly to beginner-level 
programmers, which is far from what it could be.\
`Heim` aims to bring those ideas into a more modern C++, using new principles such as **concepts**, **ranges** and 
**customization point objects** to implement a newer and easier-to-use library.

### Interface
`Heim`'s graphs' interface, instead of revolving around a centralized `graph_traits` object that needs every method and 
alias individually specialized, will make use of 
[customization point objects](https://en.cppreference.com/cpp/named_req/CustomizationPointObject). This allows more 
flexibility and control over their specialization whilst also guaranteeing robust default implementations for some of 
them.\
Here is a first glance at what this interface might look like:
```c++
using graph_type  = /* ... */;
using vertex_type = heim::graphs::vertex_t<graph_type>;
using edge_type   = heim::graphs::edge_t  <graph_type>;

graph_type        g;
vertex_descriptor u, v;
edge_descriptor   e;

// graph-related access operations
heim::graphs::vertices(g);
heim::graphs::edges   (g);
heim::graphs::vertex_count(g); // can be deduced from vertices
heim::graphs::edge_count  (g); // can be deduced from edges

// vertex-related access operations
heim::graphs::out_edges (g, v);
heim::graphs::in_edges  (g, v);
heim::graphs::out_degree(g, v); // can be deduced from out_edges
heim::graphs::in_degree (g, v); // can be deduced from in_edges
heim::graphs::degree    (g, v); // can be deduced from out_degree and/or in_degree

// edge-related access operations
heim::graphs::source   (g, e);
heim::graphs::target   (g, e);
heim::graphs::endpoints(g, e); // can be deduced from source and target

// vertex-related modification operations
heim::graphs::place_vertex (g);
heim::graphs::remove_vertex(g, v);
heim::graphs::clear_vertex (g, v);

// edge-related modification operations
heim::graphs::place_edge  (g, u, v);
heim::graphs::remove_edge (g, e);
heim::graphs::remove_edges(g, u, v);
```

### Properties
Most of the time a graph's components are tied to certain properties. For example, vertices could be associated with 
*colors* or *locations*, and edges could be associated with *distances* or *costs*. Such properties are even crucial to 
some algorithms, such as [Dijkstra's](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm) or 
[A*](https://en.wikipedia.org/wiki/A*_search_algorithm).

As accessing and writing to properties depend on what form those properties take in a given program, `Heim` provides a 
generic abstraction for *getting* and *setting* properties: the *property map*.

*Property maps* are lightweight objects. They do not hold any property data themselves or even references to other 
containers or objects that are linked to the properties.\
Instead, they only hold functions objects that themselves describe the actions to either get or set a property. This 
lets property values be references from simple containers, or be proxy types created when read and written back when 
set, or any value that can be aggregated into one object that the algorithms can use.

### Algorithms
Graph algorithms are the main reason why the previous interface and property system exist in the first place.

Some libraries provide powerful algorithms, but require users to understand many advanced customization mechanisms 
before they can obtain a useful result. Others provide simple APIs, but at the cost of flexibility, control, or 
performance.\
`Heim` aims to provide the best of both directions by providing, on top of the expert-accessible algorithms, 
layers of simplification.

Here is what that might look like with Dijkstra's shortest paths algorithm:
```c++
#include <heim/graph.hpp>

// simple use case
using graph_type      = /* ... */;
using vertex_type     = heim::graphs::vertex_t    <graph_type>;
using weight_map_type = heim::graphs::property_map<int, /* ... */>;

graph_type      g;
vertex_type     u, v;
weight_map_type weights;

auto res = heim::graphs::dijkstra(g, u, weights);

if (res.reached(v))
  auto const distance = res.distance_to(v);

// more complex use case
using distance_map_type     = heim::graphs::property_map<int        , /* ... */>;
using predecessor_map_type  = heim::graphs::property_map<vertex_type, /* ... */>;
using dijkstra_visitor_type = /* ... */;

distance_map_type     distances;
predecessor_map_type  predecessors;
dijkstra_visitor_type visitor;

auto res = heim::graphs::dijkstra(g, u, weights, distances, predecessors, visitor);

if (res.reached(v))
  auto const distance = res.distance_to(v);
```

`Heim` is set to support many graph algorithms:
- Traversal algorithms;
- Shortest path algorithms;
- Topological sorting;
- Connected component detection;
- Cycle detection;
- Minimum spanning tree algorithms;
- ...

# Installation
...

# License
...