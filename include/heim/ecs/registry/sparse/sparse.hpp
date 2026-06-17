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


template<
    typename Id           = default_identifier_t,
    typename Alloc        = std::allocator<Id>,
    typename DescSequence = type_sequence<>>
struct generic_static_registry
  : std::type_identity<generic_registry<generic_static_storage<Id, Alloc, DescSequence>, Id, Alloc>>
{
  template<
      typename    C,
      std::size_t PageSz = default_page_size_v>
  using with
  = generic_static_registry<
      Id,
      Alloc,
      typename DescSequence::template append<static_storage_descriptor<C, PageSz>>>;
};

using static_registry
= generic_static_registry<>;

} // namespace heim::ecs::sparse

#endif // HEIM_ECS_REGISTRY_SPARSE_SPARSE_HPP
