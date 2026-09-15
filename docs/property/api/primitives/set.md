# properties::set - Heim: Properties
## Synopsis
```cpp
namespace properties
{
  inline constexpr /* detail */ set{};
}
```
```cpp
template<typename M, typename K, typename ...Args>
[[nodiscard]] constexpr
void 
operator()(M &&m, K &&k, Args &&...args) const;
```
## Overview
Modifies the property associated to the key `k` by the property map `m` using the argument pack `args`.

### Customization point object
A call to `properties::set(m, k, args...)` is equivalent to:

1. `m.set(k, args...)` if it exists,
2. otherwise, `set(m, k, args...)`, where `set` is found through 
   [*argument-dependent lookup (ADL)*](https://cppreference.com/cpp/language/adl).

## Template Parameters
| Name   | Description           |
|:-------|:----------------------|
| `M`    | The property map type |
| `K`    | The key type          |
| `Args` | The value type pack   |

## Parameters
| Name   | Description                                    |
|:-------|:-----------------------------------------------|
| `m`    | The property map                               |
| `k`    | The key whose associated property is to access |
| `args` | The value pack                                 |

## Code Example 
```cpp
#include <map>
#include <memory>
#include <print>
#include <string>
#include <heim/property.hpp>

int main()
{
  std::map<int, std::string>   um{{0, "A"}, {1, "C"}};
  std::unique_ptr<std::string> up{std::make_unique<std::string>("shared_ptr")};

  auto um_map{heim::make_property_map(heim::associative_container_arg, um)};
  auto up_map{heim::make_property_map(heim::pointer_arg)};

  std::print("{} ", um.at(1));
  std::print("{} ", *up);

  heim::properties::set(um_map, 1 , "B");
  heim::properties::set(up_map, up, "unique_ptr");

  std::print("{} ", um.at(1));
  std::print("{} ", *up);
}
```

### Output
```output
A B unique_ptr 
```

## See Also
| Name                                                                  | Description                                                                  |
|:----------------------------------------------------------------------|:-----------------------------------------------------------------------------|
| [`properties::get`](get.md)                                           | Returns the property associated to a key                                     |
| [`properties::is_writable_map_for`](../traits/is_writable_map_for.md) | Checks the validity of a call to [`properties::set`](set.md) for given types |