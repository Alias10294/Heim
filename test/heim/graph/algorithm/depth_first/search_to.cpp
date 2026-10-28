#include "model.hpp"

using namespace heim_test::graph_algorithm::dfs;

TEST_CASE("DFS_to finds a target on a simple chain", "[graph][dfs][search_to]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}, {3, 4}});
  auto result = hg::dfs_to(g, at<G>(0), at<G>(3));
  REQUIRE(result.reached());
  REQUIRE(result.discovery_time() == 4);
  REQUIRE((path_ids<G>(runtime_target_path(result)) == std::vector<unsigned>{3, 2, 1, 0}));
}

TEST_CASE("DFS_to stops on a target identical to the root", "[graph][dfs][search_to]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  auto result = hg::dfs_to(g, at<G>(0), at<G>(0));
  REQUIRE(result.reached());
  REQUIRE(result.discovery_time() == 1);
  REQUIRE((path_ids<G>(runtime_target_path(result)) == std::vector<unsigned>{0}));
}

TEST_CASE("DFS_to reports an unreachable target", "[graph][dfs][search_to]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {2, 3}});
  auto result = hg::dfs_to(g, at<G>(0), at<G>(4));
  REQUIRE_FALSE(result.reached());
  REQUIRE(result.discovery_time() == std::numeric_limits<std::size_t>::max());
}

TEST_CASE("DFS_to can reach a target through a different root", "[graph][dfs][search_to]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 6);
  add_edges(g, {{0, 1}, {1, 2}, {3, 4}, {4, 5}});
  auto roots = std::array{at<G>(0), at<G>(3)};
  auto result = hg::dfs_to(g, roots, at<G>(5));
  REQUIRE(result.reached());
  REQUIRE((path_ids<G>(runtime_target_path(result)) == std::vector<unsigned>{5, 4, 3}));
}

TEST_CASE("DFS_to returns unreachable for an empty root range", "[graph][dfs][search_to]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 2);
  std::vector<vertex<G>> roots;
  auto result = hg::dfs_to(g, roots, at<G>(1));
  REQUIRE_FALSE(result.reached());
}

TEST_CASE("DFS_to handles a target on a directed cycle", "[graph][dfs][search_to]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 0}, {2, 3}});
  auto result = hg::dfs_to(g, at<G>(0), at<G>(3));
  REQUIRE(result.reached());
  REQUIRE((path_ids<G>(runtime_target_path(result)) == std::vector<unsigned>{3, 2, 1, 0}));
}

TEST_CASE("DFS_to handles a target in an undirected multigraph", "[graph][dfs][search_to]")
{
  using G = graph<false, true>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {0, 1}, {1, 2}});
  auto result = hg::dfs_to(g, at<G>(0), at<G>(2));
  REQUIRE(result.reached());
  REQUIRE((path_ids<G>(runtime_target_path(result)) == std::vector<unsigned>{2, 1, 0}));
}

TEST_CASE("DFS_to fills an explicitly supplied result once", "[graph][dfs][search_to]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}});
  hg::dfs_to_result<G> result{std::in_place_type<G>, at<G>(2)};
  hg::depth_first_search_to(g, result, at<G>(0));
  REQUIRE(result.reached());
  REQUIRE((path_ids<G>(runtime_target_path(result)) == std::vector<unsigned>{2, 1, 0}));
}

TEST_CASE("DFS_to with provided result accepts a root range", "[graph][dfs][search_to]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {2, 3}});
  hg::dfs_to_result<G> result{std::in_place_type<G>, at<G>(3)};
  std::array const roots{at<G>(0), at<G>(2)};
  hg::dfs_to(g, result, roots);
  REQUIRE(result.reached());
  REQUIRE((path_ids<G>(runtime_target_path(result)) == std::vector<unsigned>{3, 2}));
}

TEST_CASE("DFS_to result aliases expose the same type", "[graph][dfs][search_to]")
{
  using G = graph<>;
  static_assert(std::same_as<hg::dfs_to_result<G>, hg::depth_first_search_to_result<G>>);
}
