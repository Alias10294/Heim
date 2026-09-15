# property_map::get - Heim: Properties
## Synopsis
```cpp
template<typename K>
[[nodiscard]] constexpr
decltype(auto)
get(K &&k);
```
```cpp
template<typename K>
[[nodiscard]] constexpr
decltype(auto)
get(K &&k) const;
```

## Overview
Invokes `this`'s getter function object with `k` as its argument and returns its exact result.

## Template Parameters
| Name | Description  |
|:-----|:-------------|
| `K`  | The key type |

## Parameters
| Name | Description                                   |
|:-----|:----------------------------------------------|
| `k`  | The key whose associated property is accessed |

## Code Example
```cpp
#include <cstddef>
#include <print>
#include <vector>
#include <heim/property.hpp>

int main()
{
  std::vector<int>   vec   {0, 2, 4};
  auto               getter{[&vec](std::size_t k) { return vec[k]; }};
  heim::property_map pm    {getter, heim::no_property_setter};

  std::print("{} ", pm.get(0));
  std::print("{} ", pm.get(1));
  std::print("{} ", pm.get(2));
}
```

### Output
```output
0 2 4 
```

## See Also
| Name                                         | Description                                       |
|:---------------------------------------------|:--------------------------------------------------|
| [`set`](set.md)                              | Modifies the property associated with a given key |
| [`properties::get`](../../primitives/get.md) | Returns the property associated to a key          |
