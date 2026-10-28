#include "support.hpp"

using namespace heim_test::graph_algorithm::bfs;

TEST_CASE("BFS search and target search handle erased vertices and edge ids", "[graph][bfs][regression]")
{
  using G = graph<true, true>;
  G g;
  make_vertices(g, 9);
  add_edges(g, {{0, 1}, {1, 2}, {0, 3}, {3, 4}, {4, 6}, {6, 8}, {0, 7}, {7, 8}});
  g.remove_vertex(at<G>(1));
  g.remove_vertex(at<G>(5));
  g.remove_edges(at<G>(0), at<G>(3));
  add_edges(g, {{0, 4}});

  auto result = hg::bfs(g, at<G>(0));
  REQUIRE(result.reached(at<G>(0)));
  REQUIRE(result.reached(at<G>(4)));
  REQUIRE(result.reached(at<G>(6)));
  REQUIRE(result.reached(at<G>(7)));
  REQUIRE(result.reached(at<G>(8)));
  REQUIRE_FALSE(result.reached(at<G>(2)));
  REQUIRE_FALSE(result.reached(at<G>(3)));
  REQUIRE(runtime_level_of(result, at<G>(8)) == 2);
  REQUIRE(runtime_predecessor_of(result, at<G>(8)) == at<G>(7));
  REQUIRE((path_ids<G>(runtime_path_from(result, at<G>(8))) == std::vector<unsigned>{8, 7, 0}));

  auto to_result = hg::bfs_to(g, at<G>(0), at<G>(8));
  REQUIRE(to_result.reached());
  REQUIRE(runtime_target_level(to_result) == 2);
  REQUIRE((path_ids<G>(runtime_target_path(to_result)) == std::vector<unsigned>{8, 7, 0}));
}
