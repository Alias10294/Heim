# name - Heim: ...
## Synopsis
```cpp
template<typename ...Args>
struct name;

template<typename ...Args>
using name_t
= typename name<Args ...>::type;

template<typename ...Args>
inline constexpr bool name_v
= name<Args ...>::value;

template<typename ...Args>
concept name_concept
= name_v<Args ...>;
```

## Overview
...

## Template Parameters
| Name   | Description                      |
|:-------|:---------------------------------|
| `Args` | The template parameter type pack |

## Notes 
...

## See Also
| Name | Description |
|:-----|:------------|