#include <catch2/catch_test_macros.hpp>
#include <heim/graph.hpp>
#include <algorithm>
#include <cstdint>
#include <random>
#include <set>
#include <utility>

namespace hg = heim::graphs;
namespace {
using pair = std::pair<unsigned, unsigned>;

pair normalize(unsigned u, unsigned v, bool directed)
{
  return (!directed && v < u) ? pair{v, u} : pair{u, v};
}

template<bool Directed>
void check_graph(heim::adjacency_list<Directed> const& g, std::set<pair> const& reference,
                 unsigned n)
{
  REQUIRE(hg::vertex_count(g) == n);
  REQUIRE(hg::edge_count(g) == reference.size());
  std::set<pair> actual;
  for (auto e : hg::edges(g)) {
    auto [u, v] = hg::endpoints(g, e);
    REQUIRE(actual.insert(normalize(u.id(), v.id(), Directed)).second);
  }
  REQUIRE(actual == reference);
  for (unsigned u = 0; u < n; ++u) {
    std::size_t out = 0, in = 0;
    for (auto [a, b] : reference) {
      if constexpr (Directed) { out += a == u; in += b == u; }
      else { out += (a == u) + (b == u); in += (a == u) + (b == u); }
    }
    REQUIRE(hg::out_edge_count(g, u) == out);
    REQUIRE(hg::in_edge_count(g, u) == in);
  }
}

template<bool Directed>
void exercise(std::uint32_t seed)
{
  heim::adjacency_list<Directed> g;
  constexpr unsigned n = 18;
  for (unsigned i = 0; i < n; ++i) hg::place_vertex(g);
  std::set<pair> reference;
  std::mt19937 rng{seed};
  for (unsigned step = 0; step < 800; ++step) {
    unsigned u = rng() % n;
    unsigned v = rng() % n;
    if (u == v) continue; // Self-loop behavior is tested separately.
    pair edge = normalize(u, v, Directed);
    if (rng() % 3 == 0) {
      hg::remove_edges(g, u, v);
      reference.erase(edge);
    } else {
      bool inserted = reference.insert(edge).second;
      REQUIRE(hg::place_edge(g, u, v).second == inserted);
    }
    if (step % 47 == 0) check_graph(g, reference, n);
  }
  check_graph(g, reference, n);
}
}

TEST_CASE("undirected adjacency_list agrees with independent edge-set model", "[graph][adjacency_list][model]")
{
  exercise<false>(0xA17E1u);
}

TEST_CASE("directed adjacency_list agrees with independent edge-set model", "[graph][adjacency_list][model]")
{
  exercise<true>(0xD173Cu);
}
