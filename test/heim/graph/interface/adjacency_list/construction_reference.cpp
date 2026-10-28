#include "support.hpp"

#include <algorithm>
#include <array>
#include <forward_list>
#include <map>
#include <ranges>
#include <sstream>
#include <utility>
#include <vector>

using namespace heim_test::adjacency;

namespace
{
using pair_type = std::pair<unsigned, unsigned>;

// This oracle deliberately reads the *input edge sequence*, never the graph.
// It does not depend on an edge ID, incidence order, or internal storage.
inline pair_type canonical(pair_type edge, bool directed)
{
  if (!directed && edge.second < edge.first)
    std::swap(edge.first, edge.second);
  return edge;
}

template <typename G>
void require_constructed_edges(G const &g, std::vector<pair_type> const &input)
{
  std::set<unsigned> expected_vertices;
  std::map<pair_type, unsigned> expected_edges;
  for (auto [source, target] : input)
  {
    expected_vertices.insert(source);
    expected_vertices.insert(target);
    pair_type edge = canonical({source, target}, G::is_directed);
    if constexpr (G::is_multigraph)
      ++expected_edges[edge];
    else
      expected_edges[edge] = 1;
  }

  std::set<unsigned> observed_vertices;
  for (auto vertex : g.vertices())
    REQUIRE(observed_vertices.insert(static_cast<unsigned>(vertex.id())).second);
  REQUIRE(observed_vertices == expected_vertices);
  REQUIRE(g.vertex_count() == expected_vertices.size());

  std::map<pair_type, unsigned> observed_edges;
  for (auto edge : g.edges())
  {
    auto [source, target] = g.endpoints(edge);
    REQUIRE(expected_vertices.contains(static_cast<unsigned>(source.id())));
    REQUIRE(expected_vertices.contains(static_cast<unsigned>(target.id())));
    auto normalized =
        canonical({static_cast<unsigned>(source.id()), static_cast<unsigned>(target.id())}, G::is_directed);
    ++observed_edges[normalized];
  }
  REQUIRE(observed_edges == expected_edges);
  unsigned expected_count{};
  for (auto const &[edge, count] : expected_edges)
  {
    (void)edge;
    expected_count += count;
  }
  REQUIRE(g.edge_count() == expected_count);

  // In addition to the independent edge oracle, check internal incidences.
  check_consistency(g);
}

struct stream_edge
{
  unsigned first{};
  unsigned second{};
};
std::istream &operator>>(std::istream &in, stream_edge &edge)
{
  return in >> edge.first >> edge.second;
}
}

TEST_CASE(
    "adjacency_list iterator construction preserves exact directed edge sequence",
    "[graph][adjacency_list][construction][oracle]")
{
  std::vector<pair_type> const input{{8, 1}, {1, 8}, {8, 8}, {2, 9}, {8, 1}};
  graph<true> g{input.begin(), input.end()};
  require_constructed_edges(g, input);
  REQUIRE(g.place_vertex().id() == 10u);
  REQUIRE(g.edge_count() == 4); // directed simple graph: one duplicate
}

TEST_CASE(
    "adjacency_list from_range construction preserves undirected edge multiplicity",
    "[graph][adjacency_list][construction][oracle]")
{
  std::array<pair_type, 7> const edges{{{4, 2}, {2, 4}, {4, 4}, {9, 2}, {2, 9}, {4, 2}, {9, 9}}};
  std::vector<pair_type> const expected(edges.begin(), edges.end());

  graph<false, true> multi{std::from_range, edges};
  graph<false, false> simple{std::from_range, edges};
  require_constructed_edges(multi, expected);
  require_constructed_edges(simple, expected);
  REQUIRE(multi.edge_count() == 7);
  REQUIRE(simple.edge_count() == 4);
  REQUIRE(multi.place_vertex().id() == 10u);
  REQUIRE(simple.place_vertex().id() == 10u);
}

