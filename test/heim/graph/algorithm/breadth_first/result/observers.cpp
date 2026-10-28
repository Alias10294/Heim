#include "../support.hpp"

using namespace heim_test::graph_algorithm::bfs;

TEST_CASE("BFS result marks each discovered vertex with a unique time", "[graph][bfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 6);
  add_edges(g, {{0, 1}, {0, 2}, {1, 3}, {2, 4}});
  auto result = hg::bfs(g, at<G>(0));

  std::set<std::size_t> times;
  for (unsigned id = 0; id < 5; ++id)
  {
    REQUIRE(result.reached(at<G>(id)));
    REQUIRE(times.insert(result.discovery_time_of(at<G>(id))).second);
  }
  REQUIRE((times == std::set<std::size_t>{1, 2, 3, 4, 5}));
  REQUIRE_FALSE(result.reached(at<G>(5)));
  REQUIRE(result.discovery_time_of(at<G>(5)) == std::numeric_limits<std::size_t>::max());
}

TEST_CASE("BFS results compute the correct level for every source vertex", "[graph][bfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  std::array const roots{at<G>(0), at<G>(1), at<G>(2)};
  auto result = hg::bfs(g, roots);

  for (auto root : roots)
  {
    REQUIRE(result.reached(root));
    REQUIRE(result.level_of(root) == 0);
    REQUIRE(result.predecessor_of(root) == root);
  }
  REQUIRE_FALSE(result.reached(at<G>(3)));
}

TEST_CASE("BFS result accessors work on const result objects", "[graph][bfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  auto const result = hg::bfs(g, at<G>(0));

  REQUIRE(result.reached(at<G>(2)));
  REQUIRE(result.level_of(at<G>(2)) == 2);
  REQUIRE(result.predecessor_of(at<G>(2)) == at<G>(1));
  REQUIRE(result.discovery_time_of(at<G>(0)) == 1);
  REQUIRE((path_ids<G>(result.path_from(at<G>(2))) == std::vector<unsigned>{2, 1, 0}));
}

TEST_CASE("BFS_to reached and discovery_time are safe for unreachable targets", "[graph][bfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}});
  auto const result = hg::bfs_to(g, at<G>(0), at<G>(2));

  REQUIRE_FALSE(result.reached());
  REQUIRE(result.discovery_time() == std::numeric_limits<std::size_t>::max());
}

TEST_CASE("BFS_to result accessors work after move construction", "[graph][bfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  auto result = hg::bfs_to(g, at<G>(0), at<G>(2));
  auto moved = std::move(result);

  REQUIRE(moved.reached());
  REQUIRE(moved.level() == 2);
  REQUIRE((path_ids<G>(moved.path()) == std::vector<unsigned>{2, 1, 0}));
}
