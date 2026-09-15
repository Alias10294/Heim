# properties::is_writable_map_for - Heim: Properties
## Synopsis
```cpp
template<typename M, typename K, typename ...Args>
struct is_writable_map_for;

template<typename M, typename K, typename ...Args>
inline constexpr bool is_writable_map_for_v
= is_writable_map_for<M, K, Args ...>::value;

template<typename M, typename K, typename ...Args>
concept writable_map_for
= is_writable_map_for_v<M, K, Args ...>;
```

## Overview
Checks whether instances of the property map type `M` can modify properties with keys of type `K` and value arguments 
of types `Args`.

## Template Parameters
| Name       | Description            |
|:-----------|:-----------------------|
| `M`        | The property map type  |
| `K`        | The key type           |
| `Args ...` | The value type pack    |

## See Also
| Name                                                        | Description                                                                                 |
|:------------------------------------------------------------|:--------------------------------------------------------------------------------------------|
| [`properties::is_readable_map_for`](is_readable_map_for.md) | Checks the validity of a call to [`properties::get`](../primitives/get.md) for given types  |
| [`properties::is_map_for`](is_map_for.md)                   | Specifies that a type is both a readable and writable property map type for the given types |
