#include <catch2/catch_test_macros.hpp>
#include <heim/graph.hpp>
#include <heim/property.hpp>
#include "test_utils.hpp"
#include <algorithm>
#include <array>
#include <limits>
#include <random>
#include <unordered_map>
#include <vector>

namespace hg = heim::graphs;
namespace {
using graph = heim::adjacency_list<true>;
using edge = graph::edge_type;
using vertex = graph::vertex_type;
using weights = std::unordered_map<edge, unsigned>;

void add(graph& g, weights& w, unsigned u, unsigned v, unsigned weight)
{
  auto [e, inserted] = hg::place_edge(g, u, v);
  if (inserted) w.emplace(e, weight);
}
}

TEST_CASE("Dijkstra prefers cheaper multi-edge route over direct edge", "[graph][dijkstra]")
{
  graph g;
  for (unsigned i = 0; i < 5; ++i) hg::place_vertex(g);
  weights w;
  add(g, w, 0, 1, 2);
  add(g, w, 1, 2, 3);
  add(g, w, 0, 2, 20);
  add(g, w, 2, 3, 1);
  add(g, w, 1, 3, 15);
  auto r = hg::dijkstra(g, 0u, heim::make_readable_property_map(w));
  REQUIRE(r.reached(0u));
  REQUIRE(r.reached(3u));
  REQUIRE_FALSE(r.reached(4u));
  REQUIRE(r.distance_of(0u) == 0);
  REQUIRE(r.distance_of(1u) == 2);
  REQUIRE(r.distance_of(2u) == 5);
  REQUIRE(r.distance_of(3u) == 6);
  REQUIRE(heim_test::collect(r.path_from(3u)) == (std::vector<vertex>{3u, 2u, 1u, 0u}));
}

TEST_CASE("Dijkstra to target reports shortest distance and reverse path", "[graph][dijkstra_to]")
{
  graph g;
  for (unsigned i = 0; i < 4; ++i) hg::place_vertex(g);
  weights w;
  add(g, w, 0, 1, 2);
  add(g, w, 1, 2, 2);
  add(g, w, 0, 2, 10);
  add(g, w, 2, 3, 1);
  auto r = hg::dijkstra_to(g, 0u, 3u, heim::make_readable_property_map(w));
  REQUIRE(r.reached());
  REQUIRE(r.distance() == 5);
  REQUIRE(heim_test::collect(r.path()) == (std::vector<vertex>{3u, 2u, 1u, 0u}));
}

TEST_CASE("Dijkstra to unreachable target reports no path", "[graph][dijkstra_to]")
{
  graph g{{0u, 1u}, {2u, 3u}};
  weights w;
  for (auto e : hg::edges(g)) w.emplace(e, 1u);
  auto r = hg::dijkstra_to(g, 0u, 3u, heim::make_readable_property_map(w));
  REQUIRE_FALSE(r.reached());
}

TEST_CASE("Dijkstra supports zero-weight edges", "[graph][dijkstra]")
{
  graph g;
  for (unsigned i = 0; i < 3; ++i) hg::place_vertex(g);
  weights w;
  add(g, w, 0, 1, 0);
  add(g, w, 1, 2, 0);
  add(g, w, 0, 2, 10);
  auto r = hg::dijkstra(g, 0u, heim::make_readable_property_map(w));
  REQUIRE(r.distance_of(2u) == 0);
  REQUIRE(heim_test::collect(r.path_from(2u)) == (std::vector<vertex>{2u, 1u, 0u}));
}

TEST_CASE("Dijkstra distances match independent Bellman-Ford relaxation", "[graph][dijkstra][model]")
{
  constexpr unsigned n = 14;
  constexpr unsigned inf = std::numeric_limits<unsigned>::max();
  graph g;
  for (unsigned i = 0; i < n; ++i) hg::place_vertex(g);
  weights w;
  std::vector<std::array<unsigned, 3>> arcs;
  std::mt19937 rng{0xD1A257u};
  for (unsigned u = 0; u < n; ++u)
    for (unsigned v = 0; v < n; ++v)
      if (u != v && (rng() % 5) == 0) {
        unsigned cost = 1 + rng() % 20;
        add(g, w, u, v, cost);
        arcs.push_back({u, v, cost});
      }
  auto weight_map = heim::make_readable_property_map(w);
  for (unsigned source : {0u, 3u, 8u}) {
    std::vector<unsigned> expected(n, inf);
    expected[source] = 0;
    for (unsigned pass = 1; pass < n; ++pass)
      for (auto [u, v, c] : arcs)
        if (expected[u] != inf)
          expected[v] = std::min(expected[v], expected[u] + c);

    auto result = hg::dijkstra(g, vertex{source}, weight_map);
    for (unsigned target = 0; target < n; ++target) {
      INFO("source=" << source << ", target=" << target);
      if (expected[target] == inf) {
        REQUIRE_FALSE(result.reached(vertex{target}));
      } else {
        REQUIRE(result.reached(vertex{target}));
        REQUIRE(result.distance_of(vertex{target}) == expected[target]);
        REQUIRE(heim_test::is_reverse_path(g, result.path_from(vertex{target}),
                                            vertex{source}, vertex{target}));
      }
    }
  }
}
