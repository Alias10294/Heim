# properties::get - Heim: Properties
## Synopsis
```cpp
namespace properties
{
  inline constexpr /* detail */ get{};
}
```
```cpp
template<typename M, typename K>
[[nodiscard]] constexpr
decltype(auto) 
operator()(M &&m, K &&k) const;
```
## Overview
Returns the value associated to the key `k` by the property map `m`.

### Customization point object
A call to `properties::get(m, k)` is equivalent to:

1. `m.get(k)` if it exists,
2. otherwise, `get(m, k)`, where `get` is found through 
   [*argument-dependent lookup (ADL)*](https://cppreference.com/cpp/language/adl).

## Template Parameters
| Name | Description           |
|:-----|:----------------------|
| `M`  | The property map type |
| `K`  | The key type          |

## Parameters
| Name | Description                                    |
|:-----|:-----------------------------------------------|
| `m`  | The property map                               |
| `k`  | The key whose associated property is to access |

## Code Example 
```cpp
#include <map>
#include <memory>
#include <print>
#include <string>
#include <heim/property.hpp>

int main()
{
  std::map<int, std::string>   um{{0, "A"}, {1, "B"}};
  std::unique_ptr<std::string> up{std::make_unique<std::string>("unique_ptr")};

  auto um_map{heim::make_property_map(heim::associative_container_arg, um)};
  auto up_map{heim::make_property_map(heim::pointer_arg)};

  std::print("{} ", heim::properties::get(um_map, 0));
  std::print("{} ", heim::properties::get(um_map, 1));
  std::print("{} ", heim::properties::get(up_map, up));
}
```

### Output
```output
A B unique_ptr 
```

## See Also
| Name                                                                  | Description                                                                  |
|:----------------------------------------------------------------------|:-----------------------------------------------------------------------------|
| [`properties::set`](set.md)                                           | Modifies the property associated to a key                                    |
| [`properties::is_readable_map_for`](../traits/is_readable_map_for.md) | Checks the validity of a call to [`properties::get`](get.md) for given types |
| [`properties::readable_map_result`](../traits/readable_map_result.md) | Determines the result type of a call to [`properties::get`](get.md)          |