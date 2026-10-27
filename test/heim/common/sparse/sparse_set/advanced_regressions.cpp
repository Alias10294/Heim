#include "support.hpp"

#if __has_include(<inplace_vector>)
#include <inplace_vector>

#endif

namespace hs = heim_test::sparse_set;

namespace
{
template<typename S>
void require_coherent_set(S const &s, std::initializer_list<typename S::key_type> probes = {})
{
  // Do not use the old contents as an oracle after failure. The current dense
  // contents define the candidate set; verify additionally checks index
  // lookups, uniqueness, stale mappings and size.
  std::unordered_set<typename S::key_type> observed{s.begin(), s.end()};
  hs::verify(s, observed, 256, probes);
}
}

TEST_CASE(
    "sparse_set extract leaves a coherent reusable source",
    "[common][sparse_set][extract][regression]")
{
  hs::set<> source{1u, 9u, 255u, 8000u};
  auto values = std::move(source).extract();
  REQUIRE((std::unordered_set<unsigned>{values.begin(), values.end()} ==
           std::unordered_set<unsigned>{1u, 9u, 255u, 8000u}));
  hs::verify(source, {}, 256, {1u, 9u, 255u, 8000u});
  REQUIRE(source.insert(1u).second);
  REQUIRE(source.insert(10000u).second);
  hs::verify(source, {1u, 10000u}, 256, {9u, 255u, 8000u});
}

TEST_CASE(
    "sparse_set unequal allocator move assignment clears its source",
    "[common][sparse_set][move][allocator][regression]")
{
  auto log = std::make_shared<hs::allocation_log>();
  {
    using S = hs::tracked_set<8, false>;
    S source{S::allocator_type{log, 1}}, dest{S::allocator_type{log, 2}};
    for (unsigned key : {1u, 9u, 255u, 900u})
      source.insert(key);
    dest.insert(3u);
    dest = std::move(source);
    hs::verify(dest, {1u, 9u, 255u, 900u}, 901, {3u});
    hs::verify(source, {}, 901, {1u, 9u, 255u, 900u});
    REQUIRE(source.insert(1u).second);
    hs::verify(dest, {1u, 9u, 255u, 900u}, 901);
  }
  REQUIRE(log->balanced());
}

TEST_CASE(
    "sparse_set failed copy assignment preserves sparse/dense invariants without propagation",
    "[common][sparse_set][exception][copy][regression]")
{
  using S = hs::tracked_set<8, false>;
  for (std::size_t budget = 0; budget < 6; ++budget)
  {
    auto log = std::make_shared<hs::allocation_log>();
    {
      S source{S::allocator_type{log, 1}}, dest{S::allocator_type{log, 2}};
      for (unsigned i = 0; i < 12; ++i)
        source.insert(i * 16u);
      dest.insert(7000u);
      log->remaining = budget;
      bool failed = false;
      try
      {
        dest = source;
      }
      catch (std::bad_alloc const &)
      {
        failed = true;
      }
      log->remaining = std::numeric_limits<std::size_t>::max();
      INFO("copy allocation budget=" << budget);
      hs::verify(source, {0u, 16u, 32u, 48u, 64u, 80u, 96u, 112u, 128u, 144u, 160u, 176u}, 177);
      if (failed)
        require_coherent_set(dest, {7000u, 176u});
      else
        hs::verify(dest, {0u, 16u, 32u, 48u, 64u, 80u, 96u, 112u, 128u, 144u, 160u, 176u}, 256, {7000u});
      REQUIRE(dest.get_allocator().owner == 2); // POCCA is false.
      dest.clear();
      REQUIRE(dest.insert(8000u).second);
      hs::verify(dest, {8000u}, 256, {7000u, 176u});
    }
    REQUIRE(log->balanced());
  }
}

TEST_CASE(
    "sparse_set copy assignment can propagate its allocator on failure",
    "[common][sparse_set][exception][allocator]")
{
  using S = hs::tracked_set<8, true>;
  auto log = std::make_shared<hs::allocation_log>();
  {
    S source{S::allocator_type{log, 1}}, dest{S::allocator_type{log, 2}};
    for (unsigned i = 0; i < 12; ++i)
      source.insert(i * 8u);
    dest.insert(7000u);
    log->remaining = 3;
    REQUIRE_THROWS_AS(dest = source, std::bad_alloc);
    log->remaining = std::numeric_limits<std::size_t>::max();
    require_coherent_set(dest, {7000u, 80u});
    REQUIRE(source.size() == 12);
    REQUIRE(source.contains(80u));
    dest.clear();
    REQUIRE(dest.insert(8000u).second);
    hs::verify(dest, {8000u}, 256, {7000u, 80u});
  }
  REQUIRE(log->balanced());
}

#if defined(__cpp_lib_inplace_vector) && __cpp_lib_inplace_vector >= 202406L
TEST_CASE("sparse_set supports containers without allocator_type", "[common][sparse_set][generic][inplace]")
{
  using S = heim::sparse_set<unsigned, 8, std::inplace_vector<unsigned, 64>>;
  static_assert(!std::uses_allocator_v<S::container_type, S::allocator_type>);
  S s;
  REQUIRE(s.insert(1u).second);
  REQUIRE(s.insert(17u).second);
  hs::verify(s, {1u, 17u}, 18);
  S copied{s, S::allocator_type{}};
  hs::verify(copied, {1u, 17u}, 18);
  S moved{std::move(copied), S::allocator_type{}};
  hs::verify(moved, {1u, 17u}, 18);
  REQUIRE(s.try_erase(1u));
  hs::verify(s, {17u}, 18);
}
#endif
