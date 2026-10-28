#ifndef HEIM_TEST_GRAPH_ALGORITHM_BREADTH_FIRST_SUPPORT_HPP
#define HEIM_TEST_GRAPH_ALGORITHM_BREADTH_FIRST_SUPPORT_HPP

#include <heim/graph.hpp>
#include <heim/graph/algorithm/breadth_first/breadth_first.hpp>
#include <heim/property/property_map.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <queue>
#include <ranges>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace heim_test::graph_algorithm::bfs
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
std::vector<vertex<G>> make_vertices(G &g, unsigned count)
{
  std::vector<vertex<G>> result;
  result.reserve(count);

  for (unsigned i = 0; i < count; ++i)
    result.push_back(g.place_vertex());

  return result;
}

template<typename G>
vertex<G> at(unsigned id)
{ return vertex<G>{static_cast<typename G::id_type>(id)}; }

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

  for (auto v : range)
    result.push_back(static_cast<unsigned>(v.id()));

  return result;
}

template<typename G>
std::vector<unsigned> path_ids(std::ranges::input_range auto &&range)
{ return ids<G>(std::forward<decltype(range)>(range)); }

// Keep calls containing C++26 contract assertions in runtime-only wrappers.
// This also avoids a GCC 16 constant-evaluation regression seen in heap tests.
template<typename Result, typename Vertex>
auto runtime_level_of(Result const &result, Vertex v)
{ return result.level_of(v); }

template<typename Result, typename Vertex>
auto runtime_predecessor_of(Result const &result, Vertex v)
{ return result.predecessor_of(v); }

template<typename Result, typename Vertex>
auto runtime_path_from(Result const &result, Vertex v)
{ return result.path_from(v); }

template<typename Result>
auto runtime_target_level(Result const &result)
{ return result.level(); }

template<typename Result>
auto runtime_target_path(Result const &result)
{ return result.path(); }

struct random_generator
{
  std::uint32_t state;

  explicit random_generator(std::uint32_t seed) : state{seed ? seed : 1u} { }

  std::uint32_t next() noexcept
  {
    auto x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return state = x;
  }

  std::size_t bounded(std::size_t bound) noexcept
  { return static_cast<std::size_t>(next()) % bound; }
};

// This graph deliberately does not use any Heim graph storage. Its descriptor
// types are ordinary integers and its adjacency arrays have a stable order.
struct external_graph
{
  using vertex_type = int;
  using edge_type = int;
  static constexpr bool is_directed = true;
  static constexpr bool is_multigraph = true;

  struct endpoints_type
  {
    int source;
    int target;
  };

  std::vector<int> vertex_ids;
  std::vector<endpoints_type> edge_ends;
  std::vector<std::vector<int>> outgoing;

  explicit external_graph(unsigned count)
  {
    for (unsigned i = 0; i < count; ++i)
      vertex_ids.push_back(static_cast<int>(i));

    outgoing.resize(count);
  }

  int add_edge(int u, int v)
  {
    int const id = static_cast<int>(edge_ends.size());
    edge_ends.push_back({u, v});
    outgoing.at(static_cast<std::size_t>(u)).push_back(id);
    return id;
  }

  auto vertices() const { return std::span{vertex_ids}; }

  auto edges() const { return std::views::iota(0, static_cast<int>(edge_ends.size())); }

  auto out_edges(int v) const { return std::span{outgoing.at(static_cast<std::size_t>(v))}; }

  int source(int e) const { return edge_ends.at(static_cast<std::size_t>(e)).source; }

  int target(int e) const { return edge_ends.at(static_cast<std::size_t>(e)).target; }
};

inline external_graph make_external_graph()
{
  external_graph g{4};
  g.add_edge(0, 1);
  g.add_edge(0, 2);
  g.add_edge(1, 2);
  g.add_edge(2, 1);
  g.add_edge(2, 3);
  g.add_edge(3, 0);
  return g;
}

// A queue with the same operations as std::queue but observable via counters.
template<typename T>
class tracking_queue
{
public:
  struct statistics
  {
    unsigned pushes{};
    unsigned pops{};
    unsigned front_reads{};
    std::size_t maximum{};
  };

private:
  std::deque<T> m_values;
  statistics *m_stats;

public:
  explicit tracking_queue(statistics &stats) : m_stats{&stats} { }

  bool empty() const noexcept { return m_values.empty(); }

  void push(T value)
  {
    m_values.push_back(std::move(value));
    ++m_stats->pushes;
    m_stats->maximum = std::max(m_stats->maximum, m_values.size());
  }

  T const &front() const
  {
    ++m_stats->front_reads;
    return m_values.front();
  }

  void pop()
  {
    ++m_stats->pops;
    m_values.pop_front();
  }
};

template<typename K, typename V>
struct external_map
{
  std::unordered_map<K, V> *storage{};

  V &get(K key) const { return storage->at(key); }

  void set(K key, V value) const
  { storage->insert_or_assign(std::move(key), std::move(value)); }
};

enum class custom_color { white, black, gray };

} // namespace heim_test::graph_algorithm::bfs

#endif // HEIM_TEST_GRAPH_ALGORITHM_BREADTH_FIRST_SUPPORT_HPP
