#include "model.hpp"

using namespace heim_test::graph_algorithm::bfs;

TEST_CASE("BFS search computes levels and predecessors along a directed path", "[graph][bfs][search]")
{
  graph<> g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}, {3, 4}});

  auto result = hg::breadth_first_search(g, at<graph<>>(0));
  reference_graph model{5, true};
  model.edges = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};
  verify_search(g, result, model, {0});
  REQUIRE(result.level_of(at<graph<>>(4)) == 4);
}

TEST_CASE("BFS search preserves unreachable vertices", "[graph][bfs][search]")
{
  graph<> g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {3, 4}});

  auto result = hg::bfs(g, at<graph<>>(0));
  reference_graph model{5, true};
  model.edges = {{0, 1}, {1, 2}, {3, 4}};
  verify_search(g, result, model, {0});
  REQUIRE_FALSE(result.reached(at<graph<>>(3)));
  REQUIRE_FALSE(result.reached(at<graph<>>(4)));
}

TEST_CASE("BFS search uses a shortest path rather than the first longer route", "[graph][bfs][search]")
{
  graph<> g;
  make_vertices(g, 6);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}, {0, 4}, {4, 3}, {3, 5}});

  auto result = hg::bfs(g, at<graph<>>(0));
  reference_graph model{6, true};
  model.edges = {{0, 1}, {1, 2}, {2, 3}, {0, 4}, {4, 3}, {3, 5}};
  verify_search(g, result, model, {0});
  REQUIRE(result.level_of(at<graph<>>(3)) == 2);
  REQUIRE(result.level_of(at<graph<>>(5)) == 3);
}

TEST_CASE("BFS search supports multiple roots with shortest distances", "[graph][bfs][search]")
{
  graph<> g;
  make_vertices(g, 7);
  add_edges(g, {{0, 1}, {1, 2}, {2, 3}, {6, 5}, {5, 4}, {4, 3}});

  std::array const roots{at<graph<>>(0), at<graph<>>(6)};
  auto result = hg::bfs(g, roots);
  reference_graph model{7, true};
  model.edges = {{0, 1}, {1, 2}, {2, 3}, {6, 5}, {5, 4}, {4, 3}};
  verify_search(g, result, model, {0, 6});
  REQUIRE(result.level_of(at<graph<>>(3)) == 3);
}

TEST_CASE("BFS search ignores duplicate roots", "[graph][bfs][search]")
{
  graph<> g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});

  std::array const roots{at<graph<>>(0), at<graph<>>(0), at<graph<>>(1)};
  auto result = hg::bfs(g, roots);
  reference_graph model{3, true};
  model.edges = {{0, 1}, {1, 2}};
  verify_search(g, result, model, {0, 1});
  REQUIRE(result.discovery_time_of(at<graph<>>(0)) == 1);
  REQUIRE(result.discovery_time_of(at<graph<>>(1)) == 2);
}

TEST_CASE("BFS search with no roots leaves every vertex unreached", "[graph][bfs][search]")
{
  graph<> g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});

  std::vector<graph<>::vertex_type> roots;
  auto result = hg::bfs(g, roots);
  reference_graph model{3, true};
  model.edges = {{0, 1}, {1, 2}};
  verify_search(g, result, model, {});
}

TEST_CASE("BFS search processes a graph with only isolated vertices", "[graph][bfs][search]")
{
  graph<> g;
  make_vertices(g, 4);

  auto result = hg::bfs(g, at<graph<>>(2));
  reference_graph model{4, true};
  verify_search(g, result, model, {2});
  REQUIRE(result.discovery_time_of(at<graph<>>(2)) == 1);
}

TEST_CASE("BFS search on a self-loop does not rediscover its root", "[graph][bfs][search]")
{
  graph<true, true> g;
  make_vertices(g, 2);
  add_edges(g, {{0, 0}, {0, 0}, {0, 1}, {1, 1}});

  auto result = hg::bfs(g, at<graph<true, true>>(0));
  reference_graph model{2, true};
  model.edges = {{0, 0}, {0, 0}, {0, 1}, {1, 1}};
  verify_search(g, result, model, {0});
  REQUIRE(result.discovery_time_of(at<graph<true, true>>(1)) == 2);
}

TEST_CASE("BFS aliases and result type match the full algorithm names", "[graph][bfs][search]")
{
  using G = graph<>;
  static_assert(std::same_as<hg::bfs_result<G>, hg::breadth_first_search_result<G>>);
  static_assert(std::same_as<decltype(hg::bfs), decltype(hg::breadth_first_search)>);

  G g;
  make_vertices(g, 2);
  add_edges(g, {{0, 1}});
  REQUIRE(hg::bfs(g, at<G>(0)).level_of(at<G>(1)) == 1);
}
