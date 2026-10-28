#include <heim/graph/algorithm/indirect_d_ary_heap.hpp>

#include <concepts>
#include <functional>
#include <vector>

struct priority
{
  int get(int value) const noexcept { return value; }
};

struct index
{
  std::size_t get(int) const noexcept { return 0; }
  void set(int, std::size_t) const noexcept {}
};

using heap =
    heim::graphs::detail::indirect_d_ary_heap<2, std::vector<int> &, priority, index, std::ranges::less>;
static_assert(std::move_constructible<heap>);
