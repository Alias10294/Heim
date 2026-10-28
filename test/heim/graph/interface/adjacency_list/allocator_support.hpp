#ifndef HEIM_TEST_GRAPH_ADJACENCY_ALLOCATOR_SUPPORT_HPP
#define HEIM_TEST_GRAPH_ADJACENCY_ALLOCATOR_SUPPORT_HPP

#include <limits>
#include <map>
#include <memory>
#include <new>
#include <type_traits>
#include <typeindex>

#include "support.hpp"

namespace heim_test::adjacency
{
struct allocation_state
{
  struct allocation
  {
    std::size_t count;
    int owner;
    std::type_index type;
  };
  std::map<void *, allocation> outstanding;
  std::map<std::type_index, std::size_t> allocations_by_type;
  std::size_t allocations{}, deallocations{}, wrong_deallocations{};
  std::size_t remaining = std::numeric_limits<std::size_t>::max();
  bool reject{};

  bool balanced() const
  {
    return outstanding.empty() && allocations == deallocations && wrong_deallocations == 0;
  }
};

template <typename T, bool Propagate = false>
class tracked_allocator
{
public:
  using value_type = T;
  using propagate_on_container_copy_assignment = std::bool_constant<Propagate>;
  using propagate_on_container_move_assignment = std::bool_constant<Propagate>;
  using propagate_on_container_swap = std::bool_constant<Propagate>;
  using is_always_equal = std::false_type;
  template <typename U>
  struct rebind
  {
    using other = tracked_allocator<U, Propagate>;
  };

  std::shared_ptr<allocation_state> state{};
  int id{};
  tracked_allocator() = default;
  tracked_allocator(std::shared_ptr<allocation_state> s, int owner) : state(std::move(s)), id(owner) {}
  template <typename U>
  tracked_allocator(tracked_allocator<U, Propagate> const &other) noexcept : state(other.state), id(other.id)
  {
  }

  T *allocate(std::size_t n)
  {
    if (state && (state->reject || state->remaining == 0))
      throw std::bad_alloc{};
    auto *ptr = std::allocator<T>{}.allocate(n);
    if (state)
    {
      try
      {
        state->outstanding.emplace(static_cast<void *>(ptr),
                                   allocation_state::allocation{n, id, std::type_index{typeid(T)}});
        ++state->allocations_by_type[std::type_index{typeid(T)}];
      }
      catch (...)
      {
        state->outstanding.erase(static_cast<void *>(ptr));
        std::allocator<T>{}.deallocate(ptr, n);
        throw;
      }
      ++state->allocations;
      if (state->remaining != std::numeric_limits<std::size_t>::max())
        --state->remaining;
    }
    return ptr;
  }
  void deallocate(T *p, std::size_t n) noexcept
  {
    if (state)
    {
      auto it = state->outstanding.find(static_cast<void *>(p));
      if (it == state->outstanding.end() || it->second.owner != id || it->second.count != n ||
          it->second.type != std::type_index{typeid(T)})
        ++state->wrong_deallocations;
      else
        state->outstanding.erase(it);
      ++state->deallocations;
    }
    std::allocator<T>{}.deallocate(p, n);
  }
  template <typename U, bool P>
  friend class tracked_allocator;
};

template <typename A, typename B, bool P>
constexpr bool operator==(tracked_allocator<A, P> const &a, tracked_allocator<B, P> const &b) noexcept
{
  return a.id == b.id && a.state == b.state;
}

template <bool Propagate = false,
          bool Directed = false,
          bool Multi = false,
          bool In = true,
          bool Out = true,
          std::size_t Page = 8>
using tracked_graph = graph<Directed, Multi, In, Out, unsigned, Page, tracked_allocator<unsigned, Propagate>>;
}

#endif // HEIM_TEST_GRAPH_ADJACENCY_ALLOCATOR_SUPPORT_HPP
