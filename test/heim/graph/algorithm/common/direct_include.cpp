#include <heim/graph/algorithm/color.hpp>
#include <heim/graph/algorithm/visit.hpp>

#include <concepts>

static_assert(heim::graphs::colors::gray_v<heim::graphs::color> == heim::graphs::color::gray);
static_assert(std::same_as<decltype(heim::graphs::detail::invoke_visit([] {})), heim::graphs::visit>);
