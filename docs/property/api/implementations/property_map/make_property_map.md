# make_property_map - Heim: Properties
## Synopsis
```cpp
template<typename ...Args>
[[nodiscard]] constexpr
auto
make_property_map(Args &&...args);
```

### Valid Call Signatures

1.
```cpp
[[nodiscard]] constexpr
auto
make_property_map(identity_arg_t);
```
2.
```cpp
template<typename T>
[[nodiscard]] constexpr
auto
make_property_map(constant_arg_t, T &&t)
requires /* see below */;
```
3.
```cpp
template<typename Cont>
[[nodiscard]] constexpr
auto
make_property_map(associative_container_arg_t, Cont &cont);
```
4.
```cpp
template<typename It>
[[nodiscard]] constexpr
auto
make_property_map(random_access_iterator_arg_t, It it)
requires /* see below */;
```
5.
```cpp
template<typename It, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map(random_access_iterator_arg_t, It it, IndexMap imap)
requires /* see below */;
```
6.
```cpp
template<typename R>
[[nodiscard]] constexpr
auto
make_property_map(random_access_range_arg_t, R &rg)
requires /* see below */;
```
7.
```cpp
template<typename R, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map(random_access_range_arg_t, R &rg, IndexMap imap)
requires /* see below */;
```
8.
```cpp
template<typename M, typename C>
[[nodiscard]] constexpr
auto
make_property_map(member_arg_t, M C::*m);
```
9.
```cpp
[[nodiscard]] constexpr
auto
make_property_map(pointer_arg_t);
```
10.
```cpp
template<typename F>
[[nodiscard]] constexpr
auto
make_property_map(invocable_arg_t, F f)
requires /* see below */;
```
11.
```cpp
template<typename FMap, typename GMap>
[[nodiscard]] constexpr
auto
make_property_map(composable_arg_t, FMap f, GMap g)
requires /* see below */;
```

### Special Call Signatures

