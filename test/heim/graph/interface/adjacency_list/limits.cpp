#include "support.hpp"

#include <cstdint>

using namespace heim_test::adjacency;

TEST_CASE("adjacency_list supports small sparse pages", "[graph][adjacency_list][limit]")
{
  graph<false, true, true, true, unsigned, 2> g;
  reference_graph ref;
  std::vector<decltype(g)::vertex_type> vs;
  for (int i = 0; i < 30; ++i)
    vs.push_back(add_vertex(g, ref));
  for (int i = 0; i < 29; ++i)
    add_edge(g, ref, vs[i], vs[i + 1]);
  verify(g, ref);
}

TEST_CASE("adjacency_list supports large sparse pages", "[graph][adjacency_list][limit]")
{
  graph<true, false, true, true, unsigned, 64> g;
  reference_graph ref;
  std::vector<decltype(g)::vertex_type> vs;
  for (int i = 0; i < 90; ++i)
    vs.push_back(add_vertex(g, ref));
  for (int i = 1; i < 90; ++i)
    add_edge(g, ref, vs[0], vs[i]);
  verify(g, ref);
}

TEST_CASE("adjacency_list supports uint16_t identifiers within bounds", "[graph][adjacency_list][limit]")
{
  graph<true, true, true, false, std::uint16_t, 8> g;
  reference_graph ref;
  std::vector<decltype(g)::vertex_type> vs;
  for (int i = 0; i < 25; ++i)
    vs.push_back(add_vertex(g, ref));
  for (int i = 0; i < 24; ++i)
    add_edge(g, ref, vs[i], vs[i + 1]);
  verify(g, ref);
}

TEST_CASE("adjacency_list clear on an already empty graph is harmless", "[graph][adjacency_list][limit]")
{
  graph<> g;
  g.clear();
  g.clear();
  verify_empty(g);
}

TEST_CASE(
    "adjacency_list high-degree vertex deletion removes all references",
    "[graph][adjacency_list][limit]")
{
  graph<true, true> g;
  reference_graph ref;
  auto root = add_vertex(g, ref);
  for (unsigned i = 0; i < 90; ++i)
  {
    auto v = add_vertex(g, ref);
    add_edge(g, ref, root, v);
    add_edge(g, ref, v, root);
  }
  verify(g, ref);
  g.remove_vertex(root);
  ref.remove_vertex(root.id());
  verify(g, ref);
}

TEST_CASE(
    "adjacency_list clear on populated graph removes all edges and vertices",
    "[graph][adjacency_list][limit]")
{
  graph<false, true> g;
  reference_graph ref;
  auto vs = std::vector<graph<false, true>::vertex_type>{};
  for (int i = 0; i < 10; ++i)
    vs.push_back(add_vertex(g, ref));
  for (int i = 0; i < 10; ++i)
    add_edge(g, ref, vs[i], vs[(i + 1) % 10]);
  g.clear();
  ref.clear();
  verify(g, ref);
  add_vertex(g, ref);
  verify(g, ref);
}
