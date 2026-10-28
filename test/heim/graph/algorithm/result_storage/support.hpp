#ifndef HEIM_TEST_GRAPH_ALGORITHM_RESULT_STORAGE_SUPPORT_HPP
#define HEIM_TEST_GRAPH_ALGORITHM_RESULT_STORAGE_SUPPORT_HPP

#include <heim/graph/algorithm/result_storage.hpp>
#include <heim/property/primitives.hpp>
#include <heim/property/traits.hpp>

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace heim_test::graph_algorithm::result
{
namespace hp = heim::properties;
namespace hg = heim::graphs;

template <typename Key = int, typename Value = int>
using owned_storage = hg::detail::result_storage<std::unordered_map<Key, Value>, hg::default_property_map_t>;

struct external_map
{
  std::unordered_map<int, int> *values{};

  int &get(int key) const { return values->at(key); }

  void set(int key, int value) const { values->insert_or_assign(key, value); }
};

using external_storage = hg::detail::result_storage<std::unordered_map<int, int>, external_map>;

struct empty_storage
{
  int unused{};
};

} // namespace heim_test::graph_algorithm::result

#endif // HEIM_TEST_GRAPH_ALGORITHM_RESULT_STORAGE_SUPPORT_HPP
