#include "support.hpp"

using namespace heim_test::adjacency;

TEST_CASE(
    "adjacency_list directed outgoing-only removal cleans incoming references",
    "[graph][adjacency_list][regression]")
{
  graph<true, false, false, true> g;
  reference_graph ref;
  auto v = add_vertex(g, ref), w = add_vertex(g, ref), x = add_vertex(g, ref);
  add_edge(g, ref, w, v);
  add_edge(g, ref, x, v);
  add_edge(g, ref, v, v);
  g.remove_vertex(v);
  ref.remove_vertex(v.id());
  verify(g, ref);
}

TEST_CASE(
    "adjacency_list directed incoming-only removal cleans outgoing references",
    "[graph][adjacency_list][regression]")
{
  graph<true, false, true, false> g;
  reference_graph ref;
  auto v = add_vertex(g, ref), w = add_vertex(g, ref), x = add_vertex(g, ref);
  add_edge(g, ref, v, w);
  add_edge(g, ref, v, x);
  add_edge(g, ref, v, v);
  g.remove_vertex(v);
  ref.remove_vertex(v.id());
  verify(g, ref);
}

TEST_CASE(
    "adjacency_list undirected removing one self-loop does not leave incidence",
    "[graph][adjacency_list][regression]")
{
  graph<false, true> g;
  reference_graph ref;
  auto v = add_vertex(g, ref);
  auto [loop, ok] = add_edge(g, ref, v, v);
  REQUIRE(ok);
  g.remove_edge(loop);
  ref.remove_edge(loop.id());
  verify(g, ref);
  REQUIRE(g.total_edge_count(v) == 0);
}

TEST_CASE(
    "adjacency_list undirected remove_edges(u,u) leaves unrelated edges",
    "[graph][adjacency_list][regression]")
{
  graph<false, true> g;
  reference_graph ref;
  auto u = add_vertex(g, ref), v = add_vertex(g, ref);
  for (int i = 0; i < 4; ++i)
    add_edge(g, ref, u, u);
  auto keep = add_edge(g, ref, u, v).first;
  g.remove_edges(u, u);
  ref.remove_edges(u.id(), u.id(), false);
  verify(g, ref);
  REQUIRE(g.source(keep) == u);
}

TEST_CASE(
    "adjacency_list duplicate insertion after removing edge works",
    "[graph][adjacency_list][regression]")
{
  graph<> g;
  reference_graph ref;
  auto u = add_vertex(g, ref), v = add_vertex(g, ref);
  auto e = add_edge(g, ref, u, v).first;
  g.remove_edge(e);
  ref.remove_edge(e.id());
  add_edge(g, ref, u, v);
  verify(g, ref);
}

TEST_CASE(
    "adjacency_list duplicate detection survives swap-with-last erasure",
    "[graph][adjacency_list][regression]")
{
  graph<> g;
  reference_graph ref;
  auto u = add_vertex(g, ref), v = add_vertex(g, ref), w = add_vertex(g, ref);
  auto e = add_edge(g, ref, u, v).first;
  add_edge(g, ref, u, w);
  g.remove_edge(e);
  ref.remove_edge(e.id());
  add_edge(g, ref, u, v);
  verify(g, ref);
  REQUIRE(g.edge_count() == 2);
}

TEST_CASE(
    "adjacency_list repeated interleaved insertion deletion across sparse pages",
    "[graph][adjacency_list][regression]")
{
  graph<true, true, true, true, unsigned, 2> g;
  reference_graph ref;
  std::vector<graph<true, true, true, true, unsigned, 2>::vertex_type> v;
  for (unsigned i = 0; i < 25; ++i)
    v.push_back(add_vertex(g, ref));
  for (unsigned i = 0; i < 25; ++i)
    add_edge(g, ref, v[i], v[(i + 1) % 25]);
  for (unsigned i = 0; i < 25; i += 2)
  {
    g.clear_vertex(v[i]);
    ref.clear_vertex(v[i].id());
  }
  verify(g, ref);
  for (unsigned i = 1; i < 25; i += 3)
  {
    g.remove_vertex(v[i]);
    ref.remove_vertex(v[i].id());
  }
  verify(g, ref);
}

TEST_CASE(
    "adjacency_list undirected reverse duplicate detection after removing parallel edges",
    "[graph][adjacency_list][regression]")
{
  graph<> g;
  reference_graph ref;
  auto u = add_vertex(g, ref), v = add_vertex(g, ref);
  add_edge(g, ref, u, v);
  add_edge(g, ref, v, u);
  g.remove_edges(u, v);
  ref.remove_edges(u.id(), v.id(), false);
  add_edge(g, ref, v, u);
  verify(g, ref);
}
