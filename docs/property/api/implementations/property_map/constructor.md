# property_map::property_map - Heim: Properties
## Synopsis
```cpp
constexpr
property_map(getter_type g, setter_type s);
```

## Overview
Constructs `this` using the given function objects `g` and `s`.

## Requirements
`getter_type` and `setter_type` have to be [`movable`](https://cppreference.com/cpp/concepts/movable) types.

## Parameters
| Name | Description                       |
|:-----|:----------------------------------|
| `g`  | The getter function object to use |
| `s`  | The setter function object to use |

## Code Example
```cpp
#include <heim/property.hpp>

int main()
{
  auto getter{[](int k) { return 0; }};
  auto setter{[](int k, int v) { }};

  heim::property_map r{getter, heim::no_property_setter};
  static_assert( heim::properties::readable_map_for<decltype(r), int>);
  static_assert(!heim::properties::writable_map_for<decltype(r), int, int>);

  heim::property_map w{heim::no_property_getter, setter};
  static_assert(!heim::properties::readable_map_for<decltype(w), int>);
  static_assert( heim::properties::writable_map_for<decltype(w), int, int>);

  heim::property_map rw{getter, setter};
  static_assert(heim::properties::readable_map_for<decltype(rw), int>);
  static_assert(heim::properties::writable_map_for<decltype(rw), int, int>);
}
```

## Notes
All other constructors are left implicitely declared.

## See Also
| Name                                                                             | Description                                                              |
|:---------------------------------------------------------------------------------|:-------------------------------------------------------------------------|
| [`no_property_getter`](no_property.md)<br>[`no_property_setter`](no_property.md) | Denote the construction of readable/writable-only `property_map` objects |