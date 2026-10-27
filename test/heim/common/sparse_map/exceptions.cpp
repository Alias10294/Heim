#include "support.hpp"
using namespace heim_test::sparse_map;

TEST_CASE("sparse_map allocation failure on initial insert keeps map empty", "[common][sparse_map][exception]")
{
  auto state=std::make_shared<allocation_log>();
  {
    tracked_map<> m(typename tracked_map<>::allocator_type{state,1});
    state->reject=true;
    REQUIRE_THROWS_AS(m.try_emplace(123u,4),std::bad_alloc);
    state->reject=false;
    verify(m,{});
    REQUIRE(m.try_emplace(123u,4).second);
    verify(m,{{123u,4}});
  }
  REQUIRE(state->balanced());
}

TEST_CASE("sparse_map allocation failure when extending to new page retains previous values", "[common][sparse_map][exception]")
{
  auto state=std::make_shared<allocation_log>();
  {
    tracked_map<> m(typename tracked_map<>::allocator_type{state,1});
    REQUIRE(m.try_emplace(1u,10).second);
    state->reject=true;
    REQUIRE_THROWS_AS(m.try_emplace(8000u,80),std::bad_alloc);
    state->reject=false;
    verify(m,{{1u,10}});
    REQUIRE(m.try_emplace(8000u,80).second);
  }
  REQUIRE(state->balanced());
}

TEST_CASE("sparse_map failure at several allocation points leaves original elements", "[common][sparse_map][exception]")
{
  for(std::size_t allowance=0;allowance!=5;++allowance)
  {
    auto state=std::make_shared<allocation_log>();
    {
      tracked_map<> m(typename tracked_map<>::allocator_type{state,1});
      m.try_emplace(1u,10);
      state->remaining=allowance;
      bool caught=false;
      try { m.try_emplace(16000u,20); }
      catch(std::bad_alloc const&) { caught=true; }
      state->remaining=std::numeric_limits<std::size_t>::max();
      REQUIRE(m.at(1u)==10);
      if(caught) verify(m,{{1u,10}});
      else verify(m,{{1u,10},{16000u,20}});
      REQUIRE(m.try_emplace(2u,30).second);
    }
    REQUIRE(state->balanced());
  }
}

TEST_CASE("sparse_map emplace rollback on mapped constructor exception", "[common][sparse_map][exception]")
{
  struct throwing
  {
    int value;
    explicit throwing(int n):value(n)
    { if(n<0) throw std::runtime_error("mapped construction"); }
    throwing(throwing const&)=default;
    throwing(throwing&&)=default;
    throwing& operator=(throwing const&)=default;
    throwing& operator=(throwing&&)=default;
  };
  map<unsigned,throwing> m;
  REQUIRE(m.try_emplace(1u,10).second);
  REQUIRE_THROWS_AS(m.try_emplace(2u,-1),std::runtime_error);
  REQUIRE(m.size()==1);
  REQUIRE(m.contains(1u));
  REQUIRE_FALSE(m.contains(2u));
  REQUIRE(m.at(1u).value==10);
  REQUIRE(m.try_emplace(2u,20).second);
  REQUIRE(m.at(2u).value==20);
}

TEST_CASE("sparse_map failed emplace on an existing sparse page leaves no stale entry", "[common][sparse_map][exception]")
{
  struct throwing
  {
    int value;
    explicit throwing(int n):value(n)
    { if(n<0) throw std::runtime_error("mapped construction"); }
  };
  map<unsigned,throwing> m;
  REQUIRE(m.try_emplace(16u,16).second);
  REQUIRE_THROWS_AS(m.try_emplace(17u,-1),std::runtime_error);
  REQUIRE_FALSE(m.contains(17u));
  REQUIRE(m.try_emplace(17u,17).second);
  REQUIRE(m.size()==2);
}

TEST_CASE("sparse_map insert_or_assign does not construct when key already exists", "[common][sparse_map][exception]")
{
  struct counted
  {
    int value;
    explicit counted(int n):value(n)
    { if(n<0) throw std::runtime_error("bad constructor"); }
    counted &operator=(int n) { value=n; return *this; }
  };
  map<unsigned,counted> m;
  m.try_emplace(1u,42);
  REQUIRE_FALSE(m.insert_or_assign(1u,99).second);
  REQUIRE(m.at(1u).value==99);
}

TEST_CASE("sparse_map duplicate try_emplace does not allocate", "[common][sparse_map][exception]")
{
  auto state=std::make_shared<allocation_log>();
  {
    tracked_map<> m(typename tracked_map<>::allocator_type{state,1});
    m.try_emplace(1u,10);
    state->reject=true;
    REQUIRE_NOTHROW(m.try_emplace(1u,100));
    REQUIRE_NOTHROW(m.insert_or_assign(1u,200));
    state->reject=false;
    verify(m,{{1u,200}});
  }
  REQUIRE(state->balanced());
}

TEST_CASE("sparse_map allocation failure during construction frees all allocations", "[common][sparse_map][exception]")
{
  using M=tracked_map<>;
  for(std::size_t allowance=0;allowance<5;++allowance)
  {
    auto state=std::make_shared<allocation_log>();
    state->remaining=allowance;
    std::vector<M::value_type> rows{{1u,10},{1000u,20},{2000u,30}};
    try
    {
      M m(rows.begin(),rows.end(),M::allocator_type{state,1});
      REQUIRE(m.at(1u)==10);
    }
    catch(std::bad_alloc const&) { }
    REQUIRE(state->balanced());
  }
}
