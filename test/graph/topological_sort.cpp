#include <catch2/catch_test_macros.hpp>
#include <heim/graph.hpp>
#include "test_utils.hpp"
#include <algorithm>
#include <deque>
#include <iterator>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <utility>

namespace hg = heim::graphs;
namespace {
template<typename Graph, typename Range>
bool is_valid_topological_order(Graph const& graph, Range&& range)
{
  auto order = heim_test::collect(std::forward<Range>(range));
  if (order.size() != hg::vertex_count(graph)) return false;
  std::unordered_map<typename Graph::vertex_type, std::size_t> position;
  for (std::size_t i = 0; i < order.size(); ++i)
    if (!position.emplace(order[i], i).second) return false;
  for (auto e : hg::edges(graph))
    if (position.at(hg::source(graph, e)) >= position.at(hg::target(graph, e))) return false;
  return true;
}
}

TEST_CASE("topological sort places every source before each target", "[graph][topological_sort]")
{
  heim::adjacency_list<true> g{{0u, 2u}, {1u, 2u}, {2u, 3u}, {1u, 4u}, {4u, 5u}};
  auto result = hg::topological_sort(g);
  REQUIRE(is_valid_topological_order(g, result));
}

TEST_CASE("topological sort includes isolated vertices", "[graph][topological_sort]")
{
  heim::adjacency_list<true> g{{0u, 1u}};
  hg::place_vertex(g); // vertex 2
  hg::place_vertex(g); // vertex 3
  REQUIRE(is_valid_topological_order(g, hg::topological_sort(g)));
}

TEST_CASE("topological sort accepts a caller-supplied deque", "[graph][topological_sort]")
{
  heim::adjacency_list<true> g{{0u, 1u}, {0u, 2u}, {2u, 3u}};
  std::deque<heim::adjacency_list<true>::vertex_type> out;
  auto range = hg::topological_sort(g, out);
  REQUIRE(is_valid_topological_order(g, range));
  REQUIRE(out.size() == 4);
}

TEST_CASE("reverse topological sort writes targets before sources", "[graph][topological_sort]")
{
  heim::adjacency_list<true> g{{0u, 1u}, {0u, 2u}, {2u, 3u}};
  std::vector<heim::adjacency_list<true>::vertex_type> reverse;
  hg::reverse_topological_sort(g, std::back_inserter(reverse));
  std::ranges::reverse(reverse);
  REQUIRE(is_valid_topological_order(g, reverse));
}

TEST_CASE("topological sorting a directed cycle throws", "[graph][topological_sort]")
{
  heim::adjacency_list<true> cyclic{{0u, 1u}, {1u, 2u}, {2u, 0u}};
  REQUIRE_THROWS_AS(hg::topological_sort(cyclic), std::logic_error);
  std::vector<heim::adjacency_list<true>::vertex_type> output;
  REQUIRE_THROWS_AS(hg::reverse_topological_sort(cyclic, std::back_inserter(output)), std::logic_error);
}

TEST_CASE("topological sort of an empty graph is empty", "[graph][topological_sort]")
{
  heim::adjacency_list<true> g;
  REQUIRE(std::ranges::empty(hg::topological_sort(g)));
}
