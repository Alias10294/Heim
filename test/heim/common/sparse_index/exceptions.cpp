#include "support.hpp"

using namespace heim_test::sparse_index;

TEST_CASE("failed page allocation does not create an initialized entry", "[common][sparse_index][exception]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<8> idx{tracked_allocator<unsigned>{state}};
    idx.assure(1u) = 11u;
    state->fail_page_allocation = true;
    REQUIRE_THROWS_AS(idx.assure(32u), std::bad_alloc);
    check_missing(idx, 32u);
    check(idx, 1u, 11u);
    REQUIRE(state->live_pages() == 1);
    state->fail_page_allocation = false;
    idx.assure(32u) = 55u;
    check(idx, 32u, 55u);
  }
  REQUIRE(state->balanced());
}

TEST_CASE("page allocation failure on first use leaves the index reusable", "[common][sparse_index][exception]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<8> idx{tracked_allocator<unsigned>{state}};
    state->fail_page_allocation = true;
    REQUIRE_THROWS_AS(idx.assure(100u), std::bad_alloc);
    check_missing(idx, 100u);
    REQUIRE(state->live_pages() == 0);
    state->fail_page_allocation = false;
    idx.assure(100u) = 2u;
    check(idx, 100u, 2u);
  }
  REQUIRE(state->balanced());
}

TEST_CASE("copy construction page-allocation failure leaves its source intact", "[common][sparse_index][exception]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<8> src{tracked_allocator<unsigned>{state}};
    src.assure(2u) = 20u;
    src.assure(200u) = 30u;
    REQUIRE(state->live_pages() == 2);
    state->fail_page_allocation = true;
    REQUIRE_THROWS_AS(observed_index<8>{src}, std::bad_alloc);
    state->fail_page_allocation = false;
    check(src, 2u, 20u);
    check(src, 200u, 30u);
    REQUIRE(state->live_pages() == 2);
  }
  REQUIRE(state->balanced());
}

TEST_CASE("copy assignment failure does not leak and leaves source usable", "[common][sparse_index][exception]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<8> src{tracked_allocator<unsigned>{state}};
    observed_index<8> dst{tracked_allocator<unsigned>{state}};
    src.assure(2u) = 20u;
    src.assure(200u) = 30u;
    dst.assure(5u) = 55u;
    state->fail_page_allocation = true;
    REQUIRE_THROWS_AS(dst = src, std::bad_alloc);
    state->fail_page_allocation = false;
    check(src, 2u, 20u);
    check(src, 200u, 30u);
    dst = src;
    check(dst, 2u, 20u);
    check(dst, 200u, 30u);
  }
  REQUIRE(state->balanced());
}

TEST_CASE("failing outer allocation preserves existing mappings", "[common][sparse_index][exception]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<8> idx{tracked_allocator<unsigned>{state}};
    idx.assure(0u) = 1u;
    state->fail_all_allocation = true;
    REQUIRE_THROWS_AS(idx.assure(8000u), std::bad_alloc);
    state->fail_all_allocation = false;
    check(idx, 0u, 1u);
    check_missing(idx, 8000u);
    idx.assure(8000u) = 9u;
    check(idx, 8000u, 9u);
  }
  REQUIRE(state->balanced());
}

TEST_CASE("contiguous growth allocation failure preserves old data", "[common][sparse_index][exception]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<0> idx{tracked_allocator<unsigned>{state}};
    idx.assure(5u) = 55u;
    state->fail_all_allocation = true;
    REQUIRE_THROWS_AS(idx.assure(2000u), std::bad_alloc);
    state->fail_all_allocation = false;
    check(idx, 5u, 55u);
    check_missing(idx, 2000u);
    idx.assure(2000u) = 33u;
    check(idx, 2000u, 33u);
  }
  REQUIRE(state->balanced());
}

TEST_CASE("reset failure releases old pages and remains reusable", "[common][sparse_index][exception]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<8> idx{tracked_allocator<unsigned>{state}};
    idx.assure(3u) = 30u;
    std::array<unsigned, 3> keys{2, 20, 99};
    state->fail_page_allocation = true;
    REQUIRE_THROWS_AS(idx.reset(keys), std::bad_alloc);
    state->fail_page_allocation = false;
    check_missing(idx, 3u);
    idx.reset(keys);
    check(idx, 2u, 0u);
    check(idx, 20u, 1u);
    check(idx, 99u, 2u);
  }
  REQUIRE(state->balanced());
}

TEST_CASE("failed allocation can be retried several times", "[common][sparse_index][exception]")
{
  auto state = std::make_shared<allocation_log>();
  {
    observed_index<8> idx{tracked_allocator<unsigned>{state}};
    for (int attempt = 0; attempt < 5; ++attempt)
    {
      state->fail_page_allocation = true;
      REQUIRE_THROWS_AS(idx.assure(100u), std::bad_alloc);
      check_missing(idx, 100u);
    }
    state->fail_page_allocation = false;
    idx.assure(100u) = 10u;
    check(idx, 100u, 10u);
  }
  REQUIRE(state->balanced());
}
