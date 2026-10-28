#include "support.hpp"

using namespace heim_test::adjacency;

TEST_CASE(
    "adjacency_list equality compares equal construction histories",
    "[graph][adjacency_list][equality]")
{
  graph<> a{{{0, 1}, {1, 2}}}, b{{{0, 1}, {1, 2}}};
  REQUIRE(a == b);
  REQUIRE_FALSE(a != b);
}

TEST_CASE("adjacency_list equality distinguishes graph contents", "[graph][adjacency_list][equality]")
{
  graph<> a{{{0, 1}}}, b{{{0, 2}}};
  REQUIRE(a != b);
}

TEST_CASE("adjacency_list equality includes identifier generators", "[graph][adjacency_list][equality]")
{
  graph<> a, b;
  auto v = a.place_vertex();
  a.remove_vertex(v);
  verify_empty(a);
  verify_empty(b);
  REQUIRE(a != b);
  REQUIRE(a.place_vertex().id() == 1u);
  REQUIRE(b.place_vertex().id() == 0u);
}

TEST_CASE("adjacency_list equality includes edge insertion history", "[graph][adjacency_list][equality]")
{
  graph<> a, b;
  auto va = make_vertices(a, 2), vb = make_vertices(b, 2);
  auto [edge, added] = a.place_edge(va[0], va[1]);
  REQUIRE(added);
  a.remove_edge(edge);
  verify_empty(graph<>{});
  REQUIRE(a.edge_count() == 0);
  REQUIRE(b.edge_count() == 0);
  REQUIRE(a != b);
  REQUIRE(a.place_edge(va[0], va[1]).first.id() == 1u);
  REQUIRE(b.place_edge(vb[0], vb[1]).first.id() == 0u);
}

TEST_CASE("adjacency_list copy equals original until modified", "[graph][adjacency_list][equality]")
{
  graph<true, true> g{{{0, 1}, {0, 1}, {1, 2}}};
  auto clone = g;
  REQUIRE(g == clone);
  clone.place_edge(graph<true, true>::vertex_type{2}, graph<true, true>::vertex_type{2});
  REQUIRE(g != clone);
}
