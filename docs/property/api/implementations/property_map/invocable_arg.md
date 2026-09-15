# invocable_arg - Heim: Properties
## Synopsis
```cpp
struct invocable_arg_t { explicit invocable_arg_t() = default; }

inline constexpr invocable_arg_t invocable_arg{};
```

## Overview
Used with [`make_property_map`](make_property_map.md) to create a property map that maps objects to the result of 
given function with them as the argument.

## Code Example
```cpp
#include <array>
#include <map>
#include <print>
#include <ranges>
#include <heim/property.hpp>

int main()
{
  std::array<int, 3>  arr{1, 2, 3};
  std::map<int, char> map{};

  auto size_pm {heim::make_property_map(heim::invocable_arg, std::ranges::size)};
  auto empty_pm{heim::make_property_map(heim::invocable_arg, std::ranges::empty)};

  std::print("{} ", size_pm .get(arr));
  std::print("{} ", empty_pm.get(arr));
  std::print("{} ", size_pm .get(map));
  std::print("{} ", empty_pm.get(map));
}
```

### Output 
```output
3 false 0 true 
```

## See Also
| Name                                        | Description                                      |
|:--------------------------------------------|:-------------------------------------------------|
| [`make_property_map`](make_property_map.md) | Creates a property map using the given arguments |
