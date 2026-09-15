# constant_arg - Heim: Properties
## Synopsis
```cpp
struct constant_arg_t { explicit constant_arg_t() = default; }

inline constexpr constant_arg_t constant_arg{};
```

## Overview
Used with [`make_property_map`](make_property_map.md) to create a readable property map that returns the same value no 
matter the given key.

## Code Example
```cpp
#include <print>
#include <heim/property.hpp>

int main()
{
  auto pm{heim::make_property_map(heim::constant_arg, 7)};

  std::print("{} ", pm.get("hello, world !"));
  std::print("{} ", pm.get(0));
}
```

### Output
```output 
7 7 
```

## See Also
| Name                                                          | Description                                               |
|:--------------------------------------------------------------|:----------------------------------------------------------|
| [`make_property_map`](make_property_map.md)                   | Creates a property map using the given arguments          |
| [`make_readable_property_map`](make_readable_property_map.md) | Creates a readable property map using the given arguments |