TEST_CASE(
    "adjacency_list single-pass iterator constructor matches independent edge oracle",
    "[graph][adjacency_list][construction][oracle]")
{
  std::istringstream text{"10 1 10 2 1 2 2 10 10 1 2 2"};
  std::vector<pair_type> const expected{{10, 1}, {10, 2}, {1, 2}, {2, 10}, {10, 1}, {2, 2}};
  graph<true> g{std::istream_iterator<stream_edge>{text}, std::istream_iterator<stream_edge>{}};
  require_constructed_edges(g, expected);
  REQUIRE(g.place_vertex().id() == 11u);
}

TEST_CASE(
    "adjacency_list from_range accepts lazy filtered and transformed edge range",
    "[graph][adjacency_list][construction][oracle]")
{
  std::vector<std::pair<int, pair_type>> input{
      {1, {7, 3}}, {0, {9, 8}}, {1, {3, 7}}, {1, {3, 3}}, {0, {6, 6}}, {1, {11, 7}}};
  auto selected = input | std::views::filter([](auto const &item) { return item.first != 0; }) |
                  std::views::transform([](auto const &item) { return item.second; });
  std::vector<pair_type> const expected{{7, 3}, {3, 7}, {3, 3}, {11, 7}};

  graph<true, true> directed_multi{std::from_range, selected};
  graph<false, false> undirected_simple{std::from_range, selected};
  require_constructed_edges(directed_multi, expected);
  require_constructed_edges(undirected_simple, expected);
  REQUIRE(directed_multi.edge_count() == 4);
  REQUIRE(undirected_simple.edge_count() == 3);
}

TEST_CASE(
    "adjacency_list edge-list initializer and range constructors agree with raw edges",
    "[graph][adjacency_list][construction][oracle]")
{
  std::vector<pair_type> const expected{{12, 6}, {6, 12}, {6, 6}, {0, 15}, {12, 6}};
  graph<true, true> from_list{{{12, 6}, {6, 12}, {6, 6}, {0, 15}, {12, 6}}};
  graph<true, true> from_range{std::from_range, expected};
  graph<true, true> from_pair{expected.begin(), expected.end()};

  require_constructed_edges(from_list, expected);
  require_constructed_edges(from_range, expected);
  require_constructed_edges(from_pair, expected);
  REQUIRE(from_list == from_range);
  REQUIRE(from_range == from_pair);
  REQUIRE(from_list.place_vertex().id() == 16u);
}

TEST_CASE(
    "adjacency_list sparse and empty input ranges retain independent expected vertices",
    "[graph][adjacency_list][construction][oracle]")
{
  std::forward_list<pair_type> const input{{100, 2}, {2, 100}, {100, 100}, {0, 2}};
  std::vector<pair_type> const expected(input.begin(), input.end());
  graph<false> g{input.begin(), input.end()};
  require_constructed_edges(g, expected);
  REQUIRE(g.vertex_count() == 3);
  REQUIRE(g.place_vertex().id() == 101u);

  std::vector<pair_type> const empty;
  graph<true> e{std::from_range, empty};
  require_constructed_edges(e, empty);
  REQUIRE(e.place_vertex().id() == 0u);
}

TEST_CASE(
    "adjacency_list initializer-list assignment reconstructs the exact requested edges",
    "[graph][adjacency_list][construction][oracle]")
{
  graph<true, true> g{{{0, 1}, {1, 2}}};
  g = {{10, 12}, {12, 10}, {10, 12}, {12, 12}};
  std::vector<pair_type> const expected{{10, 12}, {12, 10}, {10, 12}, {12, 12}};
  require_constructed_edges(g, expected);
  REQUIRE(g.place_vertex().id() == 13u);
  auto [next, added] =
      g.place_edge(typename graph<true, true>::vertex_type{10}, typename graph<true, true>::vertex_type{12});
  REQUIRE(added);
  REQUIRE(next.id() == 4u);
}
