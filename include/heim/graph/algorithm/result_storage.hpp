#ifndef HEIM_GRAPH_ALGORITHM_RESULT_STORAGE_HPP
#define HEIM_GRAPH_ALGORITHM_RESULT_STORAGE_HPP

#include <concepts>
#include <memory>
#include <unordered_map>
#include <utility>
#include <type_traits>
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
template<
    typename Storage,
    typename Map>
class result_storage
{
private:
  struct empty_property_storage_t
  { };


  [[nodiscard]] static constexpr
  auto
  s_resolve_storage()
  {
    if constexpr (std::same_as<std::remove_cvref_t<Map>, default_property_map_t>)
      return std::make_unique<Storage>();
    else
      return empty_property_storage_t{};
  }

  template<typename S>
  [[nodiscard]] static constexpr
  auto
  s_resolve_map(Map m, S &s)
  {
    if constexpr (std::same_as<std::remove_cvref_t<Map>, default_property_map_t>)
      return heim::make_property_map(*s);
    else
      return std::forward<Map>(m);
  }


  using storage_type
  = decltype(s_resolve_storage());

public:
  using property_map_type
  = decltype(s_resolve_map(std::declval<Map &>(), std::declval<storage_type &>()));

private:
  [[no_unique_address]] storage_type      m_storage;
  [[no_unique_address]] property_map_type m_map;

public:
  constexpr
  result_storage()
  requires std::default_initializable<Map>
    : result_storage{Map{}}
  { }

  explicit constexpr
  result_storage(Map m)
    : m_storage{s_resolve_storage()}
    , m_map    {s_resolve_map(std::move(m), this->m_storage)}
  { }


  [[nodiscard]] constexpr
  property_map_type
  property_map() const
  { return this->m_map; }
};


template<typename K, typename V, typename M>
using deduced_storage_t
= result_storage<std::unordered_map<K, V>, M>;

} // namespace detail
} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_RESULT_STORAGE_HPP
