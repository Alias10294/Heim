# pointer_arg - Heim: Properties
## Synopsis
```cpp
struct pointer_arg_t { explicit pointer_arg_t() = default; }

inline constexpr pointer_arg_t pointer_arg{};
```

## Overview
Used with [`make_property_map`](make_property_map.md) to create a property map that maps pointers to their pointed-to 
object.

## Code Example
```cpp
#include <memory>
#include <print>
#include <heim/property.hpp>

int main()
{
  auto pm  {heim::make_property_map(heim::pointer_arg)};
  int  x   {1};
  auto uptr{std::make_unique<int>(2)};
  auto sptr{std::make_shared<int>(3)};

  std::print("{} ", pm.get(&x);
  std::print("{} ", pm.get(uptr);
  std::print("{} ", pm.get(sptr);
  std::println();
  
  pm.set(&x  , 10);
  pm.set(uptr, 20);
  pm.set(sptr, 30);
  
  std::print("{} ", pm.get(&x);
  std::print("{} ", pm.get(uptr);
  std::print("{} ", pm.get(sptr);
}
```

### Output
```output 
1 2 3 
10 20 30 
```

## See Also
| Name                                        | Description                                      |
|:--------------------------------------------|:-------------------------------------------------|
| [`make_property_map`](make_property_map.md) | Creates a property map using the given arguments |
