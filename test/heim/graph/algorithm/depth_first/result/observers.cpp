#include "../support.hpp"

using namespace heim_test::graph_algorithm::dfs;

TEST_CASE("DFS discovery and finish times are consecutive on a chain", "[graph][dfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}});
  auto result = hg::dfs(g, at<G>(0));
  for (unsigned i = 0; i < 4; ++i)
  {
    REQUIRE(result.discovery_time_of(at<G>(i)) == i + 1);
    REQUIRE(result.finish_time_of(at<G>(i)) == 8 - i);
  }
}

TEST_CASE("DFS timestamps are unique across all reached components", "[graph][dfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {2, 3}});
  auto roots = std::array{at<G>(0), at<G>(2)};
  auto result = hg::dfs(g, roots);
  std::set<std::size_t> times;
  for (unsigned id = 0; id < 4; ++id)
  {
    REQUIRE(times.insert(result.discovery_time_of(at<G>(id))).second);
    REQUIRE(times.insert(result.finish_time_of(at<G>(id))).second);
  }
  REQUIRE(times.size() == 8);
  for (std::size_t i = 1; i <= 8; ++i)
    REQUIRE(times.contains(i));
}

TEST_CASE("DFS result exposes invalid times for unreachable vertices", "[graph][dfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}});
  auto result = hg::dfs(g, at<G>(0));
  REQUIRE_FALSE(result.reached(at<G>(2)));
  REQUIRE(result.discovery_time_of(at<G>(2)) == std::numeric_limits<std::size_t>::max());
  REQUIRE(result.finish_time_of(at<G>(2)) == std::numeric_limits<std::size_t>::max());
}

TEST_CASE("DFS result can be observed through const references", "[graph][dfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  auto const result = hg::dfs(g, at<G>(0));
  REQUIRE(result.reached(at<G>(2)));
  REQUIRE(runtime_predecessor_of(result, at<G>(2)) == at<G>(1));
  REQUIRE(result.discovery_time_of(at<G>(2)) == 3);
  REQUIRE(result.finish_time_of(at<G>(2)) == 4);
}

TEST_CASE("DFS_to exposes invalid discovery time for unreachable target", "[graph][dfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  auto const result = hg::dfs_to(g, at<G>(0), at<G>(2));
  REQUIRE_FALSE(result.reached());
  REQUIRE(result.discovery_time() == std::numeric_limits<std::size_t>::max());
}

TEST_CASE("DFS_to emits no finish time API for its incomplete traversal", "[graph][dfs][result]")
{
  using G = graph<>;
  using R = hg::dfs_to_result<G>;
  static_assert(std::is_move_constructible_v<R>);
  G g;
  make_vertices(g, 2);
  add_edges(g, {{0, 1}});
  auto result = hg::dfs_to(g, at<G>(0), at<G>(1));
  REQUIRE(result.reached());
  REQUIRE(result.discovery_time() == 2);
}
