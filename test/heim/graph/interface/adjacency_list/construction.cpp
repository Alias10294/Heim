#include "support.hpp"
#include <array>
#include <initializer_list>
#include <sstream>
#include <utility>
using namespace heim_test::adjacency;

TEST_CASE("adjacency_list initializer-list creates sparse vertex identifiers", "[graph][adjacency_list][construction]")
{
  graph<> g{{{2,7},{7,13},{2,13}}};
  REQUIRE(g.vertex_count()==3);REQUIRE(g.edge_count()==3);
  REQUIRE((std::set<unsigned>{2,7,13}==snapshot(g).vertices));
  REQUIRE(g.place_vertex().id()==14u);
  check_consistency(g);
}
TEST_CASE("adjacency_list initializer-list ignores duplicates in simple graph", "[graph][adjacency_list][construction]")
{
  graph<> g{{{0,1},{1,0},{0,1},{0,0}}};
  REQUIRE(g.vertex_count()==2);
  REQUIRE(g.edge_count()==2);
  check_consistency(g);
}
TEST_CASE("adjacency_list initializer-list preserves parallel edges in multigraph", "[graph][adjacency_list][construction]")
{
  graph<false,true> g{{{0,1},{1,0},{0,1},{0,0}}};
  REQUIRE(g.vertex_count()==2);
  REQUIRE(g.edge_count()==4);
  check_consistency(g);
}
TEST_CASE("adjacency_list directed initializer-list retains opposite directions", "[graph][adjacency_list][construction]")
{
  graph<true> g{{{0,1},{1,0},{0,1}}};
  REQUIRE(g.vertex_count()==2);
  REQUIRE(g.edge_count()==2);
  check_consistency(g);
}
TEST_CASE("adjacency_list iterator pair builds graph", "[graph][adjacency_list][construction]")
{
  std::vector<std::pair<unsigned,unsigned>> input{{0,4},{4,8},{8,0}};
  graph<true> g(input.begin(),input.end());
  REQUIRE(g.vertex_count()==3);REQUIRE(g.edge_count()==3);
  REQUIRE(g.place_vertex().id()==9u);
  check_consistency(g);
}
TEST_CASE("adjacency_list from_range builds graph", "[graph][adjacency_list][construction]")
{
  std::array input{std::pair{0u,2u},std::pair{2u,7u}};
  graph<> g(std::from_range,input);
  REQUIRE(g.vertex_count()==3);REQUIRE(g.edge_count()==2);
  check_consistency(g);
}
namespace
{
struct input_edge
{
  unsigned first{},second{};
};
std::istream &operator>>(std::istream &is,input_edge &edge)
{ return is >> edge.first >> edge.second; }
}
TEST_CASE("adjacency_list accepts a genuinely single-pass input iterator", "[graph][adjacency_list][construction]")
{
  std::istringstream stream{"0 2 4 6 6 8"};
  graph<> g(std::istream_iterator<input_edge>{stream},std::istream_iterator<input_edge>{});
  REQUIRE(g.vertex_count()==5);REQUIRE(g.edge_count()==3);
  check_consistency(g);
}
TEST_CASE("adjacency_list constructors accept allocators", "[graph][adjacency_list][construction]")
{
  std::vector<std::pair<unsigned,unsigned>> input{{0,1},{1,2}};
  graph<> a(input.begin(),input.end(),std::allocator<unsigned>{});
  graph<> b(std::from_range,input,std::allocator<unsigned>{});
  graph<> c({{0,1},{1,2}},std::allocator<unsigned>{});
  REQUIRE(a==b);REQUIRE(b==c);
}
TEST_CASE("adjacency_list empty edge input yields empty graph", "[graph][adjacency_list][construction]")
{
  std::vector<std::pair<unsigned,unsigned>> empty;
  graph<> g(std::from_range,empty);
  verify_empty(g);
  REQUIRE(g.place_vertex().id()==0u);
}
TEST_CASE("adjacency_list move constructor transfers graph state", "[graph][adjacency_list][construction]")
{
  graph<> source{{{1,2},{2,3}}};
  auto expected=snapshot(source);
  graph<> target(std::move(source));
  verify(target,expected);
  REQUIRE(target.place_vertex().id()==4u);
}
TEST_CASE("adjacency_list copy constructor leaves independent incidence storage", "[graph][adjacency_list][construction]")
{
  graph<true,true> a{{{0,1},{0,1},{1,2}}};
  graph<true,true> b(a);
  REQUIRE(a==b);
  auto v0=graph<true,true>::vertex_type{0u},v1=graph<true,true>::vertex_type{1u};
  b.remove_edges(v0,v1);
  REQUIRE(a.edge_count()==3);
  REQUIRE(b.edge_count()==1);
  check_consistency(a);check_consistency(b);
}
