#include "../support.hpp"

using namespace heim_test::graph_algorithm::bfs;

TEST_CASE("BFS path_from returns the target-to-root chain", "[graph][bfs][result][path]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}, {3, 4}});
  auto result = hg::bfs(g, at<G>(0));

  REQUIRE((path_ids<G>(result.path_from(at<G>(4))) == std::vector<unsigned>{4, 3, 2, 1, 0}));
}

TEST_CASE("BFS path_from of a root contains that root exactly once", "[graph][bfs][result][path]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 2);
  auto result = hg::bfs(g, at<G>(1));

  REQUIRE((path_ids<G>(result.path_from(at<G>(1))) == std::vector<unsigned>{1}));
}

TEST_CASE("BFS result paths remain independent of edges added after the search", "[graph][bfs][result][path]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}});
  auto result = hg::bfs(g, at<G>(0));

  (void)g.place_edge(at<G>(0), at<G>(3));
  REQUIRE((path_ids<G>(result.path_from(at<G>(3))) == std::vector<unsigned>{3, 2, 1, 0}));
}

TEST_CASE("BFS_to path for a direct neighbor has two vertices", "[graph][bfs][result][path]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  auto result = hg::bfs_to(g, at<G>(0), at<G>(1));

  REQUIRE((path_ids<G>(result.path()) == std::vector<unsigned>{1, 0}));
}

TEST_CASE("BFS path views support standard range iteration", "[graph][bfs][result][path]")
{
  using G = graph<>;
  G g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  auto result = hg::bfs(g, at<G>(0));
  auto path = result.path_from(at<G>(2));

  static_assert(std::ranges::input_range<decltype(path)>);
  static_assert(std::ranges::view<decltype(path)>);
  REQUIRE(std::ranges::distance(path) == 3);
  REQUIRE((path_ids<G>(result.path_from(at<G>(2))) == std::vector<unsigned>{2, 1, 0}));
}
