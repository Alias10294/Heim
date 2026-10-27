#ifndef HEIM_TEST_GRAPH_UTILS_HPP
#define HEIM_TEST_GRAPH_UTILS_HPP

#include <heim/graph.hpp>
#include <algorithm>
#include <cstddef>
#include <ranges>
#include <vector>
#include <utility>

namespace heim_test {
template<typename Range>
auto collect(Range&& range)
{
  using T = std::ranges::range_value_t<Range>;
  std::vector<T> result;
  for (auto item : range) result.push_back(item);
  return result;
}

template<typename Graph, typename Vertex>
bool adjacent(Graph const& graph, Vertex from, Vertex to)
{
  for (auto e : heim::graphs::out_edges(graph, from))
    if (heim::graphs::target(graph, e) == to) return true;
  return false;
}

template<typename Graph, typename Range, typename Vertex>
bool is_reverse_path(Graph const& graph, Range&& reverse_path, Vertex source, Vertex target)
{
  auto path = collect(std::forward<Range>(reverse_path));
  if (path.empty() || path.front() != target || path.back() != source) return false;
  for (std::size_t i = 1; i < path.size(); ++i)
    if (!adjacent(graph, path[i], path[i - 1])) return false;
  return true;
}
}
#endif