1.
```cpp
template<typename K, typename T, typename Cmp, typename Alloc>
[[nodiscard]] constexpr
auto
make_property_map(std::map<K, T, Cmp, Alloc> &cont);
```
2.
```cpp
template<typename K, typename T, typename Cmp, typename Alloc>
[[nodiscard]] constexpr
auto
make_property_map(std::map<K, T, Cmp, Alloc> const &cont);
```
3.
```cpp
template<typename K, typename T, typename Hash, typename KeyEqual, typename Alloc>
[[nodiscard]] constexpr
auto
make_property_map(std::unordered_map<K, T, Hash, KeyEqual, Alloc> &cont);
```
4.
```cpp
template<typename K, typename T, typename Hash, typename KeyEqual, typename Alloc>
[[nodiscard]] constexpr
auto
make_property_map(std::unordered_map<K, T, Hash, KeyEqual, Alloc> const &cont);
```
5.
```cpp
template<typename K, typename T, typename Cmp, typename KCont, typename MCont>
[[nodiscard]] constexpr
auto
make_property_map(std::flat_map<K, T, Cmp, KCont, MCont> &cont);
```
6.
```cpp
template<typename K, typename T, typename Cmp, typename KCont, typename MCont>
[[nodiscard]] constexpr
auto
make_property_map(std::flat_map<K, T, Cmp, KCont, MCont> const &cont);
```
7.
```cpp
template<typename It>
[[nodiscard]] constexpr
auto
make_property_map(It it)
requires /* see below */;
```
8.
```cpp
template<typename It, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map(It it, IndexMap imap)
requires /* see below */;
```
9.
```cpp
template<typename T, std::size_t N>
[[nodiscard]] constexpr
auto
make_property_map(std::span<T, N> cont);
```
10.
```cpp
template<typename T, std::size_t N, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map(std::span<T, N> cont, IndexMap imap)
requires /* see below */;
```
11.
```cpp
template<typename T, std::size_t N>
[[nodiscard]] constexpr
auto
make_property_map(T (&cont)[N]);
```
12.
```cpp
template<typename T, std::size_t N>
[[nodiscard]] constexpr
auto
make_property_map(T const (&cont)[N]);
```
13.
```cpp
template<typename T, std::size_t N, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map(T (&cont)[N], IndexMap imap)
requires /* see below */;
```
14.
```cpp
template<typename T, std::size_t N, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map(T const (&cont)[N], IndexMap imap)
requires /* see below */;
```
15.
```cpp
template<typename T, std::size_t N>
[[nodiscard]] constexpr
auto
make_property_map(std::array<T, N> &cont);
```
16.
```cpp
template<typename T, std::size_t N>
[[nodiscard]] constexpr
auto
make_property_map(std::array<T, N> const &cont);
```
17.
```cpp
template<typename T, std::size_t N, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map(std::array<T, N> &cont, IndexMap imap)
requires /* see below */;
```
18.
```cpp
template<typename T, std::size_t N, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map(std::array<T, N> const &cont, IndexMap imap)
requires /* see below */;
```
19.
```cpp
template<typename T, typename Alloc>
[[nodiscard]] constexpr
auto
make_property_map(std::vector<T, Alloc> &cont);
```
20.
```cpp
template<typename T, typename Alloc>
[[nodiscard]] constexpr
auto
make_property_map(std::vector<T, Alloc> const &cont);
```
21.
```cpp
template<typename T, typename Alloc, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map(std::vector<T, Alloc> &cont, IndexMap imap)
requires /* see below */;
```
22.
```cpp
template<typename T, typename Alloc, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map(std::vector<T, Alloc> const &cont, IndexMap imap)
requires /* see below */;
```
23.
```cpp
template<typename T, typename Alloc>
[[nodiscard]] constexpr
auto
make_property_map(std::deque<T, Alloc> &cont);
```
24.
```cpp
template<typename T, typename Alloc>
[[nodiscard]] constexpr
auto
make_property_map(std::deque<T, Alloc> const &cont);
```
25.
```cpp
template<typename T, typename Alloc, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map(std::deque<T, Alloc> &cont, IndexMap imap)
requires /* see below */;
```
26.
```cpp
template<typename T, typename Alloc, typename IndexMap>
[[nodiscard]] constexpr
auto
make_property_map(std::deque<T, Alloc> const &cont, IndexMap imap)
requires /* see below */;
```
27.
```cpp
template<typename F>
[[nodiscard]] constexpr
auto
make_property_map(F &&f)
requires /* see below */;
```

## Overview
Constructs a [`property_map`](index.md) object with special getters and setters according to certain tags and arguments.

| Name                                                          | Description                                                                                                                                                           |
|:--------------------------------------------------------------|:----------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [`identity_arg`](identity_arg.md)                             | Creates a readable map that returns as property its argument unchanged                                                                                                |
| [`constant_arg`](constant_arg.md)                             | Creates a readable map that returns the same value no matter the key                                                                                                  |
| [`associative_container_arg`](associative_container_arg.md)   | Creates a map that references the given associative container                                                                                                         |
| [`random_access_iterator_arg`](random_access_iterator_arg.md) | Creates a map that associates keys with elements of a random-access sequence by using them, directly or through a given index map, as offsets from the given iterator |
| [`random_access_range_arg`](random_access_range_arg.md)       | Creates a map that associates keys with elements of the given random-access range by using them, directly or through a given index map, as offsets                    |
| [`member_arg`](member_arg.md)                                 | Creates a map that maps objects to one of their member objects or to the return type of one of their nullary member functions                                         |
| [`pointer_arg`](pointer_arg.md)                               | Creates a map that maps pointers to their pointed-to object                                                                                                           |
| [`invocable_arg`](invocable_arg.md)                           | Creates a map that maps objects to the result of the given function with them as the argument                                                                         |
| [`composable_arg`](composable_arg.md)                         | Creates a map that composes the two given property maps                                                                                                               |

