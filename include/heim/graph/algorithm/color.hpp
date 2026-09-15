#ifndef HEIM_GRAPH_ALGORITHM_COLOR_HPP
#define HEIM_GRAPH_ALGORITHM_COLOR_HPP

#include <cstdint>
#include <type_traits>

namespace heim::graphs
{
namespace colors
{
template<typename Clr>
struct white
  : std::integral_constant<Clr, Clr::white>
{ };

template<typename Clr>
inline constexpr auto white_v
= white<Clr>::value;


template<typename Clr>
struct black
  : std::integral_constant<Clr, Clr::black>
{ };

template<typename Clr>
inline constexpr auto black_v
= black<Clr>::value;


template<typename Clr>
struct gray
  : std::integral_constant<Clr, Clr::gray>
{ };

template<typename Clr>
inline constexpr auto gray_v
= gray<Clr>::value;

} // namespace colors


enum class color
  : std::uint8_t
{
  white = 0,
  black = 1,
  gray
};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_COLOR_HPP
