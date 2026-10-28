#ifndef HEIM_TEST_GRAPH_ALGORITHM_DIJKSTRA_REFERENCE_HPP
#define HEIM_TEST_GRAPH_ALGORITHM_DIJKSTRA_REFERENCE_HPP

#include "support.hpp"

namespace heim_test::graph_algorithm::dijkstra
{
struct reference_graph
{
  struct arc
  {
    unsigned from;
    unsigned to;
    unsigned weight;
  };

  unsigned count;
  bool directed;
  std::vector<arc> edges;

  explicit reference_graph(unsigned vertex_count, bool is_directed = true)
    : count{vertex_count}
    , directed{is_directed}
  { }

  void add(unsigned u, unsigned v, unsigned weight)
  { edges.push_back({u, v, weight}); }

  // Bellman-Ford over an edge list. This uses neither the Dijkstra heap
  // nor Heim graph primitives or property maps.
  std::vector<unsigned> distances(std::vector<unsigned> const &roots) const
  {
    constexpr unsigned infinity = std::numeric_limits<unsigned>::max();
    std::vector<unsigned> distance(count, infinity);

    for (unsigned r : roots)
    {
      REQUIRE(r < count);
      distance[r] = 0;
    }

    auto relax = [&](unsigned u, unsigned v, unsigned weight)
    {
      if (distance[u] == infinity)
        return false;

      auto const candidate = distance[u] + weight;
      if (candidate >= distance[v])
        return false;

      distance[v] = candidate;
      return true;
    };

    for (unsigned iteration = 1; iteration < count; ++iteration)
    {
      bool changed{};

      for (auto [u, v, weight] : edges)
      {
        changed |= relax(u, v, weight);
        if (!directed)
          changed |= relax(v, u, weight);
      }

      if (!changed)
        break;
    }

    return distance;
  }

  bool has_arc(unsigned u, unsigned v, unsigned weight) const
  {
    return std::ranges::any_of(edges, [&](arc const &a)
    {
      return a.weight == weight
          && ((a.from == u && a.to == v) || (!directed && a.from == v && a.to == u));
    });
  }
};

template<typename G>
struct weighted_graph
{
  G g;
  std::vector<unsigned> weights;
  reference_graph reference;

  explicit weighted_graph(unsigned count)
    : reference{count, G::is_directed}
  {
    make_vertices(g, count);
  }

  void add(unsigned u, unsigned v, unsigned weight)
  {
    auto [e, inserted] = g.place_edge(at<G>(u), at<G>(v));
    if (!inserted)
      return;

    auto const index = edge_index(e);
    if (index >= weights.size())
      weights.resize(index + 1);

    weights[index] = weight;
    reference.add(u, v, weight);
  }

  auto map() const
  { return weight_map<edge<G>>{&weights}; }
};

template<typename G, typename Result>
void verify_search(Result const &result,
                   reference_graph const &reference,
                   std::vector<unsigned> const &roots)
{
  auto const expected = reference.distances(roots);
  auto const infinity = std::numeric_limits<unsigned>::max();

  for (unsigned i = 0; i < reference.count; ++i)
  {
    INFO("vertex=" << i << " vertices=" << reference.count);

    auto const v = at<G>(i);
    REQUIRE(result.reached(v) == (expected[i] != infinity));
    REQUIRE(runtime_distance_of(result, v) == expected[i]);

    if (expected[i] == infinity)
      continue;

    auto const path = path_ids<G>(runtime_path_from(result, v));
    REQUIRE_FALSE(path.empty());
    REQUIRE(path.front() == i);
    REQUIRE(path.size() <= reference.count);
    REQUIRE(std::ranges::find(roots, path.back()) != roots.end());

    std::set<unsigned> seen;
    for (std::size_t j = 0; j < path.size(); ++j)
    {
      REQUIRE(path[j] < reference.count);
      REQUIRE(seen.insert(path[j]).second);

      auto const current = at<G>(path[j]);
      auto const parent = runtime_predecessor_of(result, current);
      auto const parent_id = static_cast<unsigned>(parent.id());
      REQUIRE(parent_id == path[std::min(j + 1, path.size() - 1)]);

      if (j + 1 == path.size())
      {
        REQUIRE(expected[path[j]] == 0);
        continue;
      }

      auto const u = path[j + 1];
      auto const w = path[j];
      REQUIRE(expected[u] <= expected[w]);
      REQUIRE(reference.has_arc(u, w, expected[w] - expected[u]));
    }
  }
}

template<typename G, typename Result>
void verify_target(Result const &result,
                   reference_graph const &reference,
                   std::vector<unsigned> const &roots,
                   unsigned target)
{
  auto const expected = reference.distances(roots);
  auto const infinity = std::numeric_limits<unsigned>::max();

  REQUIRE(result.reached() == (expected[target] != infinity));
  REQUIRE(runtime_target_distance(result) == expected[target]);

  if (expected[target] == infinity)
    return;

  auto const path = path_ids<G>(runtime_target_path(result));
  REQUIRE_FALSE(path.empty());
  REQUIRE(path.front() == target);
  REQUIRE(path.size() <= reference.count);
  REQUIRE(std::ranges::find(roots, path.back()) != roots.end());

  std::set<unsigned> seen;
  unsigned sum{};

  for (std::size_t i = 0; i < path.size(); ++i)
  {
    REQUIRE(seen.insert(path[i]).second);

    if (i + 1 == path.size())
      continue;

    // Parallel edges can share endpoints. With nonnegative costs, the
    // minimum edge weight between consecutive vertices is optimal.
    unsigned const child = path[i];
    unsigned const parent = path[i + 1];
    unsigned best = infinity;

    for (auto [u, v, weight] : reference.edges)
      if ((u == parent && v == child)
          || (!reference.directed && v == parent && u == child))
        best = std::min(best, weight);

    REQUIRE(best != infinity);
    sum += best;
  }

  REQUIRE(sum == expected[target]);
}

} // namespace heim_test::graph_algorithm::dijkstra

#endif // HEIM_TEST_GRAPH_ALGORITHM_DIJKSTRA_REFERENCE_HPP
