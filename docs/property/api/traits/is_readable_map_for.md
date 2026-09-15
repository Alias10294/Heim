# properties::is_readable_map_for - Heim: Properties
## Synopsis
```cpp
template<typename M, typename K>
struct is_readable_map_for;

template<typename M, typename K>
inline constexpr bool is_readable_map_for_v
= is_readable_map_for<M, K>::value;

template<typename M, typename K>
concept readable_map_for
= is_readable_map_for_v<M, K>;
```

## Overview
Checks whether instances of the property map type `M` can access properties with keys of type `K`.

## Template Parameters
| Name | Description           |
|:-----|:----------------------|
| `M`  | The property map type |
| `K`  | The key type          |

## See Also
| Name                                                        | Description                                                                                 |
|:------------------------------------------------------------|:--------------------------------------------------------------------------------------------|
| [`properties::readable_map_result`](readable_map_result.md) | Determines the result type of a call to [`properties::get`](../primitives/get.md)           |
| [`properties::is_writable_map_for`](is_writable_map_for.md) | Checks the validity of a call to [`properties::set`](../primitives/set.md) for given types  |
| [`properties::is_map_for`](is_map_for.md)                   | Specifies that a type is both a readable and writable property map type for the given types |
