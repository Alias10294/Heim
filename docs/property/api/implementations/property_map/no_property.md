# no_property_getter, no_property_setter - Heim: ...
## Synopsis
```cpp
struct no_property_getter_t { explicit no_property_getter_t() = default; }
struct no_property_setter_t { explicit no_property_setter_t() = default; }

inline constexpr no_property_getter_t no_property_getter{};
inline constexpr no_property_setter_t no_property_setter{};
```

## Overview
Defines a `property map` object, when used at construction, as not readable and/or not writable.

## Code Example
```cpp
#include <heim/property.hpp>

int main()
{
  auto getter{[](int k) { return 0; }};
  auto setter{[](int k, int v) { }};

  heim::property_map r{getter, no_property_setter};
  static_assert( heim::properties::readable_map_for<decltype(r), int>);
  static_assert(!heim::properties::writable_map_for<decltype(r), int, int>);

  heim::property_map w{no_property_getter, setter};
  static_assert(!heim::properties::readable_map_for<decltype(w), int>);
  static_assert( heim::properties::writable_map_for<decltype(w), int, int>);

  heim::property_map rw{getter, setter};
  static_assert(heim::properties::readable_map_for<decltype(rw), int>);
  static_assert(heim::properties::writable_map_for<decltype(rw), int, int>);
}
```

## Notes 
...

## See Also
| Name | Description |
|:-----|:------------|