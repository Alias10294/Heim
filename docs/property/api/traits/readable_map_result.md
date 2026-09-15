# properties::readable_map_result - Heim: Properties
## Synopsis
```cpp
template<typename M, typename K>
requires readable_map_for<M, K>
struct readable_map_result;

template<typename M, typename K>
using readable_map_result_t
= typename readable_map_result<M, K>::type;
```

## Overview
Computes the result type of the [`properties::get`](../primitives/get.md) operation for the property map `M` and key 
type `K`.

## Template Parameters
| Name | Description           |
|:-----|:----------------------|
| `M`  | The property map type |
| `K`  | The key type          |

## See Also
| Name                                                        | Description                                                                                |
|:------------------------------------------------------------|:-------------------------------------------------------------------------------------------|
| [`properties::is_readable_map_for`](is_readable_map_for.md) | Checks the validity of a call to [`properties::get`](../primitives/get.md) for given types |
