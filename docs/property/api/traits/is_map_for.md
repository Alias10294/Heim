# properties::is_map_for - Heim: Properties
## Synopsis
```cpp
template<typename M, typename K, typename ...Args>
struct is_map_for;

template<typename M, typename K, typename ...Args>
inline constexpr bool is_map_for_v
= is_map_for<M, K, Args ...>::value;

template<typename M, typename K, typename ...Args>
concept map_for
= is_map_for_v<M, K, Args ...>;
```

## Overview
Checks whether instances of the property map type `M` can both access properties with keys of type `K` and modify them 
with value arguments of types `Args`.

## Template Parameters
| Name   | Description           |
|:-------|:----------------------|
| `M`    | The property map type |
| `K`    | The key type          |
| `Args` | The value type pack   |

## See Also
| Name                                                            | Description                                                                                |
|:----------------------------------------------------------------|:-------------------------------------------------------------------------------------------|
| [`properties::is_readable_map_for`](is_readable_map_for.md)     | Checks the validity of a call to [`properties::get`](../primitives/get.md) for given types |
| [`properties::is_writable_map_for`](is_writable_map_for.md)     | Checks the validity of a call to [`properties::set`](../primitives/set.md) for given types |
| [`properties::is_reflective_map_for`](is_reflective_map_for.md) | Checks that a type is a property map that can write back accessed properties               |
