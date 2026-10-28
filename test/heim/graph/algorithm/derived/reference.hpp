#ifndef HEIM_TEST_GRAPH_ALGORITHM_DERIVED_REFERENCE_HPP
#define HEIM_TEST_GRAPH_ALGORITHM_DERIVED_REFERENCE_HPP

#include "support.hpp"

namespace heim_test::graph_algorithm::derived
{
struct reference_graph
{
  unsigned count;
  std::vector<std::pair<unsigned, unsigned>> edges;

  explicit reference_graph(unsigned vertices)
    : count{vertices}
  { }

  void add(unsigned u, unsigned v)
  { edges.emplace_back(u, v); }

  // Disjoint-set union is independent of the DFS implementation of CC.
  std::vector<unsigned> components() const
  {
    std::vector<unsigned> parent(count);
    std::iota(parent.begin(), parent.end(), 0u);

    auto root = [&](unsigned v)
    {
      while (parent[v] != v)
        v = parent[v];

      return v;
    };

    for (auto [u, v] : edges)
    {
      auto const a = root(u);
      auto const b = root(v);
      parent[a] = b;
    }

    for (unsigned &v : parent)
      v = root(v);

    return parent;
  }

  unsigned component_count() const
  {
    auto const labels = components();
    return static_cast<unsigned>(std::set<unsigned>{labels.begin(), labels.end()}.size());
  }

  bool connected(unsigned u, unsigned v) const
  {
    auto const labels = components();
    return labels[u] == labels[v];
  }

  // Every correct topological order is a permutation where each edge points
  // forwards. Reverse topological order reverses that inequality.
  template<typename R>
  void verify_order(R &&order, bool reversed = false) const
  {
    std::vector<unsigned> positions(count, count);
    unsigned next{};

    for (auto v : order)
    {
      auto const id = [&]
      {
        if constexpr (requires { v.id(); })
          return static_cast<unsigned>(v.id());
        else
          return static_cast<unsigned>(v);
      }();
      REQUIRE(id < count);
      REQUIRE(positions[id] == count);
      positions[id] = next++;
    }

    REQUIRE(next == count);

    for (auto [u, v] : edges)
    {
      if (reversed)
        REQUIRE(positions[v] < positions[u]);
      else
        REQUIRE(positions[u] < positions[v]);
    }
  }
};

template<typename G, typename Result>
void verify_components(G const &, Result const &result, reference_graph const &reference)
{
  auto const expected = reference.components();
  REQUIRE(result.component_count() == reference.component_count());
  REQUIRE(result.connected() == (reference.component_count() < 2));

  std::set<std::size_t> labels;

  for (unsigned u = 0; u < reference.count; ++u)
  {
    auto const vu = at<G>(u);
    auto const actual = static_cast<std::size_t>(result.component_id_of(vu));
    REQUIRE(actual < reference.component_count());
    labels.insert(actual);

    for (unsigned v = 0; v < reference.count; ++v)
    {
      auto const vv = at<G>(v);
      REQUIRE(result.connected(vu, vv) == (expected[u] == expected[v]));
      REQUIRE((result.component_id_of(vu) == result.component_id_of(vv)) == (expected[u] == expected[v]));
    }
  }

  REQUIRE(labels.size() == reference.component_count());
}

template<typename G>
void verify_topological(G const &g, reference_graph const &reference)
{
  auto const forward = runtime_topological_sort(g);
  auto const backwards = [&]
  {
    std::vector<vertex<G>> values;
    runtime_reverse_topological_sort(g, std::back_inserter(values));
    return values;
  }();

  reference.verify_order(forward);
  reference.verify_order(backwards, true);
}

} // namespace heim_test::graph_algorithm::derived

#endif // HEIM_TEST_GRAPH_ALGORITHM_DERIVED_REFERENCE_HPP
