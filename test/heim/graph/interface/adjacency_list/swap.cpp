#include "support.hpp"
#include <utility>
using namespace heim_test::adjacency;

TEST_CASE("adjacency_list member swap exchanges complete graph states", "[graph][adjacency_list][swap]")
{
  graph<> a{{{0,1},{1,2}}},b{{{4,5}}};
  auto sa=a,sb=b;
  a.swap(b);
  REQUIRE(a==sb);REQUIRE(b==sa);
  check_consistency(a);check_consistency(b);
}
TEST_CASE("adjacency_list ADL swap exchanges complete graph states", "[graph][adjacency_list][swap]")
{
  graph<true,true> a{{{0,1},{0,1}}},b{{{0,0},{2,3}}};
  auto sa=a,sb=b;
  using std::swap;swap(a,b);
  REQUIRE(a==sb);REQUIRE(b==sa);
  check_consistency(a);check_consistency(b);
}
TEST_CASE("adjacency_list swap preserves the next identifiers", "[graph][adjacency_list][swap]")
{
  graph<> a,b;
  make_vertices(a,4);make_vertices(b,2);
  a.swap(b);
  REQUIRE(a.place_vertex().id()==2u);
  REQUIRE(b.place_vertex().id()==4u);
}
TEST_CASE("adjacency_list self-swap retains content", "[graph][adjacency_list][swap]")
{
  graph<> g{{{0,1},{1,2}}};
  auto copy=g;
  g.swap(g);
  REQUIRE(g==copy);
  check_consistency(g);
}
