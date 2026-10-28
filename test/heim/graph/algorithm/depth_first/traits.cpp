#include "support.hpp"

using namespace heim_test::graph_algorithm::dfs;

namespace
{
template<typename R, typename V>
concept can_make_path = requires(R const &result, V v) { result.path_from(v); };

template<typename R, typename V>
concept can_make_path_from_temporary = requires(R &&result, V v)
{ std::move(result).path_from(v); };

template<typename R>
concept can_make_target_path = requires(R const &result) { result.path(); };

template<typename R>
concept can_make_target_path_from_temporary = requires(R &&result)
{ std::move(result).path(); };
}

TEST_CASE("DFS exposes its public result types and aliases", "[graph][dfs][traits]")
{
  using G = graph<>;
  using V = vertex<G>;
  using R = hg::dfs_result<G>;
  using T = hg::dfs_to_result<G>;
  static_assert(std::same_as<decltype(hg::dfs), decltype(hg::depth_first_search)>);
  static_assert(std::same_as<decltype(hg::dfs_to), decltype(hg::depth_first_search_to)>);
  static_assert(std::same_as<decltype(std::declval<R const &>().reached(V{})), bool>);
  static_assert(std::same_as<decltype(std::declval<R const &>().predecessor_of(V{})), V>);
  static_assert(std::same_as<decltype(std::declval<T const &>().reached()), bool>);
}

TEST_CASE("DFS path views cannot be extracted from temporary results", "[graph][dfs][traits]")
{
  using G = graph<>;
  using V = vertex<G>;
  static_assert(can_make_path<hg::dfs_result<G>, V>);
  static_assert(!can_make_path_from_temporary<hg::dfs_result<G>, V>);
  static_assert(can_make_target_path<hg::dfs_to_result<G>>);
  static_assert(!can_make_target_path_from_temporary<hg::dfs_to_result<G>>);
}

TEST_CASE("DFS searches accept root input ranges", "[graph][dfs][traits]")
{
  using G = graph<>;
  using V = vertex<G>;
  using Roots = std::vector<V> &;
  static_assert(std::invocable<decltype(hg::dfs), G const &, Roots>);
  static_assert(std::invocable<decltype(hg::dfs_to), G const &, Roots, V>);
  static_assert(std::invocable<decltype(hg::depth_first_visit), G const &, Roots, std::monostate>);
}

TEST_CASE("DFS visit returns bool for completed or stopped traversals", "[graph][dfs][traits]")
{
  using G = graph<>;
  using V = vertex<G>;
  static_assert(std::same_as<decltype(hg::depth_first_visit(
      std::declval<G const &>(), V{}, std::monostate{})), bool>);
}
