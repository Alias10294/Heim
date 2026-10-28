#ifndef HEIM_TEST_UNIQUE_ALLOCATED_PTR_SUPPORT_HPP
#define HEIM_TEST_UNIQUE_ALLOCATED_PTR_SUPPORT_HPP

#include <algorithm>
#include <cstddef>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>
#include <vector>

namespace heim_test::allocation
{
struct construction_failure : std::runtime_error
{
  construction_failure() : std::runtime_error{"injected construction failure"} { }
};

struct block
{
  void *address;
  std::size_t count;
  int owner;
};

struct ledger
{
  int allocations{};
  int deallocations{};
  int wrong_deallocations{};
  int construct_attempts{};
  int constructs{};
  int destroys{};
  int fail_allocation_on{};
  int fail_construct_on{};
  std::vector<block> outstanding;
};

template<typename T>
struct tracking_allocator
{
  using value_type = T;
  std::shared_ptr<ledger> log;
  int owner{};

  tracking_allocator() : log{std::make_shared<ledger>()} { }
  explicit tracking_allocator(std::shared_ptr<ledger> l, int id = 0)
    : log{std::move(l)}, owner{id} { }

  template<typename U>
  tracking_allocator(tracking_allocator<U> const &other) noexcept
    : log{other.log}, owner{other.owner} { }

  [[nodiscard]] T *allocate(std::size_t n)
  {
    ++log->allocations;
    if (log->fail_allocation_on == log->allocations)
      throw std::bad_alloc{};

    auto *ptr = std::allocator<T>{}.allocate(n);
    try
    {
      log->outstanding.push_back({ptr, n, owner});
    }
    catch (...)
    {
      std::allocator<T>{}.deallocate(ptr, n);
      throw;
    }
    return ptr;
  }

  void deallocate(T *ptr, std::size_t n) noexcept
  {
    ++log->deallocations;
    auto const it = std::find_if(log->outstanding.begin(), log->outstanding.end(),
        [ptr](block const &b)
        {
          return b.address == ptr;
        });
    if (it == log->outstanding.end())
      ++log->wrong_deallocations;
    else
    {
      if (it->count != n || it->owner != owner)
        ++log->wrong_deallocations;
      log->outstanding.erase(it);
    }
    std::allocator<T>{}.deallocate(ptr, n);
  }

  template<typename U, typename ...Args>
  void construct(U *ptr, Args && ...args)
  {
    ++log->construct_attempts;
    if (log->fail_construct_on == log->construct_attempts)
      throw construction_failure{};
    ::new (static_cast<void *>(ptr)) U(std::forward<Args>(args)...);
    ++log->constructs;
  }

  template<typename U>
  void destroy(U *ptr) noexcept
  {
    ++log->destroys;
    std::destroy_at(ptr);
  }

  template<typename U>
  friend struct tracking_allocator;

  template<typename U>
  bool operator == (tracking_allocator<U> const &other) const noexcept
  { return log == other.log && owner == other.owner; }
};

// Checks that can be performed without invoking a Catch2 assertion in noexcept code.
inline bool balanced(ledger const &log)
{
  return log.allocations == log.deallocations
      && log.wrong_deallocations == 0
      && log.outstanding.empty();
}

struct live_value
{
  inline static int alive{};
  inline static int constructed{};
  inline static int destroyed{};
  inline static int next_id{};
  inline static std::vector<int> destruction_order;

  int id{};
  int payload{};

  explicit live_value(int p = 0)
    : id{next_id++}, payload{p}
  { ++alive;
  ++constructed; }

  live_value(live_value const &other)
    : id{next_id++}, payload{other.payload}
  { ++alive;
  ++constructed; }

  live_value(live_value && other) noexcept
    : id{next_id++}, payload{other.payload}
  { ++alive;
  ++constructed; }

  ~live_value()
  {
    --alive;
    ++destroyed;
    destruction_order.push_back(id);
  }

  static void reset()
  {
    alive = 0;
    constructed = 0;
    destroyed = 0;
    next_id = 0;
    destruction_order.clear();
  }
};

struct throwing_value
{
  inline static int attempts{};
  inline static int throw_on{};
  inline static int alive{};
  inline static int destructions{};
  inline static std::vector<int> destruction_order;

  int id{};

  throwing_value()
  {
    ++attempts;
    if (throw_on == attempts)
      throw construction_failure{};
    id = attempts;
    ++alive;
  }

  ~throwing_value()
  {
    --alive;
    ++destructions;
    destruction_order.push_back(id);
  }

  static void reset(int fail = 0)
  {
    attempts = 0;
    throw_on = fail;
    alive = 0;
    destructions = 0;
    destruction_order.clear();
  }
};

struct immovable_value
{
  int value;
  explicit immovable_value(int v) : value{v} { }
  immovable_value(immovable_value const &) = delete;
  immovable_value(immovable_value && ) = delete;
};

// An allocator with no default constructor exercises the deleter constraints.
template<typename T>
struct explicit_allocator
{
  using value_type = T;
  int id;
  explicit explicit_allocator(int n) noexcept : id{n} { }
  T *allocate(std::size_t n) { return std::allocator<T>{}.allocate(n); }
  void deallocate(T *p, std::size_t n) noexcept { std::allocator<T>{}.deallocate(p, n); }
};

}
#endif