### Special Call Signatures
To simplify usage for obvious cases, special call signatures are available.

| Signatures | Description                                                        |
|:-----------|:-------------------------------------------------------------------|
| 1-6.       | Hide [`associative_container_arg`](associative_container_arg.md)   |
| 7-10.      | Hide [`random_access_iterator_arg`](random_access_iterator_arg.md) |
| 11-26.     | Hide [`random_access_range_arg`](random_access_range_arg.md)       |
| 27.        | Hides [`invocable_arg`](invocable_arg.md)                          |

## Template Parameters
| Name   | Description         |
|:-------|:--------------------|
| `Args` | The value type pack |

## Requirements
### Valid call signatures
| Signatures | Requirements                                                                                       |
|:-----------|:---------------------------------------------------------------------------------------------------|
| 2.         | `std::decay_t<T>` must be constructible from `T &&`.                                               |
| 4.         | `It` must be copy constructible and model `std::random_access_iterator`.                           |
| 5.         | `It` and `IndexMap` must be copy constructible, and `It` must model `std::random_access_iterator`. |
| 6.         | `R` must model `std::ranges::random_access_range`.                                                 |
| 7.         | `IndexMap` must be copy constructible and `R` must model `std::ranges::random_access_range`.       |
| 10.        | `F` must be copy constructible.                                                                    |
| 11.        | `FMap` and `GMap` must be copy constructible.                                                      |

### Special call signatures
| Signatures                                  | Requirements                                                                             |
|:--------------------------------------------|:-----------------------------------------------------------------------------------------|
| 7.                                          | `It` must model `std::random_access_iterator`.                                           |
| 8.                                          | `It` must model `std::random_access_iterator` and `IndexMap` must be copy constructible. |
| 10.<br>13-14.<br>17-18.<br>21-22.<br>25-26. | `IndexMap` must be copy constructible.                                                   |
| 27.                                         | `std::remove_reference_t<F>` must be a function type.                                    |

## Parameters
| Name   | Description    |
|:-------|:---------------|
| `args` | The value pack |

## Code Example
```cpp
#include <map>
#include <ranges>
#include <utility>
#include <vector>
#include <heim/property.hpp>

int main()
{
  using pair = std::pair<int, int>;

  std::map<int, int> map{};
  std::vector<int>   vec{};
  auto               it {vec.begin()};

  auto pm1 {heim::make_property_map(heim::identity_arg)};                           // 1.
  auto pm2 {heim::make_property_map(heim::constant_arg, 10.};                       // 2.
  auto pm3 {heim::make_property_map(heim::associative_container_arg, map)};         // 3.
  auto pm4 {heim::make_property_map(heim::random_access_iterator_arg, it)};         // 4.
  auto pm5 {heim::make_property_map(heim::random_access_iterator_arg, it , ac_pm)}; // 5.
  auto pm6 {heim::make_property_map(heim::random_access_range_arg   , vec)};        // 6.
  auto pm7 {heim::make_property_map(heim::random_access_range_arg   , vec, ac_pm)}; // 7.
  auto pm8 {heim::make_property_map(heim::member_arg, &pair::first)};               // 8.
  auto pm9 {heim::make_property_map(heim::pointer_arg)};                            // 9.
  auto pm10{heim::make_property_map(heim::invocable_arg, std::ranges::size)};       // 10.
  auto pm11{heim::make_property_map(heim::composable_arg, ac_pm, ri_pm)};           // 11.
}
```

## See Also
| Name                                                          | Description                                                     |
|:--------------------------------------------------------------|:----------------------------------------------------------------|
| [`make_readable_property_map`](make_readable_property_map.md) | Creates a readable property map object using specific arguments |
| [`make_writable_property_map`](make_writable_property_map.md) | Creates a writable property map object using specific arguments |
