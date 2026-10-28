#include "support.hpp"

using namespace heim_test::graph_algorithm::bfs;

namespace
{
template<typename R, typename V>
concept can_make_path = requires(R const &result, V v)
{
  result.path_from(v);
};

template<typename R, typename V>
concept can_make_path_from_temporary = requires(R &&result, V v)
{
  std::move(result).path_from(v);
};

template<typename R>
concept can_make_target_path = requires(R const &result)
{
  result.path();
};

template<typename R>
concept can_make_target_path_from_temporary = requires(R &&result)
{
  std::move(result).path();
};
}

TEST_CASE("BFS interfaces expose the expected result types", "[graph][bfs][traits]")
{
  using G = graph<>;
  using V = vertex<G>;
  using R = hg::bfs_result<G>;
  using T = hg::bfs_to_result<G>;

  static_assert(std::same_as<decltype(hg::bfs), decltype(hg::breadth_first_search)>);
  static_assert(std::same_as<decltype(hg::bfs_to), decltype(hg::breadth_first_search_to)>);
  static_assert(std::same_as<decltype(hg::breadth_first_visit),
                             decltype(hg::breadth_first_visit)>);
  static_assert(std::same_as<decltype(std::declval<R const &>().reached(std::declval<V>())), bool>);
  static_assert(std::same_as<decltype(std::declval<R const &>().predecessor_of(std::declval<V>())), V>);
  static_assert(std::same_as<decltype(std::declval<T const &>().reached()), bool>);
}

TEST_CASE("BFS path views cannot be extracted from temporary result objects", "[graph][bfs][traits]")
{
  using G = graph<>;
  using V = vertex<G>;
  static_assert(can_make_path<hg::bfs_result<G>, V>);
  static_assert(!can_make_path_from_temporary<hg::bfs_result<G>, V>);
  static_assert(can_make_target_path<hg::bfs_to_result<G>>);
  static_assert(!can_make_target_path_from_temporary<hg::bfs_to_result<G>>);
}

TEST_CASE("BFS search accepts an input range of graph vertex descriptors", "[graph][bfs][traits]")
{
  using G = graph<>;
  using V = vertex<G>;
  using InputRange = std::vector<V> &;
  static_assert(std::invocable<decltype(hg::bfs), G const &, InputRange>);
  static_assert(std::invocable<decltype(hg::bfs_to), G const &, InputRange, V>);
  static_assert(std::invocable<decltype(hg::breadth_first_visit), G const &, InputRange, std::monostate>);
}
