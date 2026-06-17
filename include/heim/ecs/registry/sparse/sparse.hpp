#ifndef HEIM_ECS_REGISTRY_SPARSE_SPARSE_HPP
#define HEIM_ECS_REGISTRY_SPARSE_SPARSE_HPP

#include <cstddef>
#include <memory>
#include "heim/ecs/identifier.hpp"
#include "storage/auto_storage.hpp"
#include "storage/static_storage.hpp"
#include "registry.hpp"

namespace heim::ecs::sparse
{
template<
    typename    Id     = default_identifier_t,
    std::size_t PageSz = default_page_size_v,
    typename    Alloc  = std::allocator<Id>>
using generic_auto_registry
= generic_registry<generic_auto_storage<Id, PageSz, Alloc>, Id, Alloc>;

using auto_registry
= generic_auto_registry<>;

} // namespace heim::ecs::sparse

#endif // HEIM_ECS_REGISTRY_SPARSE_SPARSE_HPP
