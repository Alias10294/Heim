#ifndef HEIM_GRAPH_ALGORITHM_COMMON_HPP
#define HEIM_GRAPH_ALGORITHM_COMMON_HPP

#include <utility>
#include "heim/property/property_map.hpp"

namespace heim::graphs
{
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
