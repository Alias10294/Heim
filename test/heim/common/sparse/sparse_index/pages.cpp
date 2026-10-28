#include "support.hpp"

using namespace heim_test::sparse_index;

template<std::size_t N>
void boundaries()
{
  static_assert(N >= 1);
  index<unsigned, N> idx{std::allocator<unsigned>{}};
  std::unordered_map<unsigned, unsigned> written;
  auto put = [&](unsigned key, unsigned value)
  {
    idx.assure(key) = value;
    written[key] = value;
  };
  auto const n = static_cast<unsigned>(N);
  put(0u, 11u);
  put(n - 1, 12u);
  put(n, 13u);
  put(n + 1, 14u);
  put(2 * n - 1, 15u);
  put(2 * n, 16u);
  put(5 * n, 17u);
  for (auto const &[key, value] : written)
    check(idx, key, value);
  check_missing(idx, 5 * n + 1);
  check_missing(idx, 4 * n);
  idx.clear();
  for (unsigned k = 0; k <= 5 * n + 1; ++k)
    check_missing(idx, k);
}

TEST_CASE("one-element pages handle every page boundary", "[common][sparse_index][pages]")
{
  boundaries<1>();
}

TEST_CASE("two-element pages handle every page boundary", "[common][sparse_index][pages]")
{
  boundaries<2>();
}

TEST_CASE("three-element pages handle every page boundary", "[common][sparse_index][pages]")
{
  boundaries<3>();
}

TEST_CASE("seven-element pages handle every page boundary", "[common][sparse_index][pages]")
{
  boundaries<7>();
}

TEST_CASE("eight-element pages handle every page boundary", "[common][sparse_index][pages]")
{
  boundaries<8>();
}

TEST_CASE("sixteen-element pages handle every page boundary", "[common][sparse_index][pages]")
{
  boundaries<16>();
}

TEST_CASE("31-element pages handle every page boundary", "[common][sparse_index][pages]")
{
  boundaries<31>();
}

TEST_CASE("64-element pages handle every page boundary", "[common][sparse_index][pages]")
{
  boundaries<64>();
}

TEST_CASE("default page size supports distant sparse entries", "[common][sparse_index][pages]")
{
  index<unsigned, heim::default_page_size_v> idx{std::allocator<unsigned>{}};
  auto const n = static_cast<unsigned>(heim::default_page_size_v);
  idx.assure(0u) = 4u;
  idx.assure(n) = 9u;
  idx.assure(5 * n + 1) = 21u;
  check(idx, 0u, 4u);
  check(idx, n, 9u);
  check(idx, 5 * n + 1, 21u);
  check_missing(idx, 3 * n);
}

TEST_CASE("paged storage never aliases entries in distinct pages", "[common][sparse_index][pages]")
{
  index<unsigned, 8> idx{std::allocator<unsigned>{}};
  idx.assure(0u) = 10u;
  idx.assure(8u) = 20u;
  idx.assure(16u) = 30u;
  REQUIRE(idx.get(0u) != idx.get(8u));
  REQUIRE(idx.get(8u) != idx.get(16u));
  REQUIRE(idx.get(0u) != idx.get(16u));
}

TEST_CASE("paged storage preserves addresses when the page table grows", "[common][sparse_index][pages]")
{
  index<unsigned, 8> idx{std::allocator<unsigned>{}};
  idx.assure(1u) = 123u;
  idx.assure(9u) = 456u;
  unsigned *original = idx.get(1u);
  unsigned *second = idx.get(9u);
  REQUIRE(original != nullptr);
  REQUIRE(second != nullptr);
  for (unsigned page = 2; page < 300; ++page)
  {
    idx.assure(8u * page) = page;
    REQUIRE(idx.get(1u) == original);
    REQUIRE(idx.get(9u) == second);
  }
  REQUIRE(*original == 123u);
  REQUIRE(*second == 456u);
}

TEST_CASE("page allocation initializes all other entries as missing", "[common][sparse_index][pages]")
{
  index<unsigned, 31> idx{std::allocator<unsigned>{}};
  idx.assure(3u) = 19u;
  for (unsigned key = 0; key < 31u; ++key)
    if (key == 3u)
      check(idx, key, 19u);
    else check_missing(idx, key);
}

TEST_CASE(
    "all intermediate pages remain unallocated after one distant write",
    "[common][sparse_index][pages]")
{
  auto log = std::make_shared<allocation_log>();
  observed_index<8> idx{tracked_allocator<unsigned>{log, 3}};
  idx.assure(8u * 4096u) = 7u;
  REQUIRE(log->page_allocations == 1);
  REQUIRE(log->live_pages() == 1);
  check_missing(idx, 0u);
  check_missing(idx, 8u * 1024u);
  check(idx, 8u * 4096u, 7u);
  idx.assure(8u * 1024u + 5u) = 8u;
  REQUIRE(log->page_allocations == 2);
  REQUIRE(log->live_pages() == 2);
}

TEST_CASE("multiple keys within one page require one page allocation", "[common][sparse_index][pages]")
{
  auto log = std::make_shared<allocation_log>();
  observed_index<8> idx{tracked_allocator<unsigned>{log}};
  for (unsigned k = 16; k < 24; ++k)
    idx.assure(k) = k;
  REQUIRE(log->page_allocations == 1);
  REQUIRE(log->live_pages() == 1);
  for (unsigned k = 16; k < 24; ++k)
    check(idx, k, k);
}

TEST_CASE("rewriting one page does not allocate again", "[common][sparse_index][pages]")
{
  auto log = std::make_shared<allocation_log>();
  observed_index<8> idx{tracked_allocator<unsigned>{log}};
  idx.assure(7u) = 1u;
  auto before = log->page_allocations;
  for (unsigned value = 0; value < 200; ++value)
    idx.assure(7u) = value;
  REQUIRE(log->page_allocations == before);
  REQUIRE(log->live_pages() == 1);
  check(idx, 7u, 199u);
}

TEST_CASE("clearing paged storage releases its pages", "[common][sparse_index][pages]")
{
  auto log = std::make_shared<allocation_log>();
  {
    observed_index<8> idx{tracked_allocator<unsigned>{log}};
    for (unsigned page = 0; page < 20; ++page)
      idx.assure(8u * page) = page;
    REQUIRE(log->live_pages() == 20);
    idx.clear();
    REQUIRE(log->live_pages() == 0);
    for (unsigned page = 0; page < 20; ++page)
      check_missing(idx, 8u * page);
  }
  REQUIRE(log->balanced());
}

TEST_CASE("large sparse gaps preserve existing values and missing markers", "[common][sparse_index][pages]")
{
  index<unsigned, 8> idx{std::allocator<unsigned>{}};
  idx.assure(1u) = 11u;
  idx.assure(200000u) = 22u;
  check(idx, 1u, 11u);
  check(idx, 200000u, 22u);
  check_missing(idx, 100000u);
  check_missing(idx, 199999u);
  check_missing(idx, 200001u);
}
