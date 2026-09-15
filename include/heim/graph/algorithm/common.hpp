#ifndef HEIM_GRAPH_ALGORITHM_COMMON_HPP
#define HEIM_GRAPH_ALGORITHM_COMMON_HPP

#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>
#include "heim/property/property_map.hpp"

namespace heim::graphs
{
struct default_property_map_t
{ };

inline constexpr
default_property_map_t
default_property_map{};


namespace detail
{
struct no_property_storage_t
{ };

inline constexpr
no_property_storage_t
no_property_storage{};


template<typename Map, typename Storage>
[[nodiscard]] constexpr
auto
resolve_storage()
{
  if constexpr (std::same_as<std::remove_cvref_t<Map>, default_property_map_t>)
    return std::make_unique<Storage>();
  else
    return no_property_storage;
}

template<typename Map, typename Storage>
[[nodiscard]] constexpr
decltype(auto)
resolve_property_map(Map &&m, Storage &s)
{
  if constexpr (std::same_as<Map, default_property_map_t>)
    return make_property_map(*s);
  else
    return std::forward<Map>(m);
}

} // namespace detail


template<typename Vertex>
struct search_target
{
private:
  Vertex m_v;
  bool   m_has_target;

public:
  constexpr
  search_target()
    : m_v{}, m_has_target{false}
  { }

  constexpr
  search_target(Vertex v)
    : m_v{std::move(v)}, m_has_target{true}
  { }


  constexpr
  bool
  operator()(Vertex v) const
  { return m_has_target ? m_v == v : false; }
};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_COMMON_HPP
