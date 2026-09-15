# Heim: Properties
This library proposes a common interface to represent and interact with properties of objects independently of their 
implementation. This can promote reusability and genericity in various applications, effectively abstracting the 
concepts of *getter* and *setter*.

It is itself used in the [Graphs](../graph/index.md) library in many of its algorithms.

## Overview
### Introduction
Often times, in object-oriented programming, properties represent some part of a class that needs to be read and/or 
written to.\
In reality, *when you think about it*, properties could designate *anything* associated to some object; the 
data member of an object, the result of a function applied to an object, the value associated to an object by a 
hash map, and countless more. This implies that algorithms (or code in general) that manipulate properties of objects 
need a generic abstraction in order to avoid problems such as strong coupling to the properties' origin or constrained 
reusability. This is what Heim provides with this library.

### Property mapping
Just as iterators in the C++ standard library provide an abstraction for manipulating sequential data structures, Heim 
develops the concept of the *property map*.

A property map is a lightweight, cheap-to-copy object that represents a specific association between keys and their 
properties. It is not intended to own but only reference/forward the means of associating the keys and properties.\
As such, the same property map interface can represent completely different underlying mechanisms.

### Property access
Using its property maps, this library provides a common interface for accessing properties. It relies on 
[customization point objects](https://cppreference.com/cpp/named_req/CustomizationPointObject), and consists of two 
operations:

+ [`properties::get`](api/primitives/get.md), the *getter*, which returns the given key's associated property according 
  to the given property map.
+ [`properties::set`](api/primitives/set.md), the *setter*, which modifies the given key's associated property 
  according to the given property map.

Consequently, property maps are said to be respectively *readable* and/or *writable* depending on which operations they 
support. This denomination is explicitly supported by a [comprehensive set of type traits and concepts](#traits).

### Customizability
The property map concept being entirely independent from any particular implementation, any type for which the 
appropriate [`properties::get`](api/primitives/get.md) and [`properties::set`](api/primitives/set.md) operations are 
available can be used as a property map type.

Nevertheless, this library provides [`property_map`](api/implementations/property_map/index.md) as a general-purpose 
implementation of the concept and that relies on 
[function objects](https://cppreference.com/cpp/named_req/FunctionObject).\
This implementation is also backed by [`make_property_map`](api/implementations/property_map/make_property_map.md), an 
adaptor function that helps create [`property_map`](api/implementations/property_map/index.md) objects from common or 
known patterns. It can also help compose [`property_map`](api/implementations/property_map/index.md) objects to 
construct more complex associations.

## Getting Started
### Installation
An installation guide for Heim as a whole can be found directly on the [README](../../README.md#getting-started) or 
[main](../index.md#getting-started) pages.

### Creating property maps
To create property maps from simple representations, use the
[`make_property_map`](api/implementations/property_map/make_property_map.md) function.
```cpp
std::map<int, int> m;

// map keys to their values in an associative container
auto m_pm = heim::make_property_map(m);

// map values to their logical inverse
auto n_pm = heim::make_property_map(std::logical_not<>);

// map strings to their hashed value
auto h_pm = heim::make_property_map(std::hash<std::string>{});

// and more ...
```

Property maps can also be directly constructed using 
[`property_map`'s constructor](api/implementations/property_map/constructor.md) and custom 
[function objects](https://cppreference.com/cpp/named_req/FunctionObject).
```cpp
auto getter = [](auto &&k) { return 0; };
auto setter = [](auto &&k, auto &&...args) { };

auto pm = heim::property_map{getter, setter};
```

### Accessing properties
Use the library's interface to access properties.
```cpp
// read access to a property
auto prop = heim::properties::get(pmap, key);

// write access to a property
heim::properties::set(pmap, key, prop);
```

### Composing property maps
When associations can be represented by combining multiple other associations, 
[`make_property_map`](api/implementations/property_map/make_property_map.md) can also be used along with 
[`composable_arg`](api/implementations/property_map/composable_arg.md) to combine their respective property maps.
```cpp
// x(key) <=> a(b(key))
auto x = heim::make_property_map(heim::composable_arg, a, b);

// y(key) <=> x(c(key)) <=> a(b(c(key)))
auto y = heim::make_property_map(heim::composable_arg, x, c);
```

### Implementing property maps
If what this library does not provide a fitting property map, or if the provided solutions lack specific 
functionalities, custom property map types can easily be implemented and have very few requirements:

+ be usable through [`properties::get`](api/primitives/get.md) and/or [`properties::set`](api/primitives/set.md), that 
  is provide equivalent `get` and `set` as either member or free functions;
+ not own the resources they refer to, as they are intended to be cheaply copied for algorithms to use.

## Code Example
```cpp
#include <print>
#include <unordered_map>
#include <vector>
#include <heim/property.hpp>

struct entity { int hp; };

int main()
{
  std::unordered_map<int, int> map{{0, 100}, {1, 80}, {2, 60}};
  std::vector<entity>          vec{{100}, {80}, {60}};
  
  auto map_pm{heim::make_property_map(map)};
  auto vec_pm{heim::make_property_map(
      heim::composable_arg,
      heim::make_property_map(heim::member_arg, &entity::hp),
      heim::make_property_map(vec)};
      
  std::print("{} ", heim::properties::get(map_pm, 1));
  std::print("{} ", heim::properties::get(vec_pm, 1));
  
  heim::properties::set(map_pm, 1, 20);
  heim::properties::set(vec_pm, 1, 20);
  
  std::print("{} ", heim::properties::get(map_pm, 1));
  std::print("{} ", heim::properties::get(vec_pm, 1));
}
```

## API Reference
### Primitives
| Name                                       | Description                               |
|--------------------------------------------|-------------------------------------------|
| [`properties::get`](api/primitives/get.md) | Returns the property associated to a key  |
| [`properties::set`](api/primitives/set.md) | Modifies the property associated to a key |

### Traits
| Name                                                                       | Description                                                                                 |
|----------------------------------------------------------------------------|---------------------------------------------------------------------------------------------|
| [`properties::is_readable_map_for`](api/traits/is_readable_map_for.md)     | Checks the validity of a call to [`properties::get`](api/primitives/get.md) for given types |
| [`properties::is_writable_map_for`](api/traits/is_writable_map_for.md)     | Checks the validity of a call to [`properties::set`](api/primitives/set.md) for given types |
| [`properties::is_map_for`](api/traits/is_map_for.md)                       | Specifies that a type is both a readable and writable property map type for given types     |
| [`properties::is_reflective_map_for`](api/traits/is_reflective_map_for.md) | Checks that a type is a property map that can write back accessed properties                |
| [`properties::readable_map_result`](api/traits/readable_map_result.md)     | Determines the result type of a call to [`properties::get`](api/primitives/get.md)          |

### Implementations
| Name                                                        | Description                                                                                    |
|-------------------------------------------------------------|------------------------------------------------------------------------------------------------|
| [`property_map`](api/implementations/property_map/index.md) | A versatile and generic implementation of the property map concept relying on function objects |