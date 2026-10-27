#include "support.hpp"

using namespace heim_test::sparse_map;

TEST_CASE(
    "sparse_map allocation failure on initial insert keeps map empty",
    "[common][sparse_map][exception]")
{
  auto state = std::make_shared<allocation_log>();
  {
    tracked_map<> m(typename tracked_map<>::allocator_type{state, 1});
    state->reject = true;
    REQUIRE_THROWS_AS(m.try_emplace(123u, 4), std::bad_alloc);
    state->reject = false;
    verify(m, {});
    REQUIRE(m.try_emplace(123u, 4).second);
    verify(m, {{123u, 4}});
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "sparse_map allocation failure when extending to new page retains previous values",
    "[common][sparse_map][exception]")
{
  auto state = std::make_shared<allocation_log>();
  {
    tracked_map<> m(typename tracked_map<>::allocator_type{state, 1});
    REQUIRE(m.try_emplace(1u, 10).second);
    state->reject = true;
    REQUIRE_THROWS_AS(m.try_emplace(8000u, 80), std::bad_alloc);
    state->reject = false;
    verify(m, {{1u, 10}});
    REQUIRE(m.try_emplace(8000u, 80).second);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "sparse_map failure at several allocation points leaves original elements",
    "[common][sparse_map][exception]")
{
  for (std::size_t allowance = 0;allowance != 5;++allowance)
  {
    auto state = std::make_shared<allocation_log>();
    {
      tracked_map<> m(typename tracked_map<>::allocator_type{state, 1});
      m.try_emplace(1u, 10);
      state->remaining = allowance;
      bool caught = false;
      try
      {
        m.try_emplace(16000u, 20);
      }
      catch (std::bad_alloc const&)
      {
        caught = true;
      }
      state->remaining = std::numeric_limits<std::size_t>::max();
      REQUIRE(m.at(1u) == 10);
      if (caught)
        verify(m, {{1u, 10}});
      else verify(m, {{1u, 10}, {16000u, 20}});
      REQUIRE(m.try_emplace(2u, 30).second);
    }
    REQUIRE(state->balanced());
  }
}

TEST_CASE("sparse_map emplace rollback on mapped constructor exception", "[common][sparse_map][exception]")
{
  struct throwing
  {
    int value;
    explicit throwing(int n) : value(n)
    { if (n<0) throw std::runtime_error("mapped construction"); }
    throwing(throwing const&) = default;
    throwing(throwing && ) = default;
    throwing& operator= (throwing const&) = default;
    throwing& operator= (throwing && ) = default;
  };
  map<unsigned, throwing> m;
  REQUIRE(m.try_emplace(1u, 10).second);
  REQUIRE_THROWS_AS(m.try_emplace(2u, -1), std::runtime_error);
  REQUIRE(m.size() == 1);
  REQUIRE(m.contains(1u));
  REQUIRE_FALSE(m.contains(2u));
  REQUIRE(m.at(1u).value == 10);
  REQUIRE(m.try_emplace(2u, 20).second);
  REQUIRE(m.at(2u).value == 20);
}

TEST_CASE(
    "sparse_map failed emplace on an existing sparse page leaves no stale entry",
    "[common][sparse_map][exception]")
{
  struct throwing
  {
    int value;
    explicit throwing(int n) : value(n)
    { if (n<0) throw std::runtime_error("mapped construction"); }
  };
  map<unsigned, throwing> m;
  REQUIRE(m.try_emplace(16u, 16).second);
  REQUIRE_THROWS_AS(m.try_emplace(17u, -1), std::runtime_error);
  REQUIRE_FALSE(m.contains(17u));
  REQUIRE(m.try_emplace(17u, 17).second);
  REQUIRE(m.size() == 2);
}

TEST_CASE(
    "sparse_map insert_or_assign does not construct when key already exists",
    "[common][sparse_map][exception]")
{
  struct counted
  {
    int value;
    explicit counted(int n) : value(n)
    { if (n<0) throw std::runtime_error("bad constructor"); }
    counted &operator= (int n)
    {
      value = n;
      return *this;
    }
  };
  map<unsigned, counted> m;
  m.try_emplace(1u, 42);
  // Constructing `counted(-7)` throws, but assigning the same integer does not.
  // A temporary mapped value is therefore detectable.
  REQUIRE_NOTHROW(m.insert_or_assign(1u, -7));
  REQUIRE(m.at(1u).value == -7);
}

TEST_CASE("sparse_map duplicate try_emplace does not allocate", "[common][sparse_map][exception]")
{
  auto state = std::make_shared<allocation_log>();
  {
    tracked_map<> m(typename tracked_map<>::allocator_type{state, 1});
    m.try_emplace(1u, 10);
    state->reject = true;
    REQUIRE_NOTHROW(m.try_emplace(1u, 100));
    REQUIRE_NOTHROW(m.insert_or_assign(1u, 200));
    state->reject = false;
    verify(m, {{1u, 200}});
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "sparse_map allocation failure during construction frees all allocations",
    "[common][sparse_map][exception]")
{
  using M = tracked_map<>;
  for (std::size_t allowance = 0;allowance<5;++allowance)
  {
    auto state = std::make_shared<allocation_log>();
    state->remaining = allowance;
    std::vector<M::value_type> rows{{1u, 10}, {1000u, 20}, {2000u, 30}};
    try
    {
      M m(rows.begin(), rows.end(), M::allocator_type{state, 1});
      REQUIRE(m.at(1u) == 10);
    }
    catch (std::bad_alloc const&) { }
    REQUIRE(state->balanced());
  }
}

TEST_CASE(
    "sparse_map every allocation in fresh insertion rolls back",
    "[common][sparse_map][exception][injection]")
{
  using M = tracked_map<>;
  auto exercise = [](std::size_t allowance, bool inject)
  {
    auto state = std::make_shared<allocation_log>();
    bool failed = false;
    std::size_t operations = 0;
    {
      M map{M::allocator_type{state, 17}};
      map.try_emplace(1u, 11);
      auto const before = state->allocations;
      if (inject)
        state->remaining = allowance;
      try
      {
        map.try_emplace(16000u, 22);
      }
      catch (std::bad_alloc const &)
      {
        failed = true;
      }
      state->remaining = std::numeric_limits<std::size_t>::max();
      operations = state->allocations - before;
      INFO("allowance=" << allowance << " failed=" << failed);
      if (failed)
        verify(map, {{1u, 11}}, 16001);
      else verify(map, {{1u, 11}, {16000u, 22}}, 16001);
      REQUIRE(map.try_emplace(30000u, 33).second);
      REQUIRE(map.at(30000u) == 33);
    }
    REQUIRE(state->balanced());
    return std::pair{operations, failed};
  };

  auto const [allocation_count, success] = exercise(0, false);
  REQUIRE_FALSE(success);
  REQUIRE(allocation_count >= 3);
  for (std::size_t failure_at = 0; failure_at < allocation_count; ++failure_at)
  {
    INFO("injected allocation index=" << failure_at << "/" << allocation_count);
    auto const [attempted, failed] = exercise(failure_at, true);
    REQUIRE(failed);
  }
  auto const [after, failed] = exercise(allocation_count, true);
  REQUIRE_FALSE(failed);
}

TEST_CASE(
    "sparse_map failed replace restores invariants and remains reusable",
    "[common][sparse_map][exception][replace]")
{
  using M = tracked_map<>;
  for (bool unique : {false, true})
  {
    auto state = std::make_shared<allocation_log>();
    {
      M m{M::allocator_type{state, 42}};
      m.try_emplace(1u, 10);

      // Inputs must use the map's exact container types. Construct them before
      // enabling injection so that an allocation failure comes from replace().
      using key_container = M::key_container_type;
      using mapped_container = M::mapped_container_type;
      key_container keys(
          unique ? std::initializer_list<unsigned>{9u, 17u}
                 : std::initializer_list<unsigned>{9u, 9u, 17u},
          key_container::allocator_type{state, 42});
      mapped_container values(
          unique ? std::initializer_list<int>{90, 170}
                 : std::initializer_list<int>{90, 900, 170},
          mapped_container::allocator_type{state, 42});

      state->reject = true;
      INFO("replace unique=" << unique);
      if (unique)
        REQUIRE_THROWS_AS(m.replace(heim::unique, std::move(keys), std::move(values)), std::bad_alloc);
      else
        REQUIRE_THROWS_AS(m.replace(std::move(keys), std::move(values)), std::bad_alloc);
      state->reject = false;
      // Following flat_map's exception policy, preserving the old association
      // is not required; the result must nonetheless be fully coherent.
      verify(m, snapshot(m), 256, {1u, 9u, 17u});
      m.clear();
      REQUIRE(m.try_emplace(25u, 250).second);
      verify(m, {{25u, 250}}, 256, {1u, 9u, 17u});
    }
    REQUIRE(state->balanced());
  }
}
