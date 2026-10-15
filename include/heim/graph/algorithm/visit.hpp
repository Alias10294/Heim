#ifndef HEIM_GRAPH_ALGORITHM_VISIT_HPP
#define HEIM_GRAPH_ALGORITHM_VISIT_HPP

#include <concepts>
#include <functional>
#include <utility>

namespace heim::graphs
{
enum class visit
{
  proceed = 1,
  prune,
  skip,
  stop    = 0,
};


namespace detail
{
template<typename Fn>
constexpr
visit
invoke_visit(Fn &&fn)
requires
    std::same_as<std::invoke_result_t<Fn>, visit>
 || std::same_as<std::invoke_result_t<Fn>, void>
{
  if constexpr (std::same_as<std::invoke_result_t<Fn>, visit>)
    return std::invoke(std::forward<Fn>(fn));
  else
  {
    static_cast<void>(std::invoke(std::forward<Fn>(fn)));
    return visit::proceed;
  }
}

} // namespace detail
} // namespace heim::graphs

#endif // HEIM_GRAPH_ALGORITHM_VISIT_HPP
