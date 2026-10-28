#include "support.hpp"
using namespace heim_test::adjacency;

TEST_CASE("adjacency_list creates directed edges with expected endpoints", "[graph][adjacency_list][edges]")
{
  graph<true> g;reference_graph ref;
  auto a=add_vertex(g,ref),b=add_vertex(g,ref),c=add_vertex(g,ref);
  auto [e,ok]=add_edge(g,ref,a,b);REQUIRE(ok);
  REQUIRE(g.source(e)==a);REQUIRE(g.target(e)==b);
  REQUIRE(g.endpoints(e)==std::pair{a,b});
  add_edge(g,ref,b,c);
  verify(g,ref);
}
TEST_CASE("adjacency_list rejects parallel edges in simple undirected graph", "[graph][adjacency_list][edges]")
{
  graph<> g;reference_graph ref;
  auto a=add_vertex(g,ref),b=add_vertex(g,ref);
  add_edge(g,ref,a,b);
  add_edge(g,ref,b,a);
  add_edge(g,ref,a,b);
  REQUIRE(g.edge_count()==1);
  verify(g,ref);
}
TEST_CASE("adjacency_list allows reversed distinct edges in directed graph", "[graph][adjacency_list][edges]")
{
  graph<true> g;reference_graph ref;
  auto a=add_vertex(g,ref),b=add_vertex(g,ref);
  add_edge(g,ref,a,b);add_edge(g,ref,b,a);
  add_edge(g,ref,a,b);
  REQUIRE(g.edge_count()==2);
  verify(g,ref);
}
TEST_CASE("adjacency_list multigraph retains identical parallel edges", "[graph][adjacency_list][edges]")
{
  graph<false,true> g;reference_graph ref;
  auto a=add_vertex(g,ref),b=add_vertex(g,ref);
  for(unsigned i=0;i<9;++i) add_edge(g,ref,a,b);
  REQUIRE(g.edge_count()==9);
  verify(g,ref);
}
TEST_CASE("adjacency_list directed multigraph preserves opposite edge directions", "[graph][adjacency_list][edges]")
{
  graph<true,true> g;reference_graph ref;
  auto a=add_vertex(g,ref),b=add_vertex(g,ref);
  add_edge(g,ref,a,b);add_edge(g,ref,b,a);add_edge(g,ref,a,b);
  verify(g,ref);
}
TEST_CASE("adjacency_list simple graph permits one self-loop", "[graph][adjacency_list][edges]")
{
  graph<> g;reference_graph ref;auto v=add_vertex(g,ref);
  add_edge(g,ref,v,v);add_edge(g,ref,v,v);
  REQUIRE(g.edge_count()==1);
  REQUIRE(g.total_edge_count(v)==2);
  verify(g,ref);
}
TEST_CASE("adjacency_list multigraph permits multiple self-loops", "[graph][adjacency_list][edges]")
{
  graph<true,true> g;reference_graph ref;auto v=add_vertex(g,ref);
  for(int n=0;n<4;++n)add_edge(g,ref,v,v);
  REQUIRE(g.edge_count()==4);
  REQUIRE(g.total_edge_count(v)==8);
  verify(g,ref);
}
TEST_CASE("adjacency_list edges retain ids after unrelated removals", "[graph][adjacency_list][edges]")
{
  graph<> g;reference_graph ref;
  auto v=add_vertex(g,ref),w=add_vertex(g,ref),x=add_vertex(g,ref);
  auto [a,oa]=add_edge(g,ref,v,w);REQUIRE(oa);
  auto [b,ob]=add_edge(g,ref,w,x);REQUIRE(ob);
  g.remove_edge(a);ref.remove_edge(a.id());
  auto [c,oc]=add_edge(g,ref,x,v);REQUIRE(oc);
  REQUIRE(c.id()>b.id());
  verify(g,ref);
}
