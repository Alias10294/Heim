# random_access_range_arg - Heim: Properties
## Synopsis
```cpp
struct random_access_range_arg_t { explicit random_access_range_arg_t() = default; }

inline constexpr random_access_range_arg_t random_access_range_arg{};
```

## Overview
Used with [`make_property_map`](make_property_map.md) to create a property map that associates keys with elements of a 
random-access range by using them, directly or through an index map, as offsets.

## Code Example
```cpp
#include <print>
#include <vector>
#include <heim/property.hpp>

int main()
{
  std::vector<int> vec{0, 2, 4, 5, 8};
  auto             pm {heim::make_property_map(heim::random_access_range_arg, vec)};

  std::print("{} ", pm.get(0));
  std::print("{} ", pm.get(3));
  
  pm.set(3, 6);
  std::print("{} ", pm.get(3));
}
```

### Output
```output 
0 5 6 
```

## See Also
| Name                                                          | Description                                                      |
|:--------------------------------------------------------------|:-----------------------------------------------------------------|
| [`make_property_map`](make_property_map.md)                   | Creates a property map using the given arguments                 |
| [`random_access_iterator_arg`](random_access_iterator_arg.md) | A tag used to create property maps from a random-access iterator |
