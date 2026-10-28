#include "model.hpp"

using namespace heim_test::graph_algorithm::bfs;

TEST_CASE("BFS_to finds the shortest path to a reachable target", "[graph][bfs][search_to]")
{
  graph<> g;
  make_vertices(g, 6);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}, {0, 4}, {4, 3}, {3, 5}});
  reference_graph model{6, true};
  model.edges = {{0, 1}, {1, 2}, {2, 3}, {0, 4}, {4, 3}, {3, 5}};

  auto result = hg::breadth_first_search_to(g, at<graph<>>(0), at<graph<>>(5));
  verify_target(result, model, {0}, 5);
  REQUIRE(result.level() == 3);
}

TEST_CASE("BFS_to reports an unreachable target without asking for an invalid path", "[graph][bfs][search_to]")
{
  graph<> g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {3, 4}});
  reference_graph model{5, true};
  model.edges = {{0, 1}, {1, 2}, {3, 4}};

  auto result = hg::bfs_to(g, at<graph<>>(0), at<graph<>>(4));
  verify_target(result, model, {0}, 4);
}

TEST_CASE("BFS_to recognizes a target identical to the root", "[graph][bfs][search_to]")
{
  graph<> g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});

  auto result = hg::bfs_to(g, at<graph<>>(0), at<graph<>>(0));
  REQUIRE(result.reached());
  REQUIRE(result.level() == 0);
  REQUIRE(result.discovery_time() == 1);
  REQUIRE((path_ids<graph<>>(result.path()) == std::vector<unsigned>{0}));
}

TEST_CASE("BFS_to recognizes a target among multiple roots", "[graph][bfs][search_to]")
{
  graph<> g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {3, 4}});
  std::array const roots{at<graph<>>(0), at<graph<>>(3)};

  auto result = hg::bfs_to(g, roots, at<graph<>>(3));
  REQUIRE(result.reached());
  REQUIRE(result.level() == 0);
  REQUIRE(result.discovery_time() == 2);
  REQUIRE((path_ids<graph<>>(result.path()) == std::vector<unsigned>{3}));
}

TEST_CASE("BFS_to with no roots leaves target unreachable", "[graph][bfs][search_to]")
{
  graph<> g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  std::vector<vertex<graph<>>> roots;

  auto result = hg::bfs_to(g, roots, at<graph<>>(2));
  REQUIRE_FALSE(result.reached());
  REQUIRE(result.discovery_time() == std::numeric_limits<std::size_t>::max());
}

TEST_CASE("BFS_to stops at discovery rather than finish of target", "[graph][bfs][search_to]")
{
  graph<> g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}});
  std::unordered_map<vertex<graph<>>, hg::color> colors;
  auto cm = heim::make_property_map(colors);

  auto result = hg::bfs_to(g, at<graph<>>(0), at<graph<>>(1),
                           hg::default_property_map, hg::default_property_map,
                           hg::default_property_map, cm);
  REQUIRE(result.reached());
  REQUIRE(result.level() == 1);
  REQUIRE(colors.at(at<graph<>>(0)) == hg::color::gray);
  REQUIRE(colors.at(at<graph<>>(1)) == hg::color::gray);
  REQUIRE(colors.at(at<graph<>>(2)) == hg::color::white);
}

TEST_CASE("BFS_to supports undirected multigraphs", "[graph][bfs][search_to]")
{
  graph<false, true> g;
  make_vertices(g, 4);
  add_edges(g, {{0, 1}, {0, 1}, {1, 2}, {2, 2}, {2, 3}});
  reference_graph model{4, false};
  model.edges = {{0, 1}, {0, 1}, {1, 2}, {2, 2}, {2, 3}};

  auto result = hg::bfs_to(g, at<graph<false, true>>(3), at<graph<false, true>>(0));
  verify_target(result, model, {3}, 0);
}

TEST_CASE("BFS_to aliases and result types match full names", "[graph][bfs][search_to]")
{
  using G = graph<>;
  static_assert(std::same_as<hg::bfs_to_result<G>, hg::breadth_first_search_to_result<G>>);
  static_assert(std::same_as<decltype(hg::bfs_to), decltype(hg::breadth_first_search_to)>);
  G g;
  make_vertices(g, 1);
  REQUIRE(hg::bfs_to(g, at<G>(0), at<G>(0)).reached());
}
