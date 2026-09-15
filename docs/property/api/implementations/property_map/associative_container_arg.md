# associative_container_arg - Heim: Properties
## Synopsis
```cpp
struct associative_container_arg_t { explicit associative_container_arg_t() = default; }

inline constexpr associative_container_arg_t associative_container_arg{};
```

## Overview
Used with [`make_property_map`](make_property_map.md) to create a property map that references a given associative 
container.

## Code Example
```cpp
#include <flat_map>
#include <map>
#include <print>
#include <string>
#include <unordered_map>
#include <heim/property.hpp>

int main()
{
  std::flat_map     <int, std::string> fm{{0, "A"}, {1, "B"}, {2, "C"}};
  std::map          <int, std::string> m {{0, "A"}, {1, "B"}, {2, "C"}};
  std::unordered_map<int, std::string> um{{0, "A"}, {1, "B"}, {2, "C"}};

  auto fpm{heim::make_property_map(heim::associative_container_arg, fm)};
  auto pm {heim::make_property_map(heim::associative_container_arg, m)};
  auto upm{heim::make_property_map(heim::associative_container_arg, um)};

  std::print("{} ", fpm.get(0));
  std::print("{} ", pm .get(1));
  std::print("{} ", upm.get(2));
  
  pm.set(3, "D");
  std::print("{} ", m.at(3));
}
```

### Output
```output 
A B C D 
```

## See Also
| Name                                        | Description                                      |
|:--------------------------------------------|:-------------------------------------------------|
| [`make_property_map`](make_property_map.md) | Creates a property map using the given arguments |
