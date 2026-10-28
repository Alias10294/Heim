#ifndef HEIM_TEST_GRAPH_ADJACENCY_COPY_SELECTION_SUPPORT_HPP
#define HEIM_TEST_GRAPH_ADJACENCY_COPY_SELECTION_SUPPORT_HPP

#include "allocator_support.hpp"

#include <cstddef>
#include <memory>
#include <set>
#include <type_traits>

namespace heim_test::adjacency
{
// A deliberately non-trivial SOCCC policy: copy construction must select a
// different allocator identity. All rebound allocators share the same ledger.
// Normal allocator copies and explicit allocator construction do not select.
template<typename T>
class copy_select_allocator
{
public:
  using value_type = T;
  using propagate_on_container_copy_assignment = std::false_type;
  using propagate_on_container_move_assignment = std::false_type;
  using propagate_on_container_swap = std::false_type;
  using is_always_equal = std::false_type;

  template<typename U>
  struct rebind { using other = copy_select_allocator<U>; };

  std::shared_ptr<allocation_state> state{};
  int id{};

  copy_select_allocator() = default;

  copy_select_allocator(std::shared_ptr<allocation_state> ledger, int identity) noexcept
    : state{std::move(ledger)}, id{identity}
  { }

  template<typename U>
  copy_select_allocator(copy_select_allocator<U> const &other) noexcept
    : state{other.state}, id{other.id}
  { }

  [[nodiscard]] copy_select_allocator
  select_on_container_copy_construction() const noexcept
  { return {state, id + 100}; }

  [[nodiscard]] T *allocate(std::size_t n)
  { return tracked_allocator<T, false>{state, id}.allocate(n); }

  void deallocate(T *ptr, std::size_t n) noexcept
  { tracked_allocator<T, false>{state, id}.deallocate(ptr, n); }

  template<typename>
  friend class copy_select_allocator;
};

template<typename A, typename B>
constexpr bool
operator==(copy_select_allocator<A> const &a,
           copy_select_allocator<B> const &b) noexcept
{ return a.id == b.id && a.state == b.state; }

template<bool Directed = false, bool Multi = false,
         bool In = true, bool Out = true, std::size_t Page = 8>
using copy_select_graph = graph<Directed, Multi, In, Out, unsigned, Page,
                                copy_select_allocator<unsigned>>;

inline std::size_t allocations_owned_by(allocation_state const &ledger, int owner)
{
  std::size_t count{};
  for (auto const &[ptr, info] : ledger.outstanding)
  {
    (void)ptr;
    if (info.owner == owner)
      ++count;
  }
  return count;
}

inline void require_only_owners(allocation_state const &ledger, int first, int second)
{
  for (auto const &[ptr, info] : ledger.outstanding)
  {
    (void)ptr;
    REQUIRE((info.owner == first || info.owner == second));
  }
  REQUIRE(ledger.wrong_deallocations == 0);
}

inline std::set<void *> allocated_addresses(allocation_state const &ledger)
{
  std::set<void *> addresses;
  for (auto const &[ptr, info] : ledger.outstanding)
  {
    (void)info;
    addresses.insert(ptr);
  }
  return addresses;
}

// The source graph remains alive: every new allocation belongs to the copy.
inline void require_new_allocations_owned_by(
    allocation_state const &ledger, std::set<void *> const &before, int owner)
{
  std::size_t observed{};
  for (auto const &[ptr, info] : ledger.outstanding)
    if (!before.contains(ptr))
    {
      ++observed;
      REQUIRE(info.owner == owner);
    }
  REQUIRE(observed > 0);
}
} // namespace heim_test::adjacency

#endif // HEIM_TEST_GRAPH_ADJACENCY_COPY_SELECTION_SUPPORT_HPP
