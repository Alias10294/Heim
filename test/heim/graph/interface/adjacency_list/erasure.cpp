#include "support.hpp"

using namespace heim_test::adjacency;

template <typename G>
void erase_scenario()
{
  G g;
  reference_graph ref;
  auto v = add_vertex(g, ref), w = add_vertex(g, ref), x = add_vertex(g, ref), y = add_vertex(g, ref);
  auto e0 = add_edge(g, ref, v, w).first;
  add_edge(g, ref, v, w);
  add_edge(g, ref, w, v);
  add_edge(g, ref, w, w);
  add_edge(g, ref, w, x);
  add_edge(g, ref, y, w);
  add_edge(g, ref, x, y);
  verify(g, ref);
  g.remove_edge(e0);
  ref.remove_edge(e0.id());
  verify(g, ref);
  g.remove_edges(v, w);
  ref.remove_edges(v.id(), w.id(), G::is_directed);
  verify(g, ref);
  g.clear_vertex(w);
  ref.clear_vertex(w.id());
  verify(g, ref);
  g.remove_vertex(x);
  ref.remove_vertex(x.id());
  verify(g, ref);
  add_edge(g, ref, v, y);
  verify(g, ref);
  g.clear();
  ref.clear();
  verify(g, ref);
}

TEST_CASE("adjacency_list erase scenarios: undirected simple", "[graph][adjacency_list][erasure]")
{
  erase_scenario<graph<>>();
}

TEST_CASE("adjacency_list erase scenarios: undirected multigraph", "[graph][adjacency_list][erasure]")
{
  erase_scenario<graph<false, true>>();
}

TEST_CASE("adjacency_list erase scenarios: directed full simple", "[graph][adjacency_list][erasure]")
{
  erase_scenario<graph<true>>();
}

TEST_CASE("adjacency_list erase scenarios: directed full multigraph", "[graph][adjacency_list][erasure]")
{
  erase_scenario<graph<true, true>>();
}

TEST_CASE("adjacency_list erase scenarios: directed outgoing only simple", "[graph][adjacency_list][erasure]")
{
  erase_scenario<graph<true, false, false, true>>();
}

TEST_CASE("adjacency_list erase scenarios: directed incoming only simple", "[graph][adjacency_list][erasure]")
{
  erase_scenario<graph<true, false, true, false>>();
}

TEST_CASE(
    "adjacency_list erase scenarios: directed outgoing only multigraph",
    "[graph][adjacency_list][erasure]")
{
  erase_scenario<graph<true, true, false, true>>();
}

TEST_CASE(
    "adjacency_list erase scenarios: directed incoming only multigraph",
    "[graph][adjacency_list][erasure]")
{
  erase_scenario<graph<true, true, true, false>>();
}

TEST_CASE(
    "adjacency_list remove_edges on absent pair does not modify graph",
    "[graph][adjacency_list][erasure]")
{
  graph<true> g;
  auto v = make_vertices(g, 3);
  g.place_edge(v[0], v[1]);
  auto saved = g;
  g.remove_edges(v[1], v[2]);
  REQUIRE(g == saved);
  check_consistency(g);
}

TEST_CASE("adjacency_list clear_vertex preserves vertex identity", "[graph][adjacency_list][erasure]")
{
  graph<> g;
  reference_graph ref;
  auto v = add_vertex(g, ref), w = add_vertex(g, ref);
  add_edge(g, ref, v, w);
  add_edge(g, ref, v, v);
  g.clear_vertex(v);
  ref.clear_vertex(v.id());
  REQUIRE(g.vertex_count() == 2);
  verify(g, ref);
}

TEST_CASE("adjacency_list remove_vertex on isolated vertex is local", "[graph][adjacency_list][erasure]")
{
  graph<true> g;
  reference_graph ref;
  auto v = add_vertex(g, ref), w = add_vertex(g, ref), x = add_vertex(g, ref);
  add_edge(g, ref, v, w);
  g.remove_vertex(x);
  ref.remove_vertex(x.id());
  verify(g, ref);
}

TEST_CASE("adjacency_list remove_edges respects directed orientation", "[graph][adjacency_list][erasure]")
{
  graph<true, true> g;
  reference_graph ref;
  auto v = add_vertex(g, ref), w = add_vertex(g, ref);
  add_edge(g, ref, v, w);
  add_edge(g, ref, w, v);
  add_edge(g, ref, v, w);
  g.remove_edges(v, w);
  ref.remove_edges(v.id(), w.id(), true);
  REQUIRE(g.edge_count() == 1);
  verify(g, ref);
}

TEST_CASE("adjacency_list remove_edges removes all undirected parallels", "[graph][adjacency_list][erasure]")
{
  graph<false, true> g;
  reference_graph ref;
  auto v = add_vertex(g, ref), w = add_vertex(g, ref);
  add_edge(g, ref, v, w);
  add_edge(g, ref, w, v);
  add_edge(g, ref, v, w);
  g.remove_edges(v, w);
  ref.remove_edges(v.id(), w.id(), false);
  REQUIRE(g.edge_count() == 0);
  verify(g, ref);
}

TEST_CASE(
    "adjacency_list remove_edges on self loops clears all duplicate incidences",
    "[graph][adjacency_list][erasure]")
{
  graph<false, true> g;
  reference_graph ref;
  auto v = add_vertex(g, ref), w = add_vertex(g, ref);
  for (int i = 0; i < 5; ++i)
    add_edge(g, ref, v, v);
  add_edge(g, ref, v, w);
  g.remove_edges(v, v);
  ref.remove_edges(v.id(), v.id(), false);
  REQUIRE(g.edge_count() == 1);
  verify(g, ref);
}

TEST_CASE(
    "adjacency_list erase and reinsert preserves unaffected edge ids",
    "[graph][adjacency_list][erasure]")
{
  graph<> g;
  reference_graph ref;
  auto a = add_vertex(g, ref), b = add_vertex(g, ref), c = add_vertex(g, ref);
  auto [first, ok] = add_edge(g, ref, a, b);
  REQUIRE(ok);
  auto [second, ok2] = add_edge(g, ref, b, c);
  REQUIRE(ok2);
  g.remove_edge(first);
  ref.remove_edge(first.id());
  auto [third, ok3] = add_edge(g, ref, a, c);
  REQUIRE(ok3);
  REQUIRE(third.id() > second.id());
  verify(g, ref);
}
