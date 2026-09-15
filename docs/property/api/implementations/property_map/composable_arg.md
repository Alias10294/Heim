# composable_arg - Heim: Properties
## Synopsis
```cpp
struct composable_arg_t { explicit composable_arg_t() = default; }

inline constexpr composable_arg_t composable_arg{};
```

## Overview
Used with [`make_property_map`](make_property_map.md) to create a property map that composes two given property maps.

## Code Example
```cpp
#include <map>
#include <memory>
#include <print>
#include <heim/property.hpp>

int main()
{
  std::map<int, std::unique_ptr<char>> map{{0, std::make_unique<char>('a')}};

  auto ppm {heim::make_property_map(heim::pointer_arg)};
  auto acpm{heim::make_property_map(heim::associative_container_arg, map)};
  auto cpm {heim::make_property_map(heim::composable_arg, ppm, acpm)};

  std::print("{} ", p_map.get(ac_map.get(0)));
  std::print("{} ", c_map.get(0));
}
```

### Output
```output
a a 
```

## See Also
| Name                                        | Description                                      |
|:--------------------------------------------|:-------------------------------------------------|
| [`make_property_map`](make_property_map.md) | Creates a property map using the given arguments |
