#include <catch2/catch_test_macros.hpp>
#include <heim/graph.hpp>
#include "test_utils.hpp"
#include <limits>
#include <unordered_set>

namespace hg = heim::graphs;

TEST_CASE("DFS reaches all vertices of a connected component", "[graph][dfs]")
{
  heim::adjacency_list<> g{{0u, 1u}, {0u, 2u}, {1u, 3u}, {2u, 4u}};
  auto r = hg::dfs(g, 0u);
  for (unsigned i = 0; i < 5; ++i) REQUIRE(r.reached(i));
  REQUIRE(heim_test::is_reverse_path(g, r.path_from(4u), heim::adjacency_list<>::vertex_type{0u},
                                      heim::adjacency_list<>::vertex_type{4u}));
}

TEST_CASE("DFS leaves disconnected vertices unreached", "[graph][dfs]")
{
  heim::adjacency_list<> g{{0u, 1u}, {1u, 2u}, {3u, 4u}};
  auto r = hg::dfs(g, 0u);
  REQUIRE(r.reached(2u));
  REQUIRE_FALSE(r.reached(3u));
  REQUIRE_FALSE(r.reached(4u));
  REQUIRE(r.discovery_time_of(4u) == std::numeric_limits<std::size_t>::max());
}

TEST_CASE("DFS discovery and finish times enclose descendants", "[graph][dfs]")
{
  heim::adjacency_list<true> g{{0u, 1u}, {1u, 2u}, {2u, 3u}};
  auto r = hg::dfs(g, 0u);
  for (unsigned i = 0; i < 4; ++i) {
    REQUIRE(r.reached(i));
    REQUIRE(r.discovery_time_of(i) < r.finish_time_of(i));
  }
  for (unsigned i = 0; i < 3; ++i) {
    REQUIRE(r.discovery_time_of(i) < r.discovery_time_of(i + 1));
    REQUIRE(r.finish_time_of(i) > r.finish_time_of(i + 1));
  }
}

TEST_CASE("DFS to target returns a valid root-to-target chain", "[graph][dfs_to]")
{
  heim::adjacency_list<> g{{0u, 1u}, {1u, 2u}, {2u, 3u}, {1u, 4u}};
  auto r = hg::dfs_to(g, 0u, 3u);
  REQUIRE(r.reached());
  REQUIRE(heim_test::is_reverse_path(g, r.path(), heim::adjacency_list<>::vertex_type{0u},
                                      heim::adjacency_list<>::vertex_type{3u}));
}

TEST_CASE("DFS to target handles unreachable and root-target cases", "[graph][dfs_to]")
{
  heim::adjacency_list<> g{{0u, 1u}, {2u, 3u}};
  auto unreachable = hg::dfs_to(g, 0u, 3u);
  REQUIRE_FALSE(unreachable.reached());
  auto self = hg::dfs_to(g, 0u, 0u);
  REQUIRE(self.reached());
  REQUIRE(heim_test::collect(self.path()) == (std::vector<heim::adjacency_list<>::vertex_type>{0u}));
}
