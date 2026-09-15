# identity_arg - Heim: Properties
## Synopsis
```cpp
struct identity_arg_t { explicit identity_arg_t() = default; }

inline constexpr identity_arg_t identity_arg{};
```

## Overview
Used with [`make_property_map`](make_property_map.md) to create a readable property map that returns as property its 
argument unchanged.

## Code Example
```cpp
#include <print>
#include <heim/property.hpp>

int main()
{
  auto pm{heim::make_property_map(heim::identity_arg)};

  std::print("{} ", pm.get("hello, world !"));
  std::print("{} ", pm.get(0));
}
```

### Output 
```output 
hello, world ! 0 
```

## See Also
| Name                                                          | Description                                               |
|:--------------------------------------------------------------|:----------------------------------------------------------|
| [`make_property_map`](make_property_map.md)                   | Creates a property map using the given arguments          |
| [`make_readable_property_map`](make_readable_property_map.md) | Creates a readable property map using the given arguments |
