#include <catch2/catch_test_macros.hpp>
#include <heim/graph.hpp>
#include "test_utils.hpp"
#include <array>
#include <limits>
#include <vector>

namespace hg = heim::graphs;

TEST_CASE("BFS computes minimum edge distances", "[graph][bfs]")
{
  heim::adjacency_list<> g{{0u, 1u}, {0u, 2u}, {1u, 3u}, {2u, 3u}, {3u, 4u}, {4u, 5u}};
  auto r = hg::bfs(g, 0u);
  REQUIRE(r.reached(0u));
  REQUIRE(r.reached(5u));
  REQUIRE(r.level_of(0u) == 0);
  REQUIRE(r.level_of(1u) == 1);
  REQUIRE(r.level_of(2u) == 1);
  REQUIRE(r.level_of(3u) == 2);
  REQUIRE(r.level_of(4u) == 3);
  REQUIRE(r.level_of(5u) == 4);
  REQUIRE(heim_test::is_reverse_path(g, r.path_from(5u), heim::adjacency_list<>::vertex_type{0u},
                                      heim::adjacency_list<>::vertex_type{5u}));
  REQUIRE(heim_test::collect(r.path_from(5u)).size() == 5);
}

TEST_CASE("BFS leaves inaccessible vertices unreached", "[graph][bfs]")
{
  heim::adjacency_list<> g{{0u, 1u}, {2u, 3u}};
  auto result = hg::breadth_first_search(g, 0u);
  REQUIRE(result.reached(0u));
  REQUIRE(result.reached(1u));
  REQUIRE_FALSE(result.reached(2u));
  REQUIRE_FALSE(result.reached(3u));
  REQUIRE(result.discovery_time_of(2u) == std::numeric_limits<std::size_t>::max());
}

TEST_CASE("BFS on isolated root creates a single-element path", "[graph][bfs]")
{
  heim::adjacency_list<> g;
  auto root = hg::place_vertex(g);
  auto r = hg::bfs(g, root);
  REQUIRE(r.reached(root));
  REQUIRE(r.level_of(root) == 0);
  REQUIRE(heim_test::collect(r.path_from(root)) == std::vector{root});
}

TEST_CASE("BFS to target returns shortest path and distance", "[graph][bfs_to]")
{
  heim::adjacency_list<> g{{0u, 1u}, {1u, 2u}, {0u, 3u}, {3u, 4u}, {4u, 2u}};
  auto r = hg::bfs_to(g, 0u, 2u);
  REQUIRE(r.reached());
  REQUIRE(r.level() == 2);
  REQUIRE(heim_test::collect(r.path()) == (std::vector<heim::adjacency_list<>::vertex_type>{2u, 1u, 0u}));
}

TEST_CASE("BFS to unreachable target does not report success", "[graph][bfs_to]")
{
  heim::adjacency_list<> g{{0u, 1u}, {2u, 3u}};
  auto r = hg::bfs_to(g, 0u, 3u);
  REQUIRE_FALSE(r.reached());
}

TEST_CASE("BFS range of sources measures distance to nearest source", "[graph][bfs]")
{
  heim::adjacency_list<> g{{0u, 1u}, {1u, 2u}, {2u, 3u}, {3u, 4u}};
  using vertex = heim::adjacency_list<>::vertex_type;
  std::array<vertex, 2> roots{0u, 4u};
  auto r = hg::bfs(g, roots);
  REQUIRE(r.level_of(0u) == 0);
  REQUIRE(r.level_of(4u) == 0);
  REQUIRE(r.level_of(1u) == 1);
  REQUIRE(r.level_of(3u) == 1);
  REQUIRE(r.level_of(2u) == 2);
}
