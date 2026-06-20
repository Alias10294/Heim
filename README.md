# Heim: Organize Your Data The Right Way
<code>Heim</code> is a zero-dependency, header-only C++23 library for data-oriented game and simulation programming. 
Here are its main features:
- A hypermodern C++23 API;
- A generic take on the Entity-Component-System (ECS) pattern;
- Powerful template metaprogramming utilities such as the `type_sequence`;
- [WIP] A modern graph library inspired by the famous 
[Boost Graph Library (BGL)](https://www.boost.org/doc/libs/latest/libs/graph/doc/index.html).

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
    auto       &[px, py, pz] = reg.get<position>(e);
    auto const &[vx, vy, vz] = reg.get<velocity>(e);

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
  - [Inspiration](#inspiration)
  - [Implementation](#implementation)
    - [Interface](#interface)
    - [Properties](#properties)
    - [Algorithms](#algorithms)
- [Installation](#installation)
- [License](#license)

# Introduction
...

# Entity-Component-System pattern
...

# Metaprogramming utilities
...

# Graphs
Whether it is in <code>Heim</code> itself or in the projects that use it, chances are that graphs or graph-like 
structures are going to appear. Examples of situations where graphs are useful or even <i>critical</i> vary: rendering, 
physics simulation, pathfinding, procedural generation, and <i>many</i> more.

Because having the right data structures and algorithms at hand in all those situations goes a long way towards both 
speeding up development and ensuring high performance on often important systems, <code>Heim</code> aims to provide 
<i>just that</i>.

> Please note that this section of the library is still underway, and that the information below is <i>not</i> 
guaranteed to be complete or to even be valid in future versions of the library.  

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
programmers, which is far from what it could be.<br>
<code>Heim</code> aims to bring those ideas into a more modern C++, using new principles such as <b>concepts</b>, 
<b>ranges</b> and <b>customization point objects</b> to implement a newer and easier-to-use library.

### Interface
<code>Heim</code>'s graphs' interface, instead of revolving around a centralized <code>graph_traits</code> object that 
needs every method and alias individually specialized, will make use of 
[customization point objects](https://en.cppreference.com/cpp/named_req/CustomizationPointObject). This allows more 
flexibility and control over their specialization whilst also guaranteeing robust default implementations for some of 
them.<br>
Here is a first glance at what this interface might look like:
```c++
using graph_type        = /* ... */;
using vertex_descriptor = heim::graphs::vertex_descriptor_t<graph_type>;
using edge_descriptor   = heim::graphs::edge_descriptor_t  <graph_type>;

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
<i>colors</i> or <i>locations</i>, and edges could be associated with <i>distances</i> or <i>costs</i>. Such properties 
are even crucial to some algorithms, such as [Dijkstra's](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm) or 
[A*](https://en.wikipedia.org/wiki/A*_search_algorithm).

As accessing and writing to properties depend on what form those properties take in a given program, <code>Heim</code>
provides a generic abstraction for <i>getting</i> and <i>setting</i> properties: the <i>property map</i>.

<i>Property maps</i> are lightweight objects. They do not hold any property data themselves or even references to other 
containers or objects that are linked to the properties.<br>
Instead, they only hold functions objects that themselves describe the actions to either get or set a property. This 
lets property values be references from simple containers, or be proxy types created when read and written back when 
set, or any value that can be aggregated into one object that the algorithms can use.

### Algorithms
...

# Installation
...

# License
...