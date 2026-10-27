#include <catch2/catch_test_macros.hpp>
#include <heim/common/unique_allocated_ptr.hpp>
#include "support.hpp"

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace {
template<typename T>
struct fancy_pointer
{
  using element_type = T;
  using difference_type = std::ptrdiff_t;
  T *raw{};

  constexpr fancy_pointer() noexcept = default;
  constexpr fancy_pointer(std::nullptr_t) noexcept : raw{} { }
  explicit constexpr fancy_pointer(T *p) noexcept : raw{p} { }

  [[nodiscard]] constexpr explicit operator bool() const noexcept { return raw != nullptr; }
  [[nodiscard]] constexpr T *operator->() const noexcept { return raw; }
  [[nodiscard]] constexpr T &operator*() const noexcept { return *raw; }
  [[nodiscard]] constexpr T &operator[](std::size_t i) const noexcept { return raw[i]; }
  [[nodiscard]] friend constexpr bool operator== (fancy_pointer, fancy_pointer) noexcept = default;
};

template<typename T>
struct fancy_allocator
{
  using value_type = T;
  using pointer = fancy_pointer<T>;
  std::shared_ptr<heim_test::allocation::ledger> log;
  int owner{};

  explicit fancy_allocator(std::shared_ptr<heim_test::allocation::ledger> l, int id = 0)
    : log{std::move(l)}, owner{id} { }

  template<typename U>
  fancy_allocator(fancy_allocator<U> const &other) noexcept
    : log{other.log}, owner{other.owner} { }

  [[nodiscard]] pointer allocate(std::size_t n)
  {
    ++log->allocations;
    T *raw = std::allocator<T>{}.allocate(n);
    log->outstanding.push_back({raw, n, owner});
    return pointer{raw};
  }

  void deallocate(pointer ptr, std::size_t n) noexcept
  {
    ++log->deallocations;
    auto *raw = std::to_address(ptr);
    auto const it = std::find_if(log->outstanding.begin(), log->outstanding.end(),
        [raw](heim_test::allocation::block const &b)
        {
          return b.address == raw;
        });
    if (it == log->outstanding.end())
      ++log->wrong_deallocations;
    else
    {
      if (it->count != n || it->owner != owner)
        ++log->wrong_deallocations;
      log->outstanding.erase(it);
    }
    std::allocator<T>{}.deallocate(raw, n);
  }

  template<typename U>
  friend struct fancy_allocator;

  template<typename U>
  bool operator== (fancy_allocator<U> const &other) const noexcept
  { return log == other.log && owner == other.owner; }
};
}

TEST_CASE(
    "single allocation supports allocator fancy pointers",
    "[common][unique_allocated_ptr][fancy_pointer]")
{
  auto log = std::make_shared<heim_test::allocation::ledger>();
  {
    auto p = heim::allocate_unique<int>(fancy_allocator<int>{log, 18}, 83);
    STATIC_REQUIRE(std::same_as<decltype(p)::pointer, fancy_pointer<int>>);
    REQUIRE(static_cast<bool>(p));
    REQUIRE(*p == 83);
    REQUIRE(p.get().raw == std::addressof(*p));
  }
  REQUIRE(heim_test::allocation::balanced(*log));
}

TEST_CASE(
    "array allocation supports allocator fancy pointers",
    "[common][unique_allocated_ptr][fancy_pointer][array]")
{
  auto log = std::make_shared<heim_test::allocation::ledger>();
  {
    auto p = heim::allocate_unique<int[]>(fancy_allocator<int>{log, 12}, 5);
    STATIC_REQUIRE(std::same_as<decltype(p)::pointer, fancy_pointer<int>>);
    REQUIRE(p != nullptr);
    for (int i = 0; i != 5; ++i)
      REQUIRE(p[i] == 0);
    p[2] = 34;
    REQUIRE(p[2] == 34);
  }
  REQUIRE(heim_test::allocation::balanced(*log));
}

TEST_CASE(
    "overwrite single allocation supports allocator fancy pointers",
    "[common][unique_allocated_ptr][fancy_pointer][overwrite]")
{
  auto log = std::make_shared<heim_test::allocation::ledger>();
  {
    auto p = heim::allocate_unique_for_overwrite<int>(fancy_allocator<int>{log, 5});
    *p = 77;
    REQUIRE(*p == 77);
  }
  REQUIRE(heim_test::allocation::balanced(*log));
}

TEST_CASE(
    "overwrite array allocation supports allocator fancy pointers",
    "[common][unique_allocated_ptr][fancy_pointer][array][overwrite]")
{
  auto log = std::make_shared<heim_test::allocation::ledger>();
  {
    auto p = heim::allocate_unique_for_overwrite<int[]>(fancy_allocator<int>{log, 4}, 5);
    for (int i = 0; i != 5; ++i)
      p[i] = i * 7;
    for (int i = 0; i != 5; ++i)
      REQUIRE(p[i] == i * 7);
  }
  REQUIRE(heim_test::allocation::balanced(*log));
}

TEST_CASE(
    "fancy-pointer release preserves pointer and deleter ownership",
    "[common][unique_allocated_ptr][fancy_pointer]")
{
  auto log = std::make_shared<heim_test::allocation::ledger>();
  {
    auto p = heim::allocate_unique<int>(fancy_allocator<int>{log, 20}, 14);
    auto deleter = p.get_deleter();
    auto raw = p.release();
    REQUIRE_FALSE(p);
    REQUIRE(*raw == 14);
    deleter(raw);
  }
  REQUIRE(heim_test::allocation::balanced(*log));
}

TEST_CASE(
    "fancy-pointer array move and reset preserve allocation extent",
    "[common][unique_allocated_ptr][fancy_pointer][array]")
{
  auto log = std::make_shared<heim_test::allocation::ledger>();
  {
    auto a = heim::allocate_unique<int[]>(fancy_allocator<int>{log, 22}, 11);
    a[10] = 94;
    auto b = std::move(a);
    REQUIRE_FALSE(a);
    REQUIRE(b[10] == 94);
    b.reset();
    REQUIRE_FALSE(b);
  }
  REQUIRE(heim_test::allocation::balanced(*log));
}

TEST_CASE(
    "zero-length fancy-pointer arrays stay null and do not allocate",
    "[common][unique_allocated_ptr][fancy_pointer][array]")
{
  auto log = std::make_shared<heim_test::allocation::ledger>();
  auto p = heim::allocate_unique<int[]>(fancy_allocator<int>{log, 4}, 0);
  REQUIRE_FALSE(p);
  REQUIRE(log->allocations == 0);
  REQUIRE(heim_test::allocation::balanced(*log));
}
