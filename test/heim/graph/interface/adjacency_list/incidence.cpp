#include "support.hpp"

using namespace heim_test::adjacency;

template <typename G>
void check_three_vertices()
{
  G g;
  reference_graph ref;
  auto a = add_vertex(g, ref), b = add_vertex(g, ref), c = add_vertex(g, ref);
  add_edge(g, ref, a, b);
  add_edge(g, ref, b, c);
  add_edge(g, ref, a, a);
  verify(g, ref);
}

TEST_CASE("adjacency_list undirected in and out incidences agree", "[graph][adjacency_list][incidence]")
{
  check_three_vertices<graph<>>();
  check_three_vertices<graph<false, true>>();
}

TEST_CASE("adjacency_list directed full in and out incidences agree", "[graph][adjacency_list][incidence]")
{
  check_three_vertices<graph<true>>();
  check_three_vertices<graph<true, true>>();
}

TEST_CASE("adjacency_list directed outgoing-only incidences agree", "[graph][adjacency_list][incidence]")
{
  check_three_vertices<graph<true, false, false, true>>();
  check_three_vertices<graph<true, true, false, true>>();
}

TEST_CASE("adjacency_list directed incoming-only incidences agree", "[graph][adjacency_list][incidence]")
{
  check_three_vertices<graph<true, false, true, false>>();
  check_three_vertices<graph<true, true, true, false>>();
}

TEST_CASE("adjacency_list isolated vertices have zero degrees", "[graph][adjacency_list][incidence]")
{
  graph<true> g;
  auto vs = make_vertices(g, 3);
  for (auto v : vs)
  {
    REQUIRE(g.in_edge_count(v) == 0);
    REQUIRE(g.out_edge_count(v) == 0);
    REQUIRE(g.total_edge_count(v) == 0);
  }
}

TEST_CASE("adjacency_list directed total degree counts loops twice", "[graph][adjacency_list][incidence]")
{
  graph<true> g;
  auto vs = make_vertices(g, 2);
  g.place_edge(vs[0], vs[0]);
  g.place_edge(vs[0], vs[1]);
  g.place_edge(vs[1], vs[0]);
  REQUIRE(g.out_edge_count(vs[0]) == 2);
  REQUIRE(g.in_edge_count(vs[0]) == 2);
  REQUIRE(g.total_edge_count(vs[0]) == 4);
  check_consistency(g);
}

TEST_CASE(
    "adjacency_list undirected incidence edges reorient around queried vertex",
    "[graph][adjacency_list][incidence]")
{
  graph<> g;
  auto vs = make_vertices(g, 2);
  auto [e, ok] = g.place_edge(vs[0], vs[1]);
  REQUIRE(ok);
  REQUIRE(g.source(*g.out_edges(vs[0]).begin()) == vs[0]);
  REQUIRE(g.target(*g.out_edges(vs[0]).begin()) == vs[1]);
  REQUIRE(g.source(*g.out_edges(vs[1]).begin()) == vs[1]);
  REQUIRE(g.target(*g.out_edges(vs[1]).begin()) == vs[0]);
  REQUIRE(g.endpoints(e) == std::pair{vs[0], vs[1]});
}

TEST_CASE("adjacency_list incidence ranges on const graphs are usable", "[graph][adjacency_list][incidence]")
{
  graph<true> g{{{0, 1}, {2, 1}, {1, 3}}};
  graph<true> const &cg = g;
  auto v = graph<true>::vertex_type{1u};
  REQUIRE(std::ranges::distance(cg.in_edges(v)) == 2);
  REQUIRE(std::ranges::distance(cg.out_edges(v)) == 1);
}
