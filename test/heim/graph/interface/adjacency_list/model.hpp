#ifndef HEIM_TEST_GRAPH_ADJACENCY_MODEL_HPP
#define HEIM_TEST_GRAPH_ADJACENCY_MODEL_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <map>
#include <ranges>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace heim_test::adjacency
{
// Deliberately independent of Heim containers and of adjacency-list storage.
// Stable identifiers and edge insertion orientation are modeled explicitly.
struct reference_graph
{
  struct edge
  {
    unsigned source, target;
  };
  std::set<unsigned> vertices;
  std::map<unsigned, edge> edges;
  unsigned next_vertex{};
  unsigned next_edge{};

  bool equivalent(unsigned u, unsigned v, edge const &e, bool directed) const
  {
    return (e.source == u && e.target == v) || (!directed && e.source == v && e.target == u);
  }

  bool contains_pair(unsigned u, unsigned v, bool directed) const
  {
    return std::ranges::any_of(edges,
                               [&](auto const &entry) { return equivalent(u, v, entry.second, directed); });
  }

  void add_vertex(unsigned id)
  {
    REQUIRE(id == next_vertex);
    REQUIRE(vertices.insert(id).second);
    ++next_vertex;
  }
  void add_edge(unsigned id, unsigned u, unsigned v)
  {
    REQUIRE(id == next_edge);
    REQUIRE(vertices.contains(u));
    REQUIRE(vertices.contains(v));
    REQUIRE(edges.emplace(id, edge{u, v}).second);
    ++next_edge;
  }
  void remove_edge(unsigned id) { REQUIRE(edges.erase(id) == 1); }
  void remove_edges(unsigned u, unsigned v, bool directed)
  {
    for (auto it = edges.begin(); it != edges.end();)
      if (equivalent(u, v, it->second, directed))
        it = edges.erase(it);
      else
        ++it;
  }
  void clear_vertex(unsigned v)
  {
    for (auto it = edges.begin(); it != edges.end();)
      if (it->second.source == v || it->second.target == v)
        it = edges.erase(it);
      else
        ++it;
  }
  void remove_vertex(unsigned v)
  {
    REQUIRE(vertices.erase(v) == 1);
    clear_vertex(v);
  }
  void clear()
  {
    vertices.clear();
    edges.clear();
  }
};

// Each incidence is (edge id, incident-oriented source, incident-oriented target).
using incidence = std::tuple<unsigned, unsigned, unsigned>;

template <typename G>
void verify(G const &g, reference_graph const &model)
{
  using vertex = typename G::vertex_type;
  REQUIRE(g.vertex_count() == model.vertices.size());
  REQUIRE(g.edge_count() == model.edges.size());
  std::set<unsigned> actual_vertices;
  for (auto v : g.vertices())
  {
    auto id = static_cast<unsigned>(v.id());
    INFO("vertex id=" << id);
    REQUIRE(actual_vertices.insert(id).second);
    REQUIRE(model.vertices.contains(id));
  }
  REQUIRE(actual_vertices == model.vertices);

  std::map<unsigned, reference_graph::edge> actual_edges;
  for (auto e : g.edges())
  {
    auto id = static_cast<unsigned>(e.id());
    INFO("edge id=" << id);
    REQUIRE(model.edges.contains(id));
    auto [u, v] = g.endpoints(e);
    unsigned src = static_cast<unsigned>(u.id());
    unsigned dst = static_cast<unsigned>(v.id());
    REQUIRE(src == model.edges.at(id).source);
    REQUIRE(dst == model.edges.at(id).target);
    REQUIRE(static_cast<unsigned>(g.source(e).id()) == src);
    REQUIRE(static_cast<unsigned>(g.target(e).id()) == dst);
    REQUIRE(actual_edges.emplace(id, reference_graph::edge{src, dst}).second);
    REQUIRE(model.vertices.contains(src));
    REQUIRE(model.vertices.contains(dst));
  }
  REQUIRE(actual_edges.size() == model.edges.size());

  for (unsigned v_id : model.vertices)
  {
    INFO("incidences for vertex=" << v_id);
    vertex v{static_cast<typename G::id_type>(v_id)};
    std::multiset<incidence> expected_out, expected_in;
    for (auto const &[id, e] : model.edges)
    {
      if (e.source == v_id)
        expected_out.emplace(id, v_id, e.target);
      if (e.target == v_id)
        expected_in.emplace(id, e.source, v_id);
      if constexpr (!G::is_directed)
      {
        if (e.target == v_id)
          expected_out.emplace(id, v_id, e.source);
        if (e.source == v_id)
          expected_in.emplace(id, e.target, v_id);
      }
    }
    if constexpr (G::is_out_incident)
    {
      std::multiset<incidence> actual;
      for (auto e : g.out_edges(v))
      {
        actual.emplace(static_cast<unsigned>(e.id()),
                       static_cast<unsigned>(g.source(e).id()),
                       static_cast<unsigned>(g.target(e).id()));
        REQUIRE(model.edges.contains(static_cast<unsigned>(e.id())));
      }
      REQUIRE(actual == expected_out);
      REQUIRE(g.out_edge_count(v) == expected_out.size());
    }
    if constexpr (G::is_in_incident)
    {
      std::multiset<incidence> actual;
      for (auto e : g.in_edges(v))
      {
        actual.emplace(static_cast<unsigned>(e.id()),
                       static_cast<unsigned>(g.source(e).id()),
                       static_cast<unsigned>(g.target(e).id()));
        REQUIRE(model.edges.contains(static_cast<unsigned>(e.id())));
      }
      REQUIRE(actual == expected_in);
      REQUIRE(g.in_edge_count(v) == expected_in.size());
    }
    if constexpr (G::is_in_incident && G::is_out_incident)
    {
      std::size_t degree = G::is_directed ? expected_in.size() + expected_out.size() : expected_out.size();
      REQUIRE(g.total_edge_count(v) == degree);
    }
  }
}

template <typename G>
typename G::vertex_type add_vertex(G &g, reference_graph &model)
{
  auto v = g.place_vertex();
  model.add_vertex(static_cast<unsigned>(v.id()));
  return v;
}

template <typename G>
std::pair<typename G::edge_type, bool>
add_edge(G &g, reference_graph &model, typename G::vertex_type u, typename G::vertex_type v)
{
  auto src = static_cast<unsigned>(u.id()), dst = static_cast<unsigned>(v.id());
  bool exists = model.contains_pair(src, dst, G::is_directed);
  auto result = g.place_edge(u, v);
  REQUIRE(result.second == (G::is_multigraph || !exists));
  if (result.second)
    model.add_edge(static_cast<unsigned>(result.first.id()), src, dst);
  return result;
}

struct rng
{
  std::uint32_t state;
  explicit rng(std::uint32_t seed) : state(seed ? seed : 1) {}
  std::uint32_t next() noexcept
  {
    auto n = state;
    n ^= n << 13;
    n ^= n >> 17;
    n ^= n << 5;
    return state = n;
  }
  std::size_t bounded(std::size_t upper) noexcept { return static_cast<std::size_t>(next()) % upper; }
};

inline unsigned select(std::set<unsigned> const &ids, std::size_t index)
{
  auto it = ids.begin();
  std::advance(it, static_cast<std::ptrdiff_t>(index));
  return *it;
}
inline unsigned select(std::map<unsigned, reference_graph::edge> const &ids, std::size_t index)
{
  auto it = ids.begin();
  std::advance(it, static_cast<std::ptrdiff_t>(index));
  return it->first;
}
} // namespace heim_test::adjacency

#endif // HEIM_TEST_GRAPH_ADJACENCY_MODEL_HPP
