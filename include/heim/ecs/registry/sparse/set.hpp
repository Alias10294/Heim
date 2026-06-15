#ifndef HEIM_ECS_REGISTRY_SPARSE_SET_HPP
#define HEIM_ECS_REGISTRY_SPARSE_SET_HPP

#include <cstddef>

namespace heim::ecs::sparse
{
template<
    typename    Id,
    std::size_t PageSz,
    typename    Alloc>
class generic_set
{ };

} // namespace heim::ecs::sparse

#endif // HEIM_ECS_REGISTRY_SPARSE_SET_HPP
