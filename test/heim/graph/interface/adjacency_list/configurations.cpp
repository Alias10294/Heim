#include "support.hpp"
using namespace heim_test::adjacency;

template<typename G>
void configuration_stress()
{
  G g;reference_graph ref;
  auto v=add_vertex(g,ref),w=add_vertex(g,ref),x=add_vertex(g,ref),y=add_vertex(g,ref);
  add_edge(g,ref,v,w);add_edge(g,ref,w,v);
  add_edge(g,ref,v,v);add_edge(g,ref,v,v);
  add_edge(g,ref,w,x);add_edge(g,ref,y,v);
  add_edge(g,ref,x,y);add_edge(g,ref,x,w);
  verify(g,ref);
  g.remove_vertex(v);ref.remove_vertex(v.id());verify(g,ref);
  add_edge(g,ref,w,y);verify(g,ref);
  g.clear_vertex(x);ref.clear_vertex(x.id());verify(g,ref);
  g.remove_edges(w,y);ref.remove_edges(w.id(),y.id(),G::is_directed);verify(g,ref);
}
TEST_CASE("adjacency_list configuration: undirected simple", "[graph][adjacency_list][configuration]")
{ configuration_stress<graph<false,false,true,true>>(); }
TEST_CASE("adjacency_list configuration: undirected multigraph", "[graph][adjacency_list][configuration]")
{ configuration_stress<graph<false,true,true,true>>(); }
TEST_CASE("adjacency_list configuration: directed simple full", "[graph][adjacency_list][configuration]")
{ configuration_stress<graph<true,false,true,true>>(); }
TEST_CASE("adjacency_list configuration: directed multigraph full", "[graph][adjacency_list][configuration]")
{ configuration_stress<graph<true,true,true,true>>(); }
TEST_CASE("adjacency_list configuration: directed simple outgoing", "[graph][adjacency_list][configuration]")
{ configuration_stress<graph<true,false,false,true>>(); }
TEST_CASE("adjacency_list configuration: directed multigraph outgoing", "[graph][adjacency_list][configuration]")
{ configuration_stress<graph<true,true,false,true>>(); }
TEST_CASE("adjacency_list configuration: directed simple incoming", "[graph][adjacency_list][configuration]")
{ configuration_stress<graph<true,false,true,false>>(); }
TEST_CASE("adjacency_list configuration: directed multigraph incoming", "[graph][adjacency_list][configuration]")
{ configuration_stress<graph<true,true,true,false>>(); }
