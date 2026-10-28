#ifndef HEIM_TEST_GRAPH_ALGORITHM_PREDECESSOR_VIEW_SUPPORT_HPP
#define HEIM_TEST_GRAPH_ALGORITHM_PREDECESSOR_VIEW_SUPPORT_HPP

#include <heim/graph/algorithm/predecessor_view.hpp>

#include <array>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace heim_test::graph_algorithm::predecessor
{
struct parent_map
{
  std::array<int, 16> const *parents{};

  int get(int vertex) const { return parents->at(static_cast<std::size_t>(vertex)); }
};

using view = heim::graphs::predecessor_view<parent_map, int>;

inline std::vector<int> collect(view const &path)
{
  std::vector<int> result;
  for (auto vertex : path)
    result.push_back(vertex);
  return result;
}

inline std::array<int, 16> chain()
{
  std::array<int, 16> parents{};
  for (std::size_t i = 0; i < parents.size(); ++i)
    parents[i] = static_cast<int>(i);

  parents[7] = 5;
  parents[5] = 3;
  parents[3] = 1;
  parents[1] = 0;
  return parents;
}

} // namespace heim_test::graph_algorithm::predecessor

#endif // HEIM_TEST_GRAPH_ALGORITHM_PREDECESSOR_VIEW_SUPPORT_HPP
