#include "../support.hpp"

using namespace heim_test::graph_algorithm::dfs;

TEST_CASE("DFS result can be constructed and filled in place", "[graph][dfs][result]")
{
  using G = graph<>;
  static_assert(std::constructible_from<hg::dfs_result<G>, std::in_place_type_t<G>>);
  static_assert(std::move_constructible<hg::dfs_result<G>>);
  static_assert(!std::copy_constructible<hg::dfs_result<G>>);
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  hg::dfs_result<G> result{std::in_place_type<G>};
  hg::dfs(g, result, at<G>(0));
  REQUIRE(result.reached(at<G>(2)));
  REQUIRE(result.discovery_time_of(at<G>(2)) == 3);
  REQUIRE(result.finish_time_of(at<G>(2)) == 4);
}

TEST_CASE("DFS output-result overload accepts several roots", "[graph][dfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {2, 3}});
  std::array roots{at<G>(0), at<G>(2)};
  hg::dfs_result<G> result{std::in_place_type<G>};
  hg::depth_first_search(g, result, roots);
  for (unsigned id = 0; id < 4; ++id)
    REQUIRE(result.reached(at<G>(id)));
  REQUIRE_FALSE(result.reached(at<G>(4)));
}

TEST_CASE("DFS owned result survives move construction", "[graph][dfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}});
  auto result = hg::dfs(g, at<G>(0));
  auto moved = std::move(result);
  REQUIRE(moved.reached(at<G>(3)));
  REQUIRE(moved.discovery_time_of(at<G>(3)) == 4);
  REQUIRE(moved.finish_time_of(at<G>(3)) == 5);
  REQUIRE((path_ids<G>(runtime_path_from(moved, at<G>(3))) ==
           std::vector<unsigned>{3, 2, 1, 0}));
}

TEST_CASE("DFS result is independent of a temporary root range", "[graph][dfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {2, 3}});
  auto result = hg::dfs(g, std::vector{at<G>(2)});
  REQUIRE(result.reached(at<G>(3)));
  REQUIRE_FALSE(result.reached(at<G>(1)));
}

TEST_CASE("DFS_to result stores its explicit target", "[graph][dfs][result]")
{
  using G = graph<>;
  static_assert(std::constructible_from<hg::dfs_to_result<G>, std::in_place_type_t<G>, vertex<G>>);
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}});
  hg::dfs_to_result<G> result{std::in_place_type<G>, at<G>(2)};
  hg::depth_first_search_to(g, result, at<G>(0));
  REQUIRE(result.reached());
  REQUIRE(result.discovery_time() == 3);
}

TEST_CASE("DFS_to result survives move construction", "[graph][dfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}});
  auto result = hg::dfs_to(g, at<G>(0), at<G>(2));
  auto moved = std::move(result);
  REQUIRE(moved.reached());
  REQUIRE(moved.discovery_time() == 3);
  REQUIRE((path_ids<G>(runtime_target_path(moved)) == std::vector<unsigned>{2, 1, 0}));
}

TEST_CASE("DFS and DFS_to result types have distinct observer interfaces", "[graph][dfs][result]")
{
  using G = graph<>;
  using R = hg::dfs_result<G>;
  using T = hg::dfs_to_result<G>;
  static_assert(requires(R const &r, vertex<G> v) { r.finish_time_of(v); });
  static_assert(requires(T const &r) { r.discovery_time(); });
}
