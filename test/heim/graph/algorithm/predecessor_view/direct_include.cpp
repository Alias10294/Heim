#include <heim/graph/algorithm/predecessor_view.hpp>

#include <concepts>
#include <ranges>

struct self_parent
{
  constexpr int get(int value) const noexcept { return value; }
};

using view = heim::graphs::predecessor_view<self_parent, int>;
static_assert(std::ranges::view<view>);
static_assert(std::ranges::input_range<view>);
