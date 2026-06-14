#ifndef HEIM_ECS_REGISTRY_SPARSE_POOL_HPP
#define HEIM_ECS_REGISTRY_SPARSE_POOL_HPP

#include <cstddef>
#include <type_traits>
#include "set.hpp"

namespace heim::ecs::sparse
{
template<
    typename    C,
    typename    Id,
    std::size_t PageSz,
    typename    Alloc>
class pool
  : public set<Id, PageSz, Alloc>
{ };

template<
    typename    C,
    typename    Id,
    std::size_t PageSz,
    typename    Alloc>
requires std::is_empty_v<C>
class pool
  : public set<Id, PageSz, Alloc>
{ };

} // namespace heim::ecs::sparse

#endif // HEIM_ECS_REGISTRY_SPARSE_POOL_HPP
