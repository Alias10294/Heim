#include "../support.hpp"

using namespace heim_test::graph_algorithm::dfs;

TEST_CASE("DFS path_from traverses predecessor chain back to its root", "[graph][dfs][result][path]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}, {3, 4}});
  auto result = hg::dfs(g, at<G>(0));
  REQUIRE((path_ids<G>(runtime_path_from(result, at<G>(4))) ==
           std::vector<unsigned>{4, 3, 2, 1, 0}));
}

TEST_CASE("DFS path_from a root yields that vertex exactly once", "[graph][dfs][result][path]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 2);
  auto result = hg::dfs(g, at<G>(1));
  REQUIRE((path_ids<G>(runtime_path_from(result, at<G>(1))) == std::vector<unsigned>{1}));
}

TEST_CASE("DFS path_from remains valid after the graph is modified", "[graph][dfs][result][path]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}});
  auto result = hg::dfs(g, at<G>(0));
  (void)g.place_edge(at<G>(0), at<G>(3));
  REQUIRE((path_ids<G>(runtime_path_from(result, at<G>(3))) ==
           std::vector<unsigned>{3, 2, 1, 0}));
}

TEST_CASE("DFS_to path from a root to itself has one element", "[graph][dfs][result][path]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  auto result = hg::dfs_to(g, at<G>(2), at<G>(2));
  REQUIRE((path_ids<G>(runtime_target_path(result)) == std::vector<unsigned>{2}));
}

TEST_CASE("DFS_to path to a neighbor has two elements", "[graph][dfs][result][path]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  auto result = hg::dfs_to(g, at<G>(0), at<G>(1));
  REQUIRE((path_ids<G>(runtime_target_path(result)) == std::vector<unsigned>{1, 0}));
}

TEST_CASE("DFS predecessor paths model input_range views", "[graph][dfs][result][path]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  auto result = hg::dfs(g, at<G>(0));
  auto path = runtime_path_from(result, at<G>(2));
  static_assert(std::ranges::view<decltype(path)>);
  static_assert(std::ranges::input_range<decltype(path)>);
  REQUIRE(std::ranges::distance(path) == 3);
}
