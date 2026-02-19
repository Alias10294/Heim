#ifndef HEIM_NEW_POOL_HPP
#define HEIM_NEW_POOL_HPP

#include <memory>
#include "heim/identifier.hpp"
#include "sparse_set.hpp"

namespace heim::sparse_set_based
{
template<
    typename    Component,
    typename    Identifier = heim::identifier<>,
    std::size_t PageSize   = 1024,
    typename    Allocator  = std::allocator<Identifier>>
class new_pool;

template<
    typename    Component,
    typename    Identifier,
    std::size_t PageSize,
    typename    Allocator>
class new_pool
  : public sparse_set<Identifier, PageSize, Allocator>
{
  // TODO: implement
};


}

#endif // HEIM_NEW_POOL_HPP