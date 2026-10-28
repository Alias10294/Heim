#include "support.hpp"

#include <heim/graph/interface/primitives.hpp>
#include <heim/graph/interface/traits.hpp>

using namespace heim_test::adjacency;
namespace gh = heim::graphs;

TEST_CASE(
    "adjacency_list interoperates with graph CPO read operations",
    "[graph][adjacency_list][integration]")
{
  graph<true> g;
  auto a = gh::place_vertex(g), b = gh::place_vertex(g);
  auto [e, ok] = gh::place_edge(g, a, b);
  REQUIRE(ok);
  REQUIRE(gh::vertex_count(g) == 2);
  REQUIRE(gh::edge_count(g) == 1);
  REQUIRE(gh::order(g) == 2);
  REQUIRE(gh::size(g) == 1);
  REQUIRE(gh::source(g, e) == a);
  REQUIRE(gh::target(g, e) == b);
  REQUIRE(gh::endpoints(g, e) == std::pair{a, b});
  REQUIRE(gh::out_degree(g, a) == 1);
  REQUIRE(gh::in_degree(g, b) == 1);
  REQUIRE(gh::degree(g, a) == 1);
}

TEST_CASE(
    "adjacency_list interoperates with graph CPO mutation operations",
    "[graph][adjacency_list][integration]")
{
  graph<true> g;
  auto a = gh::place_vertex(g), b = gh::place_vertex(g), c = gh::place_vertex(g);
  auto [e, ok] = gh::place_edge(g, a, b);
  REQUIRE(ok);
  gh::place_edge(g, b, c);
  gh::remove_edge(g, e);
  REQUIRE(gh::edge_count(g) == 1);
  gh::remove_edges(g, b, c);
  REQUIRE(gh::edge_count(g) == 0);
  gh::place_edge(g, a, c);
  gh::clear_vertex(g, c);
  REQUIRE(gh::edge_count(g) == 0);
  gh::remove_vertex(g, a);
  REQUIRE(gh::vertex_count(g) == 2);
  check_consistency(g);
}

TEST_CASE("adjacency_list works with CPO ranges on undirected graphs", "[graph][adjacency_list][integration]")
{
  graph<> g{{{0, 1}, {1, 2}, {2, 0}}};
  auto v = graph<>::vertex_type{0u};
  REQUIRE(std::ranges::distance(gh::vertices(g)) == 3);
  REQUIRE(std::ranges::distance(gh::edges(g)) == 3);
  REQUIRE(std::ranges::distance(gh::out_edges(g, v)) == 2);
  REQUIRE(std::ranges::distance(gh::in_edges(g, v)) == 2);
  REQUIRE(gh::total_edge_count(g, v) == 2);
  check_consistency(g);
}

TEST_CASE(
    "adjacency_list works with graph traits directed and multigraph",
    "[graph][adjacency_list][integration]")
{
  STATIC_REQUIRE(gh::directed<graph<true>>);
  STATIC_REQUIRE(gh::multigraph<graph<false, true>>);
  STATIC_REQUIRE(!gh::directed<graph<>>);
  graph<true, true> g;
  auto v = make_vertices(g, 2);
  gh::place_edge(g, v[0], v[1]);
  gh::place_edge(g, v[0], v[1]);
  REQUIRE(gh::edge_count(g) == 2);
}
