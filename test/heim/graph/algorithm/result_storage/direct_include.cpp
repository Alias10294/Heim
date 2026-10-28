#include <heim/graph/algorithm/result_storage.hpp>

#include <concepts>
#include <unordered_map>

using holder =
    heim::graphs::detail::result_storage<std::unordered_map<int, int>, heim::graphs::default_property_map_t>;

static_assert(std::move_constructible<holder>);
