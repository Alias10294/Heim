#include <catch2/catch_test_macros.hpp>
#include <heim/graph.hpp>
#include <algorithm>
#include <ranges>
#include <unordered_set>
#include <vector>

namespace hg = heim::graphs;

TEST_CASE("adjacency_list has correct initial empty state", "[graph][adjacency_list]")
{
  heim::adjacency_list<> g;
  REQUIRE(hg::vertex_count(g) == 0);
  REQUIRE(hg::edge_count(g) == 0);
  REQUIRE(std::ranges::empty(hg::vertices(g)));
  REQUIRE(std::ranges::empty(hg::edges(g)));
}

TEST_CASE("adjacency_list creates stable, distinct vertex descriptors", "[graph][adjacency_list]")
{
  heim::adjacency_list<> g;
  auto a = hg::place_vertex(g);
  auto b = hg::place_vertex(g);
  auto c = hg::place_vertex(g);
  REQUIRE(a != b);
  REQUIRE(b != c);
  REQUIRE(hg::vertex_count(g) == 3);
  REQUIRE(std::ranges::distance(hg::vertices(g)) == 3);
  std::unordered_set<decltype(a)> ids;
  for (auto v : hg::vertices(g)) REQUIRE(ids.insert(v).second);
  REQUIRE(ids.contains(a));
  REQUIRE(ids.contains(b));
  REQUIRE(ids.contains(c));
}

TEST_CASE("undirected adjacency list exposes each edge from both endpoints", "[graph][adjacency_list]")
{
  heim::adjacency_list<> g;
  auto a = hg::place_vertex(g);
  auto b = hg::place_vertex(g);
  auto [e, placed] = hg::place_edge(g, a, b);
  REQUIRE(placed);
  REQUIRE(hg::edge_count(g) == 1);
  REQUIRE(hg::out_edge_count(g, a) == 1);
  REQUIRE(hg::out_edge_count(g, b) == 1);
  REQUIRE(hg::in_edge_count(g, a) == 1);
  REQUIRE(hg::in_edge_count(g, b) == 1);
  auto ends = hg::endpoints(g, e);
  REQUIRE(((ends.first == a && ends.second == b) || (ends.first == b && ends.second == a)));
  auto from_b = *hg::out_edges(g, b).begin();
  REQUIRE(hg::source(g, from_b) == b);
  REQUIRE(hg::target(g, from_b) == a);
  REQUIRE(from_b == e);
}

TEST_CASE("simple adjacency list rejects duplicate edges", "[graph][adjacency_list]")
{
  heim::adjacency_list<> undirected{{0u, 1u}};
  REQUIRE_FALSE(hg::place_edge(undirected, 0u, 1u).second);
  REQUIRE_FALSE(hg::place_edge(undirected, 1u, 0u).second);
  REQUIRE(hg::edge_count(undirected) == 1);
  heim::adjacency_list<true> directed{{0u, 1u}};
  REQUIRE_FALSE(hg::place_edge(directed, 0u, 1u).second);
  REQUIRE(hg::place_edge(directed, 1u, 0u).second);
  REQUIRE(hg::edge_count(directed) == 2);
}

TEST_CASE("multigraph retains and removes parallel edges", "[graph][adjacency_list]")
{
  heim::adjacency_list<false, true> g;
  auto a = hg::place_vertex(g);
  auto b = hg::place_vertex(g);
  auto e1 = hg::place_edge(g, a, b);
  auto e2 = hg::place_edge(g, a, b);
  auto e3 = hg::place_edge(g, b, a);
  REQUIRE(e1.second);
  REQUIRE(e2.second);
  REQUIRE(e3.second);
  REQUIRE(e1.first != e2.first);
  REQUIRE(hg::edge_count(g) == 3);
  REQUIRE(hg::out_edge_count(g, a) == 3);
  hg::remove_edge(g, e2.first);
  REQUIRE(hg::edge_count(g) == 2);
  hg::remove_edges(g, a, b);
  REQUIRE(hg::edge_count(g) == 0);
  REQUIRE(hg::out_edge_count(g, a) == 0);
  REQUIRE(hg::out_edge_count(g, b) == 0);
}

TEST_CASE("directed adjacency list respects incoming and outgoing degree", "[graph][adjacency_list]")
{
  heim::adjacency_list<true> g{{0u, 1u}, {0u, 2u}, {2u, 1u}};
  REQUIRE(hg::vertex_count(g) == 3);
  REQUIRE(hg::edge_count(g) == 3);
  REQUIRE(hg::out_edge_count(g, 0u) == 2);
  REQUIRE(hg::in_edge_count(g, 0u) == 0);
  REQUIRE(hg::in_edge_count(g, 1u) == 2);
  REQUIRE(hg::out_edge_count(g, 1u) == 0);
  REQUIRE(hg::out_edge_count(g, 2u) == 1);
  REQUIRE(hg::in_edge_count(g, 2u) == 1);
  hg::remove_edges(g, 0u, 1u);
  REQUIRE(hg::edge_count(g) == 2);
  REQUIRE(hg::in_edge_count(g, 1u) == 1);
  REQUIRE(hg::out_edge_count(g, 0u) == 1);
}

TEST_CASE("clear_vertex erases incident edges but retains vertex", "[graph][adjacency_list]")
{
  heim::adjacency_list<true> g{{0u, 1u}, {1u, 2u}, {2u, 0u}, {3u, 1u}};
  hg::clear_vertex(g, 1u);
  REQUIRE(hg::vertex_count(g) == 4);
  REQUIRE(hg::edge_count(g) == 1);
  REQUIRE(hg::in_edge_count(g, 1u) == 0);
  REQUIRE(hg::out_edge_count(g, 1u) == 0);
  REQUIRE(hg::out_edge_count(g, 2u) == 1);
}

TEST_CASE("remove_vertex erases all incident edges and preserves other descriptors", "[graph][adjacency_list]")
{
  heim::adjacency_list<> g{{0u, 1u}, {1u, 2u}, {2u, 3u}, {0u, 3u}};
  hg::remove_vertex(g, 1u);
  REQUIRE(hg::vertex_count(g) == 3);
  REQUIRE(hg::edge_count(g) == 2);
  std::unordered_set<heim::adjacency_list<>::vertex_type> remaining;
  for (auto v : hg::vertices(g)) remaining.insert(v);
  REQUIRE_FALSE(remaining.contains(1u));
  REQUIRE(remaining.contains(0u));
  REQUIRE(remaining.contains(2u));
  REQUIRE(remaining.contains(3u));
  REQUIRE(hg::out_edge_count(g, 2u) == 1);
  auto newer = hg::place_vertex(g);
  REQUIRE(newer.id() != 1u);
  REQUIRE(remaining.contains(3u));
}

TEST_CASE("copy and assignment create independent adjacency lists", "[graph][adjacency_list]")
{
  heim::adjacency_list<> g{{0u, 1u}, {1u, 2u}};
  auto copy = g;
  REQUIRE(copy == g);
  hg::remove_edges(copy, 0u, 1u);
  REQUIRE(hg::edge_count(copy) == 1);
  REQUIRE(hg::edge_count(g) == 2);
  auto assigned = g;
  assigned = {{2u, 3u}};
  REQUIRE(hg::edge_count(assigned) == 1);
  REQUIRE(hg::edge_count(g) == 2);
}
