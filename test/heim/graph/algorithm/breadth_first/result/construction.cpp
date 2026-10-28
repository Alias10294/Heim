#include "../support.hpp"

using namespace heim_test::graph_algorithm::bfs;

TEST_CASE("BFS result is constructible in place for a graph type", "[graph][bfs][result]")
{
  using G = graph<>;
  static_assert(std::constructible_from<hg::bfs_result<G>, std::in_place_type_t<G>>);
  static_assert(std::move_constructible<hg::bfs_result<G>>);
  static_assert(!std::copy_constructible<hg::bfs_result<G>>);

  hg::bfs_result<G> result{std::in_place_type<G>};
  G g;
  make_vertices(g, 2);
  add_edges(g, {{0, 1}});
  hg::bfs(g, result, at<G>(0));
  REQUIRE(result.level_of(at<G>(1)) == 1);
}

TEST_CASE("BFS result can be populated through the output-result overload", "[graph][bfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {3, 4}});
  hg::bfs_result<G> result{std::in_place_type<G>};

  hg::breadth_first_search(g, result, at<G>(0));
  REQUIRE(result.reached(at<G>(0)));
  REQUIRE(result.reached(at<G>(2)));
  REQUIRE_FALSE(result.reached(at<G>(3)));
  REQUIRE(result.level_of(at<G>(2)) == 2);
}

TEST_CASE("BFS output-result overload accepts multiple roots", "[graph][bfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {3, 4}});
  hg::bfs_result<G> result{std::in_place_type<G>};
  std::array const roots{at<G>(0), at<G>(3)};

  hg::bfs(g, result, roots);
  REQUIRE(result.reached(at<G>(4)));
  REQUIRE(result.level_of(at<G>(4)) == 1);
  REQUIRE(result.level_of(at<G>(2)) == 2);
}

TEST_CASE("BFS returned result owns its default maps after the algorithm returns", "[graph][bfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});

  auto result = hg::bfs(g, at<G>(0));
  auto moved = std::move(result);
  REQUIRE(moved.reached(at<G>(2)));
  REQUIRE(moved.level_of(at<G>(2)) == 2);
  REQUIRE((path_ids<G>(moved.path_from(at<G>(2))) == std::vector<unsigned>{2, 1, 0}));
}

TEST_CASE("BFS result cannot return a predecessor view from a temporary", "[graph][bfs][result]")
{
  using G = graph<>;
  using R = hg::bfs_result<G>;
  using V = vertex<G>;
  static_assert(requires(R const &r, V v) { r.path_from(v); });
  // Rvalue path access is checked by the dependent concepts in traits.cpp.
}

TEST_CASE("BFS_to result stores an explicit target", "[graph][bfs][result]")
{
  using G = graph<>;
  using R = hg::bfs_to_result<G>;
  static_assert(std::constructible_from<R, std::in_place_type_t<G>, vertex<G>>);
  static_assert(std::move_constructible<R>);

  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}});
  R result{std::in_place_type<G>, at<G>(2)};
  hg::bfs_to(g, result, at<G>(0));
  REQUIRE(result.reached());
  REQUIRE(result.level() == 2);
}

TEST_CASE("BFS_to output-result overload accepts a root range", "[graph][bfs][result]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 6);
  add_edges(g, {{0, 1}, {1, 2}, {3, 4}, {4, 5}});
  hg::bfs_to_result<G> result{std::in_place_type<G>, at<G>(5)};
  std::array const roots{at<G>(0), at<G>(3)};

  hg::breadth_first_search_to(g, result, roots);
  REQUIRE(result.reached());
  REQUIRE(result.level() == 2);
  REQUIRE((path_ids<G>(result.path()) == std::vector<unsigned>{5, 4, 3}));
}

TEST_CASE("BFS_to result cannot return a path view from a temporary", "[graph][bfs][result]")
{
  using G = graph<>;
  using R = hg::bfs_to_result<G>;
  static_assert(requires(R const &r) { r.path(); });
  // Rvalue path access is checked by the dependent concepts in traits.cpp.
}
