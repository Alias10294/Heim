#include "support.hpp"

namespace hs = heim_test::sparse_set;

TEST_CASE("sparse_set paged index allocation failure preserves existing keys", "[common][sparse_set][exceptions]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    hs::tracked_set<8> s{hs::tracked_allocator<unsigned>{log, 3}};
    s.insert(1u);
    log->reject_pages = true;
    REQUIRE_THROWS_AS(s.insert(17u), std::bad_alloc);
    log->reject_pages = false;
    hs::verify(s, {1u});
    REQUIRE(s.insert(17u).second);
    hs::verify(s, {1u, 17u});
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set dense container allocation failure leaves key missing", "[common][sparse_set][exceptions]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    hs::tracked_set<8> s{hs::tracked_allocator<unsigned>{log, 3}};
    s.insert(1u);
    log->reject_dense = true;
    REQUIRE_THROWS_AS(s.insert(2u), std::bad_alloc);
    log->reject_dense = false;
    hs::verify(s, {1u});
    REQUIRE(s.insert(2u).second);
    hs::verify(s, {1u, 2u});
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set contiguous growth failure preserves previous membership", "[common][sparse_set][exceptions]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    hs::tracked_set<0> s{hs::tracked_allocator<unsigned>{log, 3}};
    s.insert(1u);
    log->reject_all = true;
    REQUIRE_THROWS_AS(s.insert(1024u), std::bad_alloc);
    log->reject_all = false;
    hs::verify(s, {1u});
    REQUIRE(s.insert(1024u).second);
    hs::verify(s, {1u, 1024u});
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set can recover from index allocation failure on an empty set", "[common][sparse_set][exceptions]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    hs::tracked_set<8> s{hs::tracked_allocator<unsigned>{log, 3}};
    log->reject_all = true;
    REQUIRE_THROWS_AS(s.insert(1u), std::bad_alloc);
    log->reject_all = false;
    hs::verify(s, {});
    REQUIRE(s.insert(1u).second);
    hs::verify(s, {1u});
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set replacement construction failure preserves the original", "[common][sparse_set][exceptions]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    using alloc = hs::tracked_allocator<unsigned>;
    hs::tracked_set<8> s{alloc{log, 1}};
    s.insert(1u);
    std::vector<unsigned, alloc> incoming{{2u, 3u, 17u}, alloc{log, 1}};
    log->reject_pages = true;
    REQUIRE_THROWS_AS(s.replace(std::move(incoming)), std::bad_alloc);
    log->reject_pages = false;
    hs::verify(s, {1u});
    REQUIRE(s.insert(17u).second);
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set unique-tag replacement failure preserves the original", "[common][sparse_set][exceptions]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    using alloc = hs::tracked_allocator<unsigned>;
    hs::tracked_set<8> s{alloc{log, 1}};
    s.insert(1u);
    std::vector<unsigned, alloc> incoming{{2u, 3u, 17u}, alloc{log, 1}};
    log->reject_pages = true;
    REQUIRE_THROWS_AS(s.replace(heim::unique, std::move(incoming)), std::bad_alloc);
    log->reject_pages = false;
    hs::verify(s, {1u});
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set construction failure releases all intermediate allocations", "[common][sparse_set][exceptions]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    using alloc = hs::tracked_allocator<unsigned>;
    auto const values = std::vector<unsigned>{1u, 9u, 17u, 25u};
    log->remaining = 3;
    REQUIRE_THROWS_AS((hs::tracked_set<8>{values.begin(), values.end(), alloc{log, 1}}),
                      std::bad_alloc);
    log->remaining = std::numeric_limits<std::size_t>::max();
  }
  REQUIRE(log->balanced());
}

TEST_CASE("sparse_set failed page allocation does not leak empty page slots", "[common][sparse_set][exceptions]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    hs::tracked_set<8> s{hs::tracked_allocator<unsigned>{log, 3}};
    s.insert(1u);
    for (unsigned repeat = 0; repeat < 6; ++repeat)
    {
      log->reject_pages = true;
      REQUIRE_THROWS_AS(s.insert(79u), std::bad_alloc);
      log->reject_pages = false;
      hs::verify(s, {1u});
    }
    REQUIRE(s.insert(79u).second);
  }
  REQUIRE(log->balanced());
}
