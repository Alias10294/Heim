#ifndef HEIM_TEST_GRAPH_ALGORITHM_DIJKSTRA_SUPPORT_HPP
#define HEIM_TEST_GRAPH_ALGORITHM_DIJKSTRA_SUPPORT_HPP

#include <heim/graph.hpp>
#include <heim/graph/algorithm/dijkstra/dijkstra.hpp>
#include <heim/property/property_map.hpp>

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <ranges>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace heim_test::graph_algorithm::dijkstra
{
namespace hg = heim::graphs;
namespace hp = heim::properties;

template<bool Directed = true, bool Multi = false,
         bool In = true, bool Out = true, std::size_t Page = 8>
using graph = heim::adjacency_list<Directed, Multi, In, Out, unsigned, Page>;

template<typename G>
using vertex = typename G::vertex_type;

template<typename G>
using edge = typename G::edge_type;

template<typename G>
vertex<G> at(unsigned id)
{ return vertex<G>{static_cast<typename G::id_type>(id)}; }

template<typename G>
void make_vertices(G &g, unsigned count)
{
  for (unsigned i = 0; i < count; ++i)
    (void)g.place_vertex();
}

template<typename E>
std::size_t edge_index(E const &e)
{
  if constexpr (requires { e.id(); })
    return static_cast<std::size_t>(e.id());
  else if constexpr (requires { e.index; })
    return static_cast<std::size_t>(e.index);
  else
    return static_cast<std::size_t>(e);
}

// Access weights by edge identity, including when undirected incidences
// reverse the source and target stored in the edge descriptor.
template<typename E, typename W = unsigned>
struct weight_map
{
  std::vector<W> const *weights{};

  W get(E e) const
  { return weights->at(edge_index(e)); }
};

template<typename K, typename V>
struct external_map
{
  std::unordered_map<K, V> *storage{};

  V &get(K key) const
  { return storage->at(key); }

  void set(K key, V value) const
  { storage->insert_or_assign(std::move(key), std::move(value)); }
};

struct random_generator
{
  std::uint32_t state;

  explicit random_generator(std::uint32_t seed)
    : state{seed ? seed : 1u}
  { }

  std::uint32_t next() noexcept
  {
    auto x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return state = x;
  }

  unsigned bounded(unsigned limit) noexcept
  { return next() % limit; }
};

enum class custom_color
{
  white,
  black,
  gray
};

// Entirely independent adjacency representation and descriptor types.
// It has no Heim adjacency_list storage or edge ID conventions.
template<bool Directed = true>
struct external_graph
{
  using vertex_type = int;

  struct edge_type
  {
    int index;
    int from;
    int to;
  };

  static constexpr bool is_directed = Directed;
  static constexpr bool is_multigraph = true;

  std::vector<int> vertex_ids;
  std::vector<std::vector<edge_type>> outgoing;
  std::vector<std::pair<int, int>> endpoints;

  explicit external_graph(int count)
  {
    for (int v = 0; v < count; ++v)
      vertex_ids.push_back(v);

    outgoing.resize(static_cast<std::size_t>(count));
  }

  void add_edge(int from, int to)
  {
    int const index = static_cast<int>(endpoints.size());
    endpoints.emplace_back(from, to);
    outgoing.at(static_cast<std::size_t>(from)).push_back({index, from, to});

    if constexpr (!Directed)
      outgoing.at(static_cast<std::size_t>(to)).push_back({index, to, from});
  }

  auto vertices() const
  { return std::span{vertex_ids}; }

  auto out_edges(int v) const
  { return std::span{outgoing.at(static_cast<std::size_t>(v))}; }

  int source(edge_type e) const
  { return e.from; }

  int target(edge_type e) const
  { return e.to; }
};

// Keep visit calls out of Catch2's speculative constant-evaluation path.
// The algorithm remains constexpr; this boundary is test-only.
template<typename... Args>
bool runtime_visit(Args &&...args)
{ return hg::dijkstra_visit(std::forward<Args>(args)...); }

// GCC 16 can speculatively constant-evaluate constexpr contract assertions.
// Keep calls with contracts across non-constexpr function boundaries.
template<typename Result, typename V>
auto runtime_predecessor_of(Result const &result, V v)
{ return result.predecessor_of(v); }

template<typename Result, typename V>
auto runtime_path_from(Result const &result, V v)
{ return result.path_from(v); }

template<typename Result>
auto runtime_target_path(Result const &result)
{ return result.path(); }

template<typename Result, typename V>
auto runtime_distance_of(Result const &result, V v)
{ return result.distance_of(v); }

template<typename Result>
auto runtime_target_distance(Result const &result)
{ return result.distance(); }

template<typename G>
std::vector<unsigned> ids(std::ranges::input_range auto &&range)
{
  std::vector<unsigned> result;

  for (auto v : range)
    result.push_back(static_cast<unsigned>(v.id()));

  return result;
}

template<typename G>
std::vector<unsigned> path_ids(std::ranges::input_range auto &&range)
{ return ids<G>(std::forward<decltype(range)>(range)); }

} // namespace heim_test::graph_algorithm::dijkstra

#endif // HEIM_TEST_GRAPH_ALGORITHM_DIJKSTRA_SUPPORT_HPP
