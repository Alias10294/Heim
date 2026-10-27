#include <catch2/catch_test_macros.hpp>
#include <heim/graph.hpp>
#include "test_utils.hpp"
#include <algorithm>
#include <cstddef>
#include <limits>
#include <queue>
#include <random>
#include <vector>

namespace hg = heim::graphs;

TEST_CASE("BFS shortest levels agree with a plain queue-based oracle", "[graph][bfs][model]")
{
  constexpr unsigned n = 35;
  constexpr unsigned inf = std::numeric_limits<unsigned>::max();
  heim::adjacency_list<true> graph;
  for (unsigned u = 0; u < n; ++u) hg::place_vertex(graph);
  std::vector<std::vector<unsigned>> arcs(n);
  std::mt19937 rng{0xBF517u};
  for (unsigned u = 0; u < n; ++u)
    for (unsigned v = 0; v < n; ++v)
      if (u != v && rng() % 6 == 0) {
        hg::place_edge(graph, u, v);
        arcs[u].push_back(v);
      }
  for (unsigned source : {0u, 8u, 19u}) {
    std::vector<unsigned> expected(n, inf);
    std::queue<unsigned> queue;
    expected[source] = 0;
    queue.push(source);
    while (!queue.empty()) {
      auto u = queue.front();
      queue.pop();
      for (auto v : arcs[u]) if (expected[v] == inf) {
        expected[v] = expected[u] + 1;
        queue.push(v);
      }
    }
    auto result = hg::bfs(graph, typename heim::adjacency_list<true>::vertex_type{source});
    for (unsigned v = 0; v < n; ++v) {
      INFO("source=" << source << " target=" << v);
      if (expected[v] == inf) REQUIRE_FALSE(result.reached(v));
      else {
        REQUIRE(result.reached(v));
        REQUIRE(result.level_of(v) == expected[v]);
        REQUIRE(heim_test::collect(result.path_from(v)).size() == expected[v] + 1);
        REQUIRE(heim_test::is_reverse_path(graph, result.path_from(v),
                  typename heim::adjacency_list<true>::vertex_type{source},
                  typename heim::adjacency_list<true>::vertex_type{v}));
      }
    }
  }
}
