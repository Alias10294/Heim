#include <catch2/catch_test_macros.hpp>
#include <heim/common/sparse.hpp>
#include <algorithm>
#include <cstdint>
#include <numeric>
#include <random>
#include <unordered_set>
#include <vector>
#include <utility>

namespace {
using set_type = heim::sparse_set<unsigned>;

void check_against(set_type const& actual, std::unordered_set<unsigned> const& expected)
{
  REQUIRE(actual.size() == expected.size());
  for (auto key : expected) {
    INFO("key=" << key);
    REQUIRE(actual.contains(key));
    REQUIRE(actual.find(key) != actual.end());
  }
  std::unordered_set<unsigned> iterated;
  for (auto key : actual) {
    REQUIRE(expected.contains(key));
    REQUIRE(iterated.insert(key).second);
  }
  REQUIRE(iterated == expected);
}
}

TEST_CASE("sparse_set starts empty and supports gaps in key space", "[common][sparse_set]")
{
  set_type s;
  REQUIRE(s.empty());
  REQUIRE(s.find(17) == s.end());
  REQUIRE_FALSE(s.try_erase(17));
  for (auto key : {0u, 1u, 128u, 4096u}) REQUIRE(s.insert(key).second);
  check_against(s, {0u, 1u, 128u, 4096u});
  REQUIRE_FALSE(s.insert(128u).second);
  REQUIRE_FALSE(s.try_emplace(1u).second);
  REQUIRE(s.size() == 4);
}

TEST_CASE("sparse_set deletion fixes the dense-index mapping", "[common][sparse_set]")
{
  set_type s{10u, 20u, 30u, 40u, 50u};
  REQUIRE(s.try_erase(30));
  check_against(s, {10u, 20u, 40u, 50u});
  REQUIRE(s.try_erase(50));
  REQUIRE(s.try_erase(10));
  REQUIRE_FALSE(s.try_erase(10));
  check_against(s, {20u, 40u});
  REQUIRE(s.insert(30u).second);
  REQUIRE(s.insert(10u).second);
  check_against(s, {10u, 20u, 30u, 40u});
}

TEST_CASE("sparse_set copy, move, swap and clear preserve membership", "[common][sparse_set]")
{
  set_type src{0u, 7u, 77u};
  set_type copy{src};
  REQUIRE(copy == src);
  REQUIRE(copy.try_erase(7));
  REQUIRE(src.contains(7));
  set_type moved{std::move(copy)};
  check_against(moved, {0u, 77u});
  set_type other{4u, 5u};
  swap(moved, other);
  check_against(moved, {4u, 5u});
  check_against(other, {0u, 77u});
  other.clear();
  REQUIRE(other.empty());
  REQUIRE_FALSE(other.contains(77));
  REQUIRE(other.insert(77u).second);
}

TEST_CASE("sparse_set range insertion and replacement deduplicate keys", "[common][sparse_set]")
{
  set_type s;
  std::vector<unsigned> keys{8, 4, 8, 9, 4, 10};
  s.insert_range(keys);
  check_against(s, {4u, 8u, 9u, 10u});
  s.replace(std::vector<unsigned>{7, 7, 3, 1, 3});
  check_against(s, {1u, 3u, 7u});
}

TEST_CASE("sparse_set deterministic model-based mutation regression", "[common][sparse_set][model]")
{
  set_type s;
  std::unordered_set<unsigned> oracle;
  std::mt19937 rng{0x5E7A42u};
  for (unsigned step = 0; step < 3000; ++step) {
    const unsigned key = rng() % 700;
    if (rng() % 3 == 0) {
      REQUIRE(s.try_erase(key) == (oracle.erase(key) != 0));
    } else {
      REQUIRE(s.insert(key).second == oracle.insert(key).second);
    }
    if (step % 97 == 0) check_against(s, oracle);
  }
  check_against(s, oracle);
}
