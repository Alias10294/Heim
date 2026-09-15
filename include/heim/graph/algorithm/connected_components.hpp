#ifndef HEIM_GRAPH_ALGORITHM_CONNECTED_COMPONENTS_HPP
#define HEIM_GRAPH_ALGORITHM_CONNECTED_COMPONENTS_HPP

#include <cstddef>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include "heim/graph/interface/primitives.hpp"
#include "heim/graph/interface/traits.hpp"
#include "heim/property/primitives.hpp"
#include "heim/property/traits.hpp"
#include "color.hpp"
#include "common.hpp"
#include "depth_first_search.hpp"

namespace heim::graphs
{
namespace detail
{
template<
    typename ComponentMap,
    typename Component>
class connected_components_visitor
  : public basic_depth_first_search_visitor
{
private:
  ComponentMap m_comp_map;
  Component    m_comp_count;

public:
  constexpr
  connected_components_visitor()
  = default;

  constexpr
  connected_components_visitor(
      ComponentMap comp_map,
      Component    comp_count)
    : m_comp_map  {std::move(comp_map)}
    , m_comp_count{std::move(comp_count)}
  { }


  template<typename Graph>
  constexpr
  void
  discover_vertex(Graph const &, vertex_t<std::remove_cvref_t<Graph>> v)
  { properties::set(this->m_comp_map, v, this->m_comp_count); }
};


template<
    typename CompMap,
    typename Graph>
constexpr
auto
resolve_component_count()
{
  if constexpr (properties::readable_map_for<CompMap &, vertex_t<std::remove_cvref_t<Graph>> &>)
    return std::remove_cvref_t<properties::readable_map_result_t<CompMap &, vertex_t<std::remove_cvref_t<Graph>> &>>{};
  else
    return std::size_t{};
}

} // namespace detail


template<
    typename Graph,
    typename CompMap,
    typename CompStorage,
    typename Component>
class connected_components_result
{
private:
  [[no_unique_address]] CompStorage m_comp_storage;
  [[no_unique_address]] CompMap     m_comp_map;
  Component                         m_comp_count;

public:
  constexpr
  connected_components_result()
  = default;

  constexpr
  connected_components_result(
      std::in_place_type_t<Graph>,
      CompMap       comp_map,
      CompStorage &&comp_storage,
      Component     comp_count)
    : m_comp_storage{std::move(comp_storage)}
    , m_comp_map    {std::move(comp_map)}
    , m_comp_count  {std::move(comp_count)}
  { }


  [[nodiscard]] constexpr
  auto
  component_map() const
  { return this->m_comp_map; }

  [[nodiscard]] constexpr
  auto
  component_count() const
  { return this->m_comp_count; }


  constexpr
  auto
  component_of(vertex_t<std::remove_cvref_t<Graph>> v) const
  { return properties::get(this->m_comp_map, std::move(v)); }

  constexpr
  bool
  connected() const
  { return this->component_count() == Component{1}; }

  constexpr
  bool
  connected(
      vertex_t<std::remove_cvref_t<Graph>> u,
      vertex_t<std::remove_cvref_t<Graph>> v) const
  { return this->component_of(std::move(u)) == this->component_of(std::move(v)); }
};


namespace detail
{
struct connected_components_fn
{
  template<
      typename Graph,
      typename ComponentMap = default_property_map_t,
      typename ColorMap     = default_property_map_t>
  constexpr
  auto
  operator()(
      Graph const &g,
      ComponentMap comp = {},
      ColorMap     clr  = {}) const
  requires (!directed<std::remove_cvref_t<Graph>>)
  {
    using vertex_type
    = vertex_t<std::remove_cvref_t<Graph>>;

    auto comp_storage{resolve_storage<ComponentMap, std::unordered_map<vertex_type, std::size_t>>()};
    auto clr_storage {resolve_storage<ColorMap    , std::unordered_map<vertex_type, color>>()};

    auto comp_map{resolve_property_map(std::move(comp), comp_storage)};
    auto clr_map {resolve_property_map(std::move(clr ), clr_storage)};

    using color_type
    = std::remove_cvref_t<properties::readable_map_result_t<decltype(clr_map), vertex_type &>>;

    for (vertex_type v : vertices(g))
      properties::set(clr_map, v, colors::white_v<color_type>);

    auto comp_count{resolve_component_count<decltype(comp_map), Graph>()};

    for (vertex_type v : vertices(g))
    {
      if (properties::get(clr_map, v) != colors::white_v<color_type>)
        continue;

      depth_first_visit(
          g,
          v,
          connected_components_visitor{comp_map, comp_count},
          clr_map);
      ++comp_count;
    }

    return connected_components_result{
        std::in_place_type<Graph>,
        std::move(comp_map),
        std::move(comp_storage),
        std::move(comp_count)};
  }
};

} // namespace detail

inline constexpr detail::connected_components_fn connected_components{};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_CONNECTED_COMPONENTS_HPP
