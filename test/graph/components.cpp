#include <catch2/catch_test_macros.hpp>
#include <heim/graph.hpp>
#include <algorithm>
#include <numeric>
#include <random>
#include <vector>
#include <cstddef>

namespace hg = heim::graphs;

TEST_CASE("empty graph has zero connected components", "[graph][components]")
{
  heim::adjacency_list<> g;
  auto r = hg::connected_components(g);
  REQUIRE(r.component_count() == 0);
  REQUIRE(r.connected());
}

TEST_CASE("isolated vertices have distinct component IDs", "[graph][components]")
{
  heim::adjacency_list<> g;
  for (int i = 0; i < 5; ++i) hg::place_vertex(g);
  auto r = hg::cc(g);
  REQUIRE(r.component_count() == 5);
  REQUIRE_FALSE(r.connected());
  for (unsigned i = 0; i < 5; ++i)
    for (unsigned j = 0; j < 5; ++j)
      REQUIRE(r.connected(i, j) == (i == j));
}

TEST_CASE("connected components partition graph correctly", "[graph][components]")
{
  heim::adjacency_list<> g{{0u, 1u}, {1u, 2u}, {3u, 4u}};
  hg::place_vertex(g); // isolated vertex 5
  auto r = hg::cc(g);
  REQUIRE(r.component_count() == 3);
  REQUIRE(r.connected(0u, 2u));
  REQUIRE(r.connected(3u, 4u));
  REQUIRE_FALSE(r.connected(0u, 4u));
  REQUIRE_FALSE(r.connected(4u, 5u));
  REQUIRE(r.component_id_of(0u) == r.component_id_of(2u));
  REQUIRE(r.component_id_of(3u) == r.component_id_of(4u));
  REQUIRE(r.component_id_of(0u) != r.component_id_of(3u));
}

TEST_CASE("edge deletion changes component structure", "[graph][components]")
{
  heim::adjacency_list<> g{{0u, 1u}, {1u, 2u}, {2u, 3u}};
  REQUIRE(hg::cc(g).connected());
  hg::remove_edges(g, 1u, 2u);
  auto r = hg::cc(g);
  REQUIRE(r.component_count() == 2);
  REQUIRE(r.connected(0u, 1u));
  REQUIRE(r.connected(2u, 3u));
  REQUIRE_FALSE(r.connected(0u, 3u));
}

TEST_CASE("connected components matches independent union-find model", "[graph][components][model]")
{
  constexpr unsigned n = 42;
  heim::adjacency_list<> g;
  for (unsigned i = 0; i < n; ++i) hg::place_vertex(g);
  std::vector<unsigned> parent(n);
  std::iota(parent.begin(), parent.end(), 0u);
  auto root = [&](unsigned x) {
    while (parent[x] != x) x = parent[x];
    return x;
  };
  std::mt19937 rng{0xCC27u};
  for (unsigned i = 0; i < 75; ++i) {
    unsigned u = rng() % n, v = rng() % n;
    if (u == v) continue;
    hg::place_edge(g, u, v);
    parent[root(u)] = root(v);
  }
  auto r = hg::cc(g);
  std::size_t count = 0;
  for (unsigned v = 0; v < n; ++v) if (root(v) == v) ++count;
  REQUIRE(r.component_count() == count);
  for (unsigned u = 0; u < n; ++u)
    for (unsigned v = 0; v < n; ++v)
      REQUIRE(r.connected(u, v) == (root(u) == root(v)));
}
