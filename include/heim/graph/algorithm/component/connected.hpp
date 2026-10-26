#ifndef HEIM_GRAPH_ALGORITHM_COMPONENT_CONNECTED_HPP
#define HEIM_GRAPH_ALGORITHM_COMPONENT_CONNECTED_HPP

#include <cstddef>
#include <type_traits>
#include <utility>
#include "heim/graph/algorithm/depth_first/visit.hpp"
#include "heim/graph/algorithm/result_storage.hpp"
#include "heim/graph/interface/primitives.hpp"
#include "heim/graph/interface/traits.hpp"
#include "heim/property/primitives.hpp"
#include "heim/property/traits.hpp"

namespace heim::graphs
{
namespace detail::cc
{
namespace fn
{
struct algo;

} // namespace fn


template<
    typename G,
    typename IdMap>
using component_id_t
= std::remove_cvref_t<properties::readable_map_result_t<IdMap &, vertex_t<std::remove_cvref_t<G>>>>;

template<
    typename G,
    typename IdMap>
class dfs_visitor
{
private:
  IdMap                     m_idm;
  component_id_t<G, IdMap> &m_cnt;
  component_id_t<G, IdMap>  m_curr;

public:
  constexpr
  dfs_visitor(
      std::in_place_type_t<G>,
      IdMap                     idm,
      component_id_t<G, IdMap> &cnt)
    : m_idm {std::move(idm)}
    , m_cnt {cnt}
    , m_curr{}
  { }


  constexpr
  void
  start_vertex(G const &, vertex_t<std::remove_cvref_t<G>>)
  { this->m_curr = this->m_cnt; ++this->m_cnt; }

  constexpr
  void
  discover_vertex(G const &, vertex_t<std::remove_cvref_t<G>> v)
  { properties::set(this->m_idm, v, this->m_curr); }
};

} // namespace detail::cc


template<
    typename G,
    typename IdMap = default_property_map_t>
class connected_components_result
{
  friend struct detail::cc::fn::algo;

private:
  using vertex_type
  = vertex_t<std::remove_cvref_t<G>>;

  using component_id_type
  = detail::cc::component_id_t<
      G,
      typename detail::deduced_storage_t<
          vertex_type, std::size_t, IdMap>
          ::property_map_type>;

private:
  detail::deduced_storage_t<vertex_type, std::size_t, IdMap> m_id_rstorage;
  component_id_type                                          m_count;

private:
  [[nodiscard]] constexpr
  auto
  m_component_id_map() const
  { return this->m_id_rstorage.property_map(); }

  [[nodiscard]] constexpr
  component_id_type &
  m_component_count()
  { return this->m_count; }

public:
  constexpr
  connected_components_result(
      std::in_place_type_t<G>,
      IdMap idm = {})
    : m_id_rstorage{std::move(idm)}
    , m_count      {}
  { }


  [[nodiscard]] constexpr
  auto
  component_count() const
  { return this->m_count; }

  [[nodiscard]] constexpr
  auto
  component_id_of(vertex_type v) const
  { return properties::get(this->m_component_id_map(), std::move(v)); }


  [[nodiscard]] constexpr
  bool
  connected() const
  { return this->m_count < 2; }

  [[nodiscard]] constexpr
  bool
  connected(vertex_type u, vertex_type v) const
  {
    return properties::get(this->m_component_id_map(), std::move(u))
        == properties::get(this->m_component_id_map(), std::move(v));
  }
};

template<
    typename G,
    typename IdMap = default_property_map_t>
using cc_result
= connected_components_result<G, IdMap>;


namespace detail::cc::fn
{
struct algo
{
  template<
      typename G,
      typename IdMap = default_property_map_t,
      typename CMap  = default_property_map_t>
  constexpr
  void
  operator()(
      G             const &g,
      cc_result<G, IdMap> &res,
      CMap                 cm  = {}) const
  requires (!directed<std::remove_cvref_t<G>>)
  {
    depth_first_visit(
        g,
        vertices(g),
        dfs_visitor{
            std::in_place_type<G>,
            res.m_component_id_map(),
            res.m_component_count ()},
        std::move(cm));
  }

  template<
      typename G,
      typename IdMap = default_property_map_t,
      typename CMap  = default_property_map_t>
  constexpr
  cc_result<G, IdMap>
  operator()(
      G const &g,
      IdMap    idm = {},
      CMap     cm  = {}) const
  requires (!directed<std::remove_cvref_t<G>>)
  {
    cc_result res{std::in_place_type<G>, std::move(idm)};

    (*this)(g, res, std::move(cm));
    return res;
  }
};

} // namespace detail::cc::fn

inline constexpr detail::cc::fn::algo connected_components{};
inline constexpr detail::cc::fn::algo cc                  {};

} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_COMPONENT_CONNECTED_HPP
