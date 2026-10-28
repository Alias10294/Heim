#ifndef HEIM_TEST_GRAPH_ALGORITHM_DERIVED_SUPPORT_HPP
#define HEIM_TEST_GRAPH_ALGORITHM_DERIVED_SUPPORT_HPP

#include <heim/graph.hpp>
#include <heim/graph/algorithm/component/connected.hpp>
#include <heim/graph/algorithm/topological_sort.hpp>
#include <heim/property/property_map.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <concepts>
#include <deque>
#include <forward_list>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <ranges>
#include <set>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace heim_test::graph_algorithm::derived
{
namespace hg = heim::graphs;
namespace hp = heim::properties;

template<bool Directed = true, bool Multi = false,
         bool In = true, bool Out = true, std::size_t Page = 8>
using graph = heim::adjacency_list<Directed, Multi, In, Out, unsigned, Page>;

template<typename G>
using vertex = typename G::vertex_type;

template<typename G>
vertex<G> at(unsigned id)
{ return vertex<G>{static_cast<typename G::id_type>(id)}; }

template<typename G>
void make_vertices(G &g, unsigned count)
{
  for (unsigned i = 0; i < count; ++i)
    (void)g.place_vertex();
}

template<typename G>
void add_edges(G &g, std::initializer_list<std::pair<unsigned, unsigned>> edges)
{
  for (auto [u, v] : edges)
    (void)g.place_edge(at<G>(u), at<G>(v));
}

template<typename G>
std::vector<unsigned> ids(std::ranges::input_range auto &&range)
{
  std::vector<unsigned> result;

  for (auto value : range)
    result.push_back(static_cast<unsigned>(value.id()));

  return result;
}

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

// The independent graph deliberately uses integer vertex descriptors and
// stores oriented incidences without depending on Heim graph storage.
template<bool Directed>
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
  std::vector<std::pair<int, int>> edge_ends;

  explicit external_graph(int count)
  {
    for (int v = 0; v < count; ++v)
      vertex_ids.push_back(v);

    outgoing.resize(static_cast<std::size_t>(count));
  }

  void add_edge(int from, int to)
  {
    int const index = static_cast<int>(edge_ends.size());
    edge_ends.emplace_back(from, to);
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

template<typename K, typename V>
struct external_map
{
  std::unordered_map<K, V> *storage{};

  V &get(K key) const
  { return storage->at(key); }

  void set(K key, V value) const
  { storage->insert_or_assign(std::move(key), std::move(value)); }
};

enum class custom_color
{
  white,
  black,
  gray
};

// Keep calls with C++26 contract assertions behind non-constexpr boundaries,
// avoiding GCC 16 speculative constant evaluation of contracts.
template<typename G>
auto runtime_topological_sort(G const &g)
{ return hg::topological_sort(g); }

template<typename G, typename Cont>
auto runtime_topological_sort(G const &g, Cont &&container)
{ return hg::topological_sort(g, std::forward<Cont>(container)); }

template<typename G, typename Cont, typename CMap>
auto runtime_topological_sort(G const &g, Cont &&container, CMap map)
{ return hg::topological_sort(g, std::forward<Cont>(container), std::move(map)); }

template<typename G, typename OutputIt>
auto runtime_reverse_topological_sort(G const &g, OutputIt it)
{ return hg::reverse_topological_sort(g, std::move(it)); }

template<typename G, typename OutputIt, typename CMap>
auto runtime_reverse_topological_sort(G const &g, OutputIt it, CMap map)
{ return hg::reverse_topological_sort(g, std::move(it), std::move(map)); }

} // namespace heim_test::graph_algorithm::derived

#endif // HEIM_TEST_GRAPH_ALGORITHM_DERIVED_SUPPORT_HPP
