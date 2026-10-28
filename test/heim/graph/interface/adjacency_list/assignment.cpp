#include "support.hpp"

using namespace heim_test::adjacency;

TEST_CASE("adjacency_list copy assignment replaces the whole graph", "[graph][adjacency_list][assignment]")
{
  graph<> src{{{0, 2}, {2, 5}}}, dst{{{10, 11}}};
  auto expected = snapshot(src);
  dst = src;
  REQUIRE(dst == src);
  verify(dst, expected);
  dst.place_vertex();
  REQUIRE(dst != src);
  check_consistency(src);
}

TEST_CASE("adjacency_list move assignment transfers whole graph", "[graph][adjacency_list][assignment]")
{
  graph<true> src{{{0, 1}, {1, 2}}}, dst{{{10, 11}}};
  auto expected = snapshot(src);
  dst = std::move(src);
  verify(dst, expected);
  REQUIRE(dst.place_vertex().id() == 3u);
}

TEST_CASE(
    "adjacency_list initializer-list assignment replaces previous vertices and edges",
    "[graph][adjacency_list][assignment]")
{
  graph<> g{{{0, 1}, {2, 3}}};
  g = {{5, 7}, {7, 9}};
  REQUIRE((snapshot(g).vertices == std::set<unsigned>{5, 7, 9}));
  REQUIRE(g.edge_count() == 2);
  REQUIRE(g.place_vertex().id() == 10u);
  check_consistency(g);
}

TEST_CASE("adjacency_list initializer-list assignment can clear graph", "[graph][adjacency_list][assignment]")
{
  graph<> g{{{2, 3}}};
  g = {};
  verify_empty(g);
  REQUIRE(g.place_vertex().id() == 0u);
}

TEST_CASE("adjacency_list copy self-assignment maintains graph", "[graph][adjacency_list][assignment]")
{
  graph<false, true> g{{{0, 1}, {0, 1}, {1, 1}}};
  auto const saved = g;
  g = g;
  REQUIRE(g == saved);
  check_consistency(g);
}

TEST_CASE(
    "adjacency_list initializer-list assignment resets identifier generators",
    "[graph][adjacency_list][assignment]")
{
  graph<true> g{{{20, 40}}};
  g = {{1, 2}};
  REQUIRE(g.place_vertex().id() == 3u);
  auto e = g.place_edge(graph<true>::vertex_type{1u}, graph<true>::vertex_type{2u});
  REQUIRE_FALSE(e.second);
  auto f = g.place_edge(graph<true>::vertex_type{2u}, graph<true>::vertex_type{1u});
  REQUIRE(f.second);
  REQUIRE(f.first.id() == 1u);
}
