# properties::is_reflective_map_for - Heim: Properties
## Synopsis
```cpp
template<typename M, typename K>
struct is_reflective_map_for;

template<typename M, typename K>
inline constexpr bool is_reflective_map_for_v
= is_reflective_map_for<M, K>::value;

template<typename M, typename K>
concept reflective_map_for
= is_reflective_map_for_v<M, K>;
```

## Overview
Checks whether instances of the property map type `M` can access properties with keys of type `K` and write back to 
properties with the accessed objects' type.

## Template Parameters
| Name | Description           |
|:-----|:----------------------|
| `M`  | The property map type |
| `K`  | The key type          |

## See Also
| Name                                                        | Description                                                                             |
|:------------------------------------------------------------|:----------------------------------------------------------------------------------------|
| [`properties::is_map_for`](is_map_for.md)                   | Specifies that a type is both a readable and writable property map type for given types |
| [`properties::readable_map_result`](readable_map_result.md) | Determines the result type of a call to [`properties::get`](../primitives/get.md)       |
