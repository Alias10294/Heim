#include "support.hpp"

using namespace heim_test::sparse_map;

TEST_CASE("sparse_map allocator reaches key, value and index storage", "[common][sparse_map][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    tracked_map<> m(typename tracked_map<>::allocator_type{state, 1});
    REQUIRE(m.get_allocator().id == 1);
    for (unsigned i = 0;i<60;++i)
      m.try_emplace(i*8u, int(i));
    REQUIRE(state->allocations > 0);
    REQUIRE_FALSE(state->outstanding.empty());
    REQUIRE(state->allocated_by_region[static_cast<std::size_t>(allocation_log::region::keys)] > 0);
    REQUIRE(state->allocated_by_region[static_cast<std::size_t>(allocation_log::region::values)] > 0);
    REQUIRE(state->allocated_by_region[static_cast<std::size_t>(allocation_log::region::pages)] > 0);
    REQUIRE(state->allocated_by_region[static_cast<std::size_t>(allocation_log::region::page_table)] > 0);
    m.clear();
    REQUIRE(m.empty());
  }
  REQUIRE(state->balanced());
}

TEST_CASE("sparse_map allocator-aware copy constructor deep copies", "[common][sparse_map][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    tracked_map<> a(typename tracked_map<>::allocator_type{state, 1});
    a.try_emplace(1u, 10);
    a.try_emplace(9u, 90);
    tracked_map<> b(a, typename tracked_map<>::allocator_type{state, 2});
    REQUIRE(b.get_allocator().id == 2);
    b.at(1u) = 99;
    REQUIRE(a.at(1u) == 10);
    verify(a, {{1u, 10}, {9u, 90}});
    verify(b, {{1u, 99}, {9u, 90}});
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "sparse_map allocator-aware move constructor with equal allocator",
    "[common][sparse_map][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    using M = tracked_map<>;
    M a(M::allocator_type{state, 3});
    a.try_emplace(2u, 20);
    a.try_emplace(16u, 160);
    M b(std::move(a), M::allocator_type{state, 3});
    verify(b, {{2u, 20}, {16u, 160}});
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "sparse_map allocator-aware move constructor with different allocator",
    "[common][sparse_map][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    using M = tracked_map<>;
    M a(M::allocator_type{state, 3});
    a.try_emplace(2u, 20);
    a.try_emplace(16u, 160);
    M b(std::move(a), M::allocator_type{state, 4});
    verify(b, {{2u, 20}, {16u, 160}});
    REQUIRE(b.get_allocator().id == 4);
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "sparse_map copy assignment retains allocator without propagation",
    "[common][sparse_map][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    using M = tracked_map<false>;
    M src(M::allocator_type{state, 2}), dst(M::allocator_type{state, 3});
    src.try_emplace(2u, 20);
    src.try_emplace(9u, 90);
    dst.try_emplace(77u, 770);
    dst = src;
    REQUIRE(dst.get_allocator().id == 3);
    verify(dst, {{2u, 20}, {9u, 90}});
    verify(src, {{2u, 20}, {9u, 90}});
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "sparse_map move assignment retains allocator without propagation",
    "[common][sparse_map][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    using M = tracked_map<false>;
    M src(M::allocator_type{state, 2}), dst(M::allocator_type{state, 3});
    src.try_emplace(2u, 20);
    src.try_emplace(9u, 90);
    dst.try_emplace(77u, 770);
    dst = std::move(src);
    REQUIRE(dst.get_allocator().id == 3);
    verify(dst, {{2u, 20}, {9u, 90}});
  }
  REQUIRE(state->balanced());
}

TEST_CASE("sparse_map copy assignment propagates enabled allocator", "[common][sparse_map][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    using M = tracked_map<true>;
    M src(M::allocator_type{state, 2}), dst(M::allocator_type{state, 3});
    src.try_emplace(2u, 20);
    src.try_emplace(9u, 90);
    dst.try_emplace(77u, 770);
    dst = src;
    REQUIRE(dst.get_allocator().id == 2);
    verify(dst, {{2u, 20}, {9u, 90}});
  }
  REQUIRE(state->balanced());
}

TEST_CASE("sparse_map move assignment propagates enabled allocator", "[common][sparse_map][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    using M = tracked_map<true>;
    M src(M::allocator_type{state, 2}), dst(M::allocator_type{state, 3});
    src.try_emplace(2u, 20);
    src.try_emplace(9u, 90);
    dst.try_emplace(77u, 770);
    dst = std::move(src);
    REQUIRE(dst.get_allocator().id == 2);
    verify(dst, {{2u, 20}, {9u, 90}});
  }
  REQUIRE(state->balanced());
}

TEST_CASE("sparse_map swap equal nonpropagating allocators", "[common][sparse_map][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    using M = tracked_map<false>;
    M a(M::allocator_type{state, 1}), b(M::allocator_type{state, 1});
    a.try_emplace(3u, 30);
    b.try_emplace(9u, 90);
    swap(a, b);
    verify(a, {{9u, 90}});
    verify(b, {{3u, 30}});
  }
  REQUIRE(state->balanced());
}

TEST_CASE("sparse_map swap distinct propagating allocators", "[common][sparse_map][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    using M = tracked_map<true>;
    M a(M::allocator_type{state, 1}), b(M::allocator_type{state, 2});
    a.try_emplace(3u, 30);
    b.try_emplace(9u, 90);
    swap(a, b);
    REQUIRE(a.get_allocator().id == 2);
    REQUIRE(b.get_allocator().id == 1);
    verify(a, {{9u, 90}});
    verify(b, {{3u, 30}});
  }
  REQUIRE(state->balanced());
}

TEST_CASE(
    "sparse_map allocator balances allocations across repeated rehash-like growth",
    "[common][sparse_map][allocator]")
{
  auto state = std::make_shared<allocation_log>();
  {
    tracked_map<> m(typename tracked_map<>::allocator_type{state, 1});
    for (unsigned i = 0;i<350;++i)
      m.try_emplace(i*31u, int(i));
    for (unsigned i = 0;i<350;i += 3)
      REQUIRE(m.try_erase(i*31u));
    for (unsigned i = 0;i<350;i += 3)
      REQUIRE(m.try_emplace(i*31u, -int(i)).second);
  }
  REQUIRE(state->balanced());
}
