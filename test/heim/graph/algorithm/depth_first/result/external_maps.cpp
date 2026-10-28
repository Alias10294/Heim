#include "../support.hpp"

using namespace heim_test::graph_algorithm::dfs;

TEST_CASE("DFS external maps remain accessible after search", "[graph][dfs][result][external]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}});
  std::unordered_map<V, V> parents;
  std::unordered_map<V, std::size_t> discovery;
  std::unordered_map<V, std::size_t> finish;
  using PM = external_map<V, V>;
  using TM = external_map<V, std::size_t>;
  hg::dfs_result<G, PM, TM, TM> result{
      std::in_place_type<G>, PM{&parents}, TM{&discovery}, TM{&finish}};
  hg::dfs(g, result, at<G>(0));
  REQUIRE(parents.at(at<G>(2)) == at<G>(1));
  REQUIRE(discovery.at(at<G>(2)) == 3);
  REQUIRE(finish.at(at<G>(0)) == 6);
  REQUIRE((path_ids<G>(runtime_path_from(result, at<G>(2))) ==
           std::vector<unsigned>{2, 1, 0}));
}

TEST_CASE("DFS_to result can use externally owned maps", "[graph][dfs][result][external]")
{
  using G = graph<>;
  using V = vertex<G>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  std::unordered_map<V, V> parents;
  std::unordered_map<V, std::size_t> discovery;
  using PM = external_map<V, V>;
  using DM = external_map<V, std::size_t>;
  hg::dfs_to_result<G, PM, DM> result{
      std::in_place_type<G>, at<G>(2), PM{&parents}, DM{&discovery}};
  hg::dfs_to(g, result, at<G>(0));
  REQUIRE(result.reached());
  REQUIRE(discovery.at(at<G>(2)) == 3);
  REQUIRE((path_ids<G>(runtime_target_path(result)) == std::vector<unsigned>{2, 1, 0}));
}

TEST_CASE("DFS results own independent default maps", "[graph][dfs][result][external]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {3, 2}});
  auto left = hg::dfs(g, at<G>(0));
  auto right = hg::dfs(g, at<G>(3));
  REQUIRE(left.reached(at<G>(1)));
  REQUIRE_FALSE(right.reached(at<G>(1)));
  REQUIRE(left.reached(at<G>(2)));
  REQUIRE(right.reached(at<G>(2)));
  REQUIRE((path_ids<G>(runtime_path_from(left, at<G>(2))) == std::vector<unsigned>{2, 1, 0}));
  REQUIRE((path_ids<G>(runtime_path_from(right, at<G>(2))) == std::vector<unsigned>{2, 3}));
}
