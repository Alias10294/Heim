#include "support.hpp"
using namespace heim_test::adjacency;

template<typename G>
void enumerate_graphs(unsigned vertex_count)
{
  std::vector<std::pair<unsigned,unsigned>> candidates;
  for(unsigned a=0;a<vertex_count;++a)
    for(unsigned b=0;b<vertex_count;++b)
      if(G::is_directed || a<=b)candidates.emplace_back(a,b);
  std::size_t all=std::size_t{1}<<candidates.size();
  for(std::size_t mask=0;mask<all;++mask)
  {
    INFO("mask=" << mask << " directed=" << G::is_directed
         << " multi=" << G::is_multigraph);
    G g;reference_graph model;
    std::vector<typename G::vertex_type> vertices;
    for(unsigned v=0;v<vertex_count;++v)vertices.push_back(add_vertex(g,model));
    for(std::size_t bit=0;bit<candidates.size();++bit)
      if(mask & (std::size_t{1}<<bit))
      {
        auto [u,v]=candidates[bit];
        add_edge(g,model,vertices[u],vertices[v]);
        if constexpr (G::is_multigraph)
          if((bit+mask)%3==0)add_edge(g,model,vertices[u],vertices[v]);
      }
    verify(g,model);
    // Every graph must also survive the removal of each vertex in an independent copy.
    for(unsigned removed=0;removed<vertex_count;++removed)
    {
      auto copy=g,expected=model;
      copy.remove_vertex(vertices[removed]);expected.remove_vertex(removed);
      verify(copy,expected);
    }
  }
}
TEST_CASE("adjacency_list exhaustive: all undirected simple graphs on 3 vertices", "[graph][adjacency_list][exhaustive]")
{enumerate_graphs<graph<>>(3);}
TEST_CASE("adjacency_list exhaustive: all undirected multigraph patterns on 3 vertices", "[graph][adjacency_list][exhaustive]")
{enumerate_graphs<graph<false,true>>(3);}
TEST_CASE("adjacency_list exhaustive: all directed simple graphs on 3 vertices", "[graph][adjacency_list][exhaustive]")
{enumerate_graphs<graph<true>>(3);}
TEST_CASE("adjacency_list exhaustive: all directed multigraph patterns on 3 vertices", "[graph][adjacency_list][exhaustive]")
{enumerate_graphs<graph<true,true>>(3);}
TEST_CASE("adjacency_list exhaustive: outgoing-only directed simple graphs on 3 vertices", "[graph][adjacency_list][exhaustive]")
{enumerate_graphs<graph<true,false,false,true>>(3);}
TEST_CASE("adjacency_list exhaustive: incoming-only directed simple graphs on 3 vertices", "[graph][adjacency_list][exhaustive]")
{enumerate_graphs<graph<true,false,true,false>>(3);}
TEST_CASE("adjacency_list exhaustive: outgoing-only directed multigraphs on 3 vertices", "[graph][adjacency_list][exhaustive]")
{enumerate_graphs<graph<true,true,false,true>>(3);}
TEST_CASE("adjacency_list exhaustive: incoming-only directed multigraphs on 3 vertices", "[graph][adjacency_list][exhaustive]")
{enumerate_graphs<graph<true,true,true,false>>(3);}
