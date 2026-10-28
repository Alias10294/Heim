#ifndef HEIM_TEST_GRAPH_ALGORITHM_BREADTH_FIRST_MODEL_HPP
#define HEIM_TEST_GRAPH_ALGORITHM_BREADTH_FIRST_MODEL_HPP

#include "support.hpp"

namespace heim_test::graph_algorithm::bfs
{
struct reference_graph
{
  unsigned count{};
  bool directed{};
  std::vector<std::pair<unsigned, unsigned>> edges;

  explicit reference_graph(unsigned size, bool is_directed)
    : count{size}, directed{is_directed}
  { }

  void add(unsigned u, unsigned v)
  { edges.emplace_back(u, v); }

  bool adjacent(unsigned u, unsigned v) const
  {
    return std::ranges::any_of(edges, [&](auto const &e)
    {
      return (e.first == u && e.second == v)
          || (!directed && e.first == v && e.second == u);
    });
  }

  // Independent textbook FIFO BFS on a plain edge list.
  std::vector<int> distances(std::vector<unsigned> const &roots) const
  {
    std::vector<int> distance(count, -1);
    std::queue<unsigned> pending;

    for (unsigned root : roots)
    {
      REQUIRE(root < count);
      if (distance[root] >= 0)
        continue;

      distance[root] = 0;
      pending.push(root);
    }

    while (!pending.empty())
    {
      unsigned const u = pending.front();
      pending.pop();

      for (auto [a, b] : edges)
      {
        auto visit = [&](unsigned v)
        {
          if (distance[v] < 0)
          {
            distance[v] = distance[u] + 1;
            pending.push(v);
          }
        };

        if (a == u)
          visit(b);

        if (!directed && b == u)
          visit(a);
      }
    }

    return distance;
  }
};

template<typename G>
void verify_search(G const &g,
                   hg::bfs_result<G> const &result,
                   reference_graph const &model,
                   std::vector<unsigned> const &roots)
{
  auto const distance = model.distances(roots);
  std::size_t reached_count{};
  std::set<std::size_t> times;
  auto const invalid = std::numeric_limits<std::size_t>::max();

  for (unsigned id = 0; id < model.count; ++id)
  {
    INFO("vertex=" << id << " directed=" << model.directed);
    auto const v = at<G>(id);
    bool const reached = distance[id] >= 0;
    REQUIRE(result.reached(v) == reached);

    if (!reached)
    {
      REQUIRE(result.discovery_time_of(v) == invalid);
      continue;
    }

    ++reached_count;
    REQUIRE(runtime_level_of(result, v) == static_cast<std::size_t>(distance[id]));
    auto const time = result.discovery_time_of(v);
    REQUIRE(time != invalid);
    REQUIRE(time >= 1);
    REQUIRE(times.insert(time).second);

    auto const predecessor = runtime_predecessor_of(result, v);
    unsigned const parent = static_cast<unsigned>(predecessor.id());
    REQUIRE(parent < model.count);

    if (distance[id] == 0)
    {
      REQUIRE(parent == id);
      REQUIRE(std::ranges::find(roots, id) != roots.end());
    }
    else
    {
      REQUIRE(model.adjacent(parent, id));
      REQUIRE(distance[parent] == distance[id] - 1);
    }

    auto const path = path_ids<G>(runtime_path_from(result, v));
    REQUIRE(path.size() == static_cast<std::size_t>(distance[id]) + 1);
    REQUIRE(path.front() == id);
    REQUIRE(std::ranges::find(roots, path.back()) != roots.end());

    for (std::size_t j = 1; j < path.size(); ++j)
    {
      REQUIRE(model.adjacent(path[j], path[j - 1]));
      REQUIRE(distance[path[j]] == distance[path[j - 1]] - 1);
    }
  }

  REQUIRE(times.size() == reached_count);
  for (std::size_t t = 1; t <= reached_count; ++t)
    REQUIRE(times.contains(t));

  REQUIRE(g.vertex_count() == model.count);
}

template<typename G>
void verify_target(hg::bfs_to_result<G> const &result,
                   reference_graph const &model,
                   std::vector<unsigned> const &roots,
                   unsigned target)
{
  auto const distance = model.distances(roots);
  REQUIRE(result.reached() == (distance[target] >= 0));

  if (distance[target] < 0)
  {
    REQUIRE(result.discovery_time() == std::numeric_limits<std::size_t>::max());
    return;
  }

  REQUIRE(runtime_target_level(result) == static_cast<std::size_t>(distance[target]));
  REQUIRE(result.discovery_time() >= 1);
  REQUIRE(result.discovery_time() <= model.count);
  auto const path = path_ids<G>(runtime_target_path(result));
  REQUIRE(path.size() == static_cast<std::size_t>(distance[target]) + 1);
  REQUIRE(path.front() == target);
  REQUIRE(std::ranges::find(roots, path.back()) != roots.end());

  for (std::size_t i = 1; i < path.size(); ++i)
    REQUIRE(model.adjacent(path[i], path[i - 1]));
}

template<typename G>
void populate(G &g, reference_graph &model,
              std::vector<std::pair<unsigned, unsigned>> const &pairs)
{
  for (auto [u, v] : pairs)
  {
    (void)g.place_edge(at<G>(u), at<G>(v));
    model.add(u, v);
  }
}

} // namespace heim_test::graph_algorithm::bfs

#endif // HEIM_TEST_GRAPH_ALGORITHM_BREADTH_FIRST_MODEL_HPP
