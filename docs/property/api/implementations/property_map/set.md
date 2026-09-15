# property_map::set - Heim: Properties
## Synopsis
```cpp
template<typename K, typename ...Args>
constexpr
void
set(K &&k, Args &...&args)
noexcept(std::is_nothrow_invocable_v<setter_type &, K &&, Args &&...>)
requires std::is_invocable_v        <setter_type &, K &&, Args &&...>;
```
```cpp
template<typename K, typename ...Args>
constexpr
void
set(K &&k, Args &&...args) const
noexcept(std::is_nothrow_invocable_v<setter_type const &, K &&, Args &&...>)
requires std::is_invocable_v        <setter_type const &, K &&, Args &&...>;
```

## Overview
Invokes `this`'s setter function object with `k` as its first argument and `args` as its following arguments.

## Template Parameters
| Name   | Description         |
|:-------|:--------------------|
| `K`    | The key type        |
| `Args` | The value type pack |

## Parameters
| Name   | Description                                      |
|:-------|:-------------------------------------------------|
| `k`    | The key whose associated property is assigned to |
| `args` | The value pack                                   |

## Code Example
```cpp
#include <cstddef>
#include <print>
#include <vector>
#include <heim/property.hpp>

int main()
{
  std::vector<int>   vec   {0, 2, 4};
  auto               setter{[&vec](std::size_t const k; int v) { vec[k] = v; }};
  heim::property_map pm    {heim::no_property_getter, setter};

  std::print("{} ", vec[0]);
  pm.set(0, 1);
  std::print("{} ", vec[0]);
}
```

### Output
```output
0 1 
```

## See Also
| Name                                         | Description                                     |
|:---------------------------------------------|:------------------------------------------------|
| [`get`](get.md)                              | Returns the property associated to a given key  |
| [`properties::set`](../../primitives/set.md) | Modifies the property associated to a given key |
