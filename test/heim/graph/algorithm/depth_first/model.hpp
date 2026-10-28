#ifndef HEIM_TEST_GRAPH_ALGORITHM_DEPTH_FIRST_MODEL_HPP
#define HEIM_TEST_GRAPH_ALGORITHM_DEPTH_FIRST_MODEL_HPP

#include "support.hpp"

namespace heim_test::graph_algorithm::dfs
{
// Independent edge-list oracle. Reachability is deliberately computed without
// invoking Heim algorithms, traversals, property maps or descriptors.
struct reference_graph
{
  unsigned count{};
  bool directed{};
  std::vector<std::pair<unsigned, unsigned>> edges;

  explicit reference_graph(unsigned vertices, bool is_directed)
    : count{vertices}, directed{is_directed}
  { }

  void add(unsigned u, unsigned v)
  {
    REQUIRE(u < count);
    REQUIRE(v < count);
    edges.emplace_back(u, v);
  }

  bool adjacent(unsigned u, unsigned v) const
  {
    return std::ranges::any_of(edges, [&](auto const &edge)
    {
      return (edge.first == u && edge.second == v)
          || (!directed && edge.first == v && edge.second == u);
    });
  }

  std::vector<bool> reachable(std::vector<unsigned> const &roots) const
  {
    std::vector<bool> visited(count);
    std::vector<unsigned> stack;
    for (unsigned root : roots)
    {
      REQUIRE(root < count);
      stack.push_back(root);
    }

    while (!stack.empty())
    {
      unsigned const u = stack.back();
      stack.pop_back();
      if (visited[u])
        continue;

      visited[u] = true;
      for (auto [a, b] : edges)
      {
        if (a == u && !visited[b])
          stack.push_back(b);
        if (!directed && b == u && !visited[a])
          stack.push_back(a);
      }
    }
    return visited;
  }
};

// Verify properties that every *completed* DFS forest must satisfy, without
// depending on the order of the adjacency container or a particular DFS tree.
template<typename G>
void verify_search(G const &g,
                   hg::dfs_result<G> const &result,
                   reference_graph const &model,
                   std::vector<unsigned> const &roots)
{
  REQUIRE(g.vertex_count() == model.count);
  auto const expected = model.reachable(roots);
  auto const invalid = std::numeric_limits<std::size_t>::max();
  std::set<std::size_t> timestamps;
  std::size_t discovered{};

  for (unsigned id = 0; id < model.count; ++id)
  {
    INFO("vertex=" << id << " directed=" << model.directed);
    auto const v = at<G>(id);
    REQUIRE(result.reached(v) == expected[id]);

    std::size_t const d = result.discovery_time_of(v);
    std::size_t const f = result.finish_time_of(v);
    if (!expected[id])
    {
      REQUIRE(d == invalid);
      REQUIRE(f == invalid);
      continue;
    }

    ++discovered;
    REQUIRE(d > 0);
    REQUIRE(d < f);
    REQUIRE(f != invalid);
    REQUIRE(timestamps.insert(d).second);
    REQUIRE(timestamps.insert(f).second);

    auto const predecessor = runtime_predecessor_of(result, v);
    unsigned const parent = static_cast<unsigned>(predecessor.id());
    REQUIRE(parent < model.count);
    if (parent != id)
    {
      REQUIRE(model.adjacent(parent, id));
      REQUIRE(result.reached(at<G>(parent)));
      REQUIRE(result.discovery_time_of(at<G>(parent)) < d);
      REQUIRE(result.finish_time_of(at<G>(parent)) > f);
    }
    else
    {
      REQUIRE(std::ranges::find(roots, id) != roots.end());
    }

    auto const path = path_ids<G>(runtime_path_from(result, v));
    REQUIRE_FALSE(path.empty());
    REQUIRE(path.front() == id);
    REQUIRE(std::ranges::find(roots, path.back()) != roots.end());
    REQUIRE(path.size() <= model.count);
    REQUIRE(std::set<unsigned>(path.begin(), path.end()).size() == path.size());
    for (std::size_t i = 1; i < path.size(); ++i)
    {
      REQUIRE(model.adjacent(path[i], path[i - 1]));
      REQUIRE(result.discovery_time_of(at<G>(path[i])) <
              result.discovery_time_of(at<G>(path[i - 1])));
      REQUIRE(result.finish_time_of(at<G>(path[i])) >
              result.finish_time_of(at<G>(path[i - 1])));
    }
  }

  REQUIRE(timestamps.size() == 2 * discovered);
  for (std::size_t t = 1; t <= 2 * discovered; ++t)
    REQUIRE(timestamps.contains(t));

  // DFS intervals for two visited vertices are nested or disjoint.
  for (unsigned u = 0; u < model.count; ++u)
    for (unsigned v = u + 1; v < model.count; ++v)
    {
      if (!expected[u] || !expected[v])
        continue;
      auto const du = result.discovery_time_of(at<G>(u));
      auto const fu = result.finish_time_of(at<G>(u));
      auto const dv = result.discovery_time_of(at<G>(v));
      auto const fv = result.finish_time_of(at<G>(v));
      REQUIRE((fu < dv || fv < du || (du < dv && fv < fu) || (dv < du && fu < fv)));
    }
}

template<typename G>
void verify_target(hg::dfs_to_result<G> const &result,
                   reference_graph const &model,
                   std::vector<unsigned> const &roots,
                   unsigned target)
{
  auto const expected = model.reachable(roots);
  REQUIRE(result.reached() == expected.at(target));
  if (!expected[target])
  {
    REQUIRE(result.discovery_time() == std::numeric_limits<std::size_t>::max());
    return;
  }

  REQUIRE(result.discovery_time() >= 1);
  REQUIRE(result.discovery_time() <= model.count);
  auto const path = path_ids<G>(runtime_target_path(result));
  REQUIRE_FALSE(path.empty());
  REQUIRE(path.front() == target);
  REQUIRE(std::ranges::find(roots, path.back()) != roots.end());
  REQUIRE(path.size() <= model.count);
  REQUIRE(std::set<unsigned>(path.begin(), path.end()).size() == path.size());
  for (std::size_t i = 1; i < path.size(); ++i)
    REQUIRE(model.adjacent(path[i], path[i - 1]));
}

template<typename G>
void check_model(G const &g,
                 reference_graph const &model,
                 std::vector<unsigned> const &roots)
{
  auto const root_descriptors = [&]
  {
    std::vector<vertex<G>> result;
    for (unsigned r : roots)
      result.push_back(at<G>(r));
    return result;
  }();
  auto const result = hg::dfs(g, root_descriptors);
  verify_search(g, result, model, roots);
}

} // namespace heim_test::graph_algorithm::dfs

#endif // HEIM_TEST_GRAPH_ALGORITHM_DEPTH_FIRST_MODEL_HPP
