#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <heim/common/unique_allocated_ptr.hpp>

#include "support.hpp"

using heim_test::allocation::immovable_value;
using heim_test::allocation::live_value;

TEST_CASE("single allocation forwards constructor arguments", "[common][unique_allocated_ptr][single]")
{
  auto p = heim::allocate_unique<std::string>(std::allocator<std::string>{}, 4u, 'x');
  REQUIRE(*p == "xxxx");
}

TEST_CASE("single allocation supports immovable objects", "[common][unique_allocated_ptr][single]")
{
  auto p = heim::allocate_unique<immovable_value>(std::allocator<immovable_value>{}, 92);
  REQUIRE(p->value == 92);
}

TEST_CASE("single allocation owns exactly one object", "[common][unique_allocated_ptr][single]")
{
  live_value::reset();
  {
    auto p = heim::allocate_unique<live_value>(std::allocator<live_value>{}, 71);
    REQUIRE(live_value::alive == 1);
    REQUIRE(live_value::constructed == 1);
    REQUIRE(p->payload == 71);
  }
  REQUIRE(live_value::alive == 0);
  REQUIRE(live_value::destroyed == 1);
}

TEST_CASE(
    "moving a single allocation transfers ownership without constructing another object",
    "[common][unique_allocated_ptr][single]")
{
  live_value::reset();
  {
    auto first = heim::allocate_unique<live_value>(std::allocator<live_value>{}, 4);
    auto *address = first.get();
    auto second = std::move(first);
    REQUIRE(first == nullptr);
    REQUIRE(second.get() == address);
    REQUIRE(live_value::constructed == 1);
    REQUIRE(live_value::alive == 1);
  }
  REQUIRE(live_value::destroyed == 1);
}

TEST_CASE("move assignment destroys the previous pointee", "[common][unique_allocated_ptr][single]")
{
  live_value::reset();
  {
    auto source = heim::allocate_unique<live_value>(std::allocator<live_value>{}, 10);
    auto dest = heim::allocate_unique<live_value>(std::allocator<live_value>{}, 20);
    auto *address = source.get();
    dest = std::move(source);
    REQUIRE(source == nullptr);
    REQUIRE(dest.get() == address);
    REQUIRE(dest->payload == 10);
    REQUIRE(live_value::alive == 1);
    REQUIRE(live_value::destroyed == 1);
  }
  REQUIRE(live_value::destroyed == 2);
}

TEST_CASE("reset destroys the current single pointee once", "[common][unique_allocated_ptr][single]")
{
  live_value::reset();
  auto p = heim::allocate_unique<live_value>(std::allocator<live_value>{});
  p.reset();
  REQUIRE_FALSE(p);
  REQUIRE(live_value::alive == 0);
  REQUIRE(live_value::destroyed == 1);
  p.reset();
  REQUIRE(live_value::destroyed == 1);
}

TEST_CASE(
    "release transfers cleanup responsibility to the caller",
    "[common][unique_allocated_ptr][single]")
{
  live_value::reset();
  auto p = heim::allocate_unique<live_value>(std::allocator<live_value>{});
  auto deleter = p.get_deleter();
  auto raw = p.release();
  REQUIRE_FALSE(p);
  REQUIRE(live_value::alive == 1);
  deleter(raw);
  REQUIRE(live_value::alive == 0);
  REQUIRE(live_value::destroyed == 1);
}

TEST_CASE(
    "swapping single allocations preserves both pointers and values",
    "[common][unique_allocated_ptr][single]")
{
  live_value::reset();
  {
    auto a = heim::allocate_unique<live_value>(std::allocator<live_value>{}, 4);
    auto b = heim::allocate_unique<live_value>(std::allocator<live_value>{}, 8);
    auto *pa = a.get();
    auto *pb = b.get();
    a.swap(b);
    REQUIRE(a.get() == pb);
    REQUIRE(b.get() == pa);
    REQUIRE(a->payload == 8);
    REQUIRE(b->payload == 4);
  }
  REQUIRE(live_value::destroyed == 2);
}

TEST_CASE(
    "overwrite single allocation constructs and destroys class types",
    "[common][unique_allocated_ptr][single][overwrite]")
{
  live_value::reset();
  {
    auto p = heim::allocate_unique_for_overwrite<live_value>(std::allocator<live_value>{});
    REQUIRE(live_value::alive == 1);
    REQUIRE(p->payload == 0);
    p->payload = 132;
    REQUIRE(p->payload == 132);
  }
  REQUIRE(live_value::alive == 0);
  REQUIRE(live_value::destroyed == 1);
}

TEST_CASE(
    "overwrite single scalar is writable before it is read",
    "[common][unique_allocated_ptr][single][overwrite]")
{
  auto p = heim::allocate_unique_for_overwrite<int>(std::allocator<int>{});
  REQUIRE(p != nullptr);
  *p = 712;
  REQUIRE(*p == 712);
}

TEST_CASE("normal single scalar allocation value-initializes", "[common][unique_allocated_ptr][single]")
{
  auto p = heim::allocate_unique<int>(std::allocator<int>{});
  REQUIRE(*p == 0);
}

TEST_CASE(
    "single allocation forwards move-only constructor arguments",
    "[common][unique_allocated_ptr][single]")
{
  struct holder {
    std::unique_ptr<int> value;
    explicit holder(std::unique_ptr<int> v) : value{std::move(v)} { }
  };
  auto p = heim::allocate_unique<holder>(std::allocator<holder>{}, std::make_unique<int>(55));
  REQUIRE(*p->value == 55);
}

TEST_CASE(
    "single allocation accepts an explicit non-default-constructible allocator",
    "[common][unique_allocated_ptr][single]")
{
  auto p = heim::allocate_unique<int>(heim_test::allocation::explicit_allocator<int>{11}, 47);
  REQUIRE(*p == 47);
}
