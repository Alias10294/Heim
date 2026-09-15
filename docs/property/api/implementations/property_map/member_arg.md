# member_arg - Heim: Properties
## Synopsis
```cpp
struct member_arg_t { explicit member_arg_t() = default; }

inline constexpr member_arg_t member_arg{};
```

## Overview
Used with [`make_property_map`](make_property_map.md) to create a property map that maps objects to one of their member 
objects or to the return type of one of their nullary member functions.

## Code Example
```cpp
#include <optional>
#include <print>
#include <string>
#include <heim/property.hpp>

int main()
{
  std::pair<int, std::string> p{0, "A"};
  std::optional<std::string>  o{std::string{"hello, world !"}};

  auto pfpm{heim::make_property_map(heim::member_arg, &pair::first)};
  auto pspm{heim::make_property_map(heim::member_arg, &pair::second)};
  auto opm {heim::make_property_map(heim::member_arg, &opt ::value)};

  std::print("{} ", pfpm.get(p));
  std::print("{} ", pspm.get(p));
  std::print("{} ", opm .get(o));
  std::println();
  
  pfpm.set(p, 1);
  pspm.set(p, "B");
  opm .set(o, "goodbye !");
  
  std::print("{} ", pfpm.get(p));
  std::print("{} ", pspm.get(p));
  std::print("{} ", opm .get(o));
}
```

### Output
```output
0 A hello, world !
1 B goodbye !
```

## See Also
| Name                                        | Description                                      |
|:--------------------------------------------|:-------------------------------------------------|
| [`make_property_map`](make_property_map.md) | Creates a property map using the given arguments |
