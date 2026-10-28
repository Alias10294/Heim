#include "support.hpp"
using namespace heim_test::adjacency;

template<typename G>
void exercise_model(unsigned seed,unsigned operations)
{
  G g;reference_graph model;rng random{seed};
  for(unsigned step=0;step<operations;++step)
  {
    INFO("seed=" << seed << " step=" << step << " directed=" << G::is_directed
         << " multi=" << G::is_multigraph << " in=" << G::is_in_incident
         << " out=" << G::is_out_incident);
    std::size_t count=model.vertices.size();
    unsigned op=random.next()%100;
    if(count==0 || (op<20 && count<32))
      add_vertex(g,model);
    else if(op<58 && model.edges.size()<85)
    {
      auto u=select(model.vertices,random.bounded(count));
      auto v=select(model.vertices,random.bounded(count));
      add_edge(g,model,typename G::vertex_type{static_cast<typename G::id_type>(u)},
                       typename G::vertex_type{static_cast<typename G::id_type>(v)});
    }
    else if(op<69 && !model.edges.empty())
    {
      auto id=select(model.edges,random.bounded(model.edges.size()));
      // Only invoke remove_edge on a descriptor taken from the live graph.
      auto current_edges=g.edges();
      auto it=std::ranges::find_if(current_edges,[&](auto e){return e.id()==id;});
      REQUIRE(it!=std::ranges::end(current_edges));
      auto edge=*it;g.remove_edge(edge);model.remove_edge(id);
    }
    else if(op<79)
    {
      auto u=select(model.vertices,random.bounded(count));
      auto v=select(model.vertices,random.bounded(count));
      g.remove_edges(typename G::vertex_type{static_cast<typename G::id_type>(u)},
                     typename G::vertex_type{static_cast<typename G::id_type>(v)});
      model.remove_edges(u,v,G::is_directed);
    }
    else if(op<88)
    {
      auto v=select(model.vertices,random.bounded(count));
      g.clear_vertex(typename G::vertex_type{static_cast<typename G::id_type>(v)});
      model.clear_vertex(v);
    }
    else if(op<97)
    {
      auto v=select(model.vertices,random.bounded(count));
      g.remove_vertex(typename G::vertex_type{static_cast<typename G::id_type>(v)});
      model.remove_vertex(v);
    }
    else {g.clear();model.clear();}
    verify(g,model);
  }
}
TEST_CASE("adjacency_list model: undirected simple, reproducible sequences", "[graph][adjacency_list][model]")
{exercise_model<graph<>>(0x1a2b3c4du,450);exercise_model<graph<>>(0xabcdef23u,450);}
TEST_CASE("adjacency_list model: undirected multigraph, reproducible sequences", "[graph][adjacency_list][model]")
{exercise_model<graph<false,true>>(0x1a2b3c4du,450);exercise_model<graph<false,true>>(0xabcdef23u,450);}
TEST_CASE("adjacency_list model: directed simple, full incidence", "[graph][adjacency_list][model]")
{exercise_model<graph<true>>(0x1a2b3c4du,450);exercise_model<graph<true>>(0xabcdef23u,450);}
TEST_CASE("adjacency_list model: directed multigraph, full incidence", "[graph][adjacency_list][model]")
{exercise_model<graph<true,true>>(0x1a2b3c4du,450);exercise_model<graph<true,true>>(0xabcdef23u,450);}
TEST_CASE("adjacency_list model: directed simple, outgoing incidence", "[graph][adjacency_list][model]")
{exercise_model<graph<true,false,false,true>>(0x1a2b3c4du,450);exercise_model<graph<true,false,false,true>>(0xabcdef23u,450);}
TEST_CASE("adjacency_list model: directed multigraph, outgoing incidence", "[graph][adjacency_list][model]")
{exercise_model<graph<true,true,false,true>>(0x1a2b3c4du,450);exercise_model<graph<true,true,false,true>>(0xabcdef23u,450);}
TEST_CASE("adjacency_list model: directed simple, incoming incidence", "[graph][adjacency_list][model]")
{exercise_model<graph<true,false,true,false>>(0x1a2b3c4du,450);exercise_model<graph<true,false,true,false>>(0xabcdef23u,450);}
TEST_CASE("adjacency_list model: directed multigraph, incoming incidence", "[graph][adjacency_list][model]")
{exercise_model<graph<true,true,true,false>>(0x1a2b3c4du,450);exercise_model<graph<true,true,true,false>>(0xabcdef23u,450);}
