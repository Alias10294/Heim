#include "support.hpp"
using namespace heim_test::adjacency;

TEST_CASE("adjacency_list default constructed graph is empty", "[graph][adjacency_list][basic]")
{
  graph<> g;verify_empty(g);
  REQUIRE(std::ranges::distance(g.vertices())==0);
  REQUIRE(std::ranges::distance(g.edges())==0);
}
TEST_CASE("adjacency_list construction with explicit allocator is empty", "[graph][adjacency_list][basic]")
{
  graph<> g{std::allocator<unsigned>{}};
  verify_empty(g);
}
TEST_CASE("adjacency_list copies remain independent", "[graph][adjacency_list][basic]")
{
  graph<> g{{{0,1},{1,2}}};
  auto copy=g;
  REQUIRE(g==copy);
  copy.place_vertex();
  REQUIRE(g!=copy);
  REQUIRE(g.vertex_count()==3);
  REQUIRE(copy.vertex_count()==4);
  check_consistency(g);check_consistency(copy);
}
TEST_CASE("adjacency_list missing edges do not create spurious vertices", "[graph][adjacency_list][basic]")
{
  graph<> g;auto v=make_vertices(g,3);
  auto result=g.place_edge(v[0],v[1]);
  REQUIRE(result.second);
  auto dup=g.place_edge(v[1],v[0]);
  REQUIRE_FALSE(dup.second);
  REQUIRE(g.vertex_count()==3);
  REQUIRE(g.edge_count()==1);
  check_consistency(g);
}
TEST_CASE("adjacency_list empty ranges after all vertices removed", "[graph][adjacency_list][basic]")
{
  graph<> g;auto v=make_vertices(g,3);
  auto e=g.place_edge(v[0],v[1]);REQUIRE(e.second);
  g.remove_vertex(v[0]);g.remove_vertex(v[1]);g.remove_vertex(v[2]);
  verify_empty(g);
  auto next=g.place_vertex();
  REQUIRE(next.id()==3u);
}
