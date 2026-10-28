#include <heim/graph/algorithm/depth_first/visit.hpp>

#include <concepts>
#include <type_traits>

static_assert(std::is_class_v<decltype(heim::graphs::depth_first_visit)>);
