#include "support.hpp"
using namespace heim_test::sparse_map;

namespace
{
struct throwing_assignment
{
  int value{};
  inline static bool fail_move_assignment = false;
  inline static bool fail_copy_assignment = false;

  explicit throwing_assignment(int v) : value(v) { }
  throwing_assignment(throwing_assignment const &) = default;
  throwing_assignment(throwing_assignment &&) noexcept = default;
  throwing_assignment &operator=(throwing_assignment const &other)
  {
    if (fail_copy_assignment) throw std::runtime_error{"injected copy assignment"};
    value = other.value;
    return *this;
  }
  throwing_assignment &operator=(throwing_assignment &&other)
  {
    if (fail_move_assignment) throw std::runtime_error{"injected move assignment"};
    value = other.value;
    return *this;
  }
};
}

TEST_CASE("sparse_map erase propagates mapped assignment exception without corrupting keys", "[common][sparse_map][exception][mapped]")
{
  map<unsigned, throwing_assignment> map;
  map.try_emplace(1u, 10);
  map.try_emplace(9u, 90);
  throwing_assignment::fail_move_assignment = true;
  try
  {
    REQUIRE_THROWS_AS(map.erase(1u), std::runtime_error);
  }
  catch (...)
  {
    throwing_assignment::fail_move_assignment = false;
    throw;
  }
  throwing_assignment::fail_move_assignment = false;
  REQUIRE(map.size() == 2);
  REQUIRE(map.contains(1u));
  REQUIRE(map.contains(9u));
  REQUIRE(map.at(1u).value == 10);
  REQUIRE(map.at(9u).value == 90);
  REQUIRE(map.try_erase(1u));
  REQUIRE(map.at(9u).value == 90);
}

TEST_CASE("sparse_map insert_or_assign handles throwing mapped copy assignment", "[common][sparse_map][exception][mapped]")
{
  map<unsigned, throwing_assignment> map;
  map.try_emplace(1u, 10);
  throwing_assignment replacement{99};
  throwing_assignment::fail_copy_assignment = true;
  try
  {
    REQUIRE_THROWS_AS(map.insert_or_assign(1u, replacement), std::runtime_error);
  }
  catch (...)
  {
    throwing_assignment::fail_copy_assignment = false;
    throw;
  }
  throwing_assignment::fail_copy_assignment = false;
  REQUIRE(map.size() == 1);
  REQUIRE(map.at(1u).value == 10);
  REQUIRE_FALSE(map.insert_or_assign(1u, replacement).second);
  REQUIRE(map.at(1u).value == 99);
}

TEST_CASE("sparse_map failed first copy-assignment allocation preserves both operands", "[common][sparse_map][exception][copy]")
{
  using M = tracked_map<>;
  auto state = std::make_shared<allocation_log>();
  {
    M source{M::allocator_type{state, 1}};
    M destination{M::allocator_type{state, 1}};
    for (unsigned key = 0; key < 40; ++key)
      source.try_emplace(key * 8, static_cast<int>(key));
    destination.try_emplace(7000u, 70);
    state->remaining = 0;
    REQUIRE_THROWS_AS(destination = source, std::bad_alloc);
    state->remaining = std::numeric_limits<std::size_t>::max();
    verify(destination, {{7000u, 70}}, 7001);
    REQUIRE(source.size() == 40);
    REQUIRE(source.at(8u) == 1);
    REQUIRE(destination.try_emplace(8000u, 80).second);
  }
  REQUIRE(state->balanced());
}
