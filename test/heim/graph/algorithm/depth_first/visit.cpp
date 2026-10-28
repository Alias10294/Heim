#include "visitors.hpp"

using namespace heim_test::graph_algorithm::dfs;

TEST_CASE("DFS visit completes an isolated root", "[graph][dfs][visit]")
{
  graph<> g;
  make_vertices(g, 3);
  REQUIRE(hg::depth_first_visit(g, at<graph<>>(1), std::monostate{}));
}

TEST_CASE("DFS visit accepts a nonempty root range", "[graph][dfs][visit]")
{
  graph<> g;
  make_vertices(g, 5);
  add_edges(g, {{0, 1}, {1, 2}, {3, 4}});
  auto roots = std::array{at<graph<>>(0), at<graph<>>(3)};
  REQUIRE(hg::depth_first_visit(g, roots, std::monostate{}));
}

TEST_CASE("DFS visit accepts an empty root range", "[graph][dfs][visit]")
{
  graph<> g;
  make_vertices(g, 2);
  std::vector<vertex<graph<>>> roots;
  REQUIRE(hg::depth_first_visit(g, roots, std::monostate{}));
}

TEST_CASE("DFS visit accepts an empty graph and empty roots", "[graph][dfs][visit]")
{
  graph<> g;
  std::vector<vertex<graph<>>> roots;
  REQUIRE(hg::depth_first_visit(g, roots, std::monostate{}));
}

TEST_CASE("DFS visit visits only roots reachable within the directed graph", "[graph][dfs][visit]")
{
  auto g = make_event_graph();
  std::vector<int> discovered;
  struct visitor
  {
    std::vector<int> *values;
    void discover_vertex(external_graph const &, int v) const { values->push_back(v); }
  } vis{&discovered};

  REQUIRE(hg::depth_first_visit(g, 0, vis));
  REQUIRE((discovered == std::vector<int>{0, 1, 2}));
}

TEST_CASE("DFS visit skips repeated roots that have already been discovered", "[graph][dfs][visit]")
{
  auto g = make_event_graph();
  std::vector<int> roots_started;
  struct visitor
  {
    std::vector<int> *roots;
    void start_vertex(external_graph const &, int v) const { roots->push_back(v); }
  } vis{&roots_started};
  std::array const roots{0, 1, 0, 3, 4};
  REQUIRE(hg::depth_first_visit(g, roots, vis));
  REQUIRE((roots_started == std::vector<int>{0, 3}));
}

TEST_CASE("DFS visit handles directed self-loops and parallel edges", "[graph][dfs][visit]")
{
  graph<true, true> g;
  make_vertices(g, 2);
  add_edges(g, {{0, 0}, {0, 1}, {0, 1}, {1, 0}});
  REQUIRE(hg::depth_first_visit(g, at<graph<true, true>>(0), std::monostate{}));
}

TEST_CASE("DFS visit handles undirected self-loops and parallel edges", "[graph][dfs][visit]")
{
  graph<false, true> g;
  make_vertices(g, 3);
  add_edges(g, {{0, 0}, {0, 1}, {0, 1}, {1, 2}});
  REQUIRE(hg::depth_first_visit(g, at<graph<false, true>>(0), std::monostate{}));
}

TEST_CASE("DFS visit works on outgoing-only directed graph", "[graph][dfs][visit]")
{
  graph<true, false, false, true> g;
  make_vertices(g, 3);
  add_edges(g, {{0, 1}, {1, 2}});
  REQUIRE(hg::depth_first_visit(g, at<decltype(g)>(0), std::monostate{}));
}

