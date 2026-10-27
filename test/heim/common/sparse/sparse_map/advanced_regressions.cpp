#include "support.hpp"

#if __has_include(<inplace_vector>)
#include <inplace_vector>

#endif

using namespace heim_test::sparse_map;

namespace
{
template<typename M>
void require_coherent_map(M const &m, std::initializer_list<typename M::key_type> probes = {})
{
  // The map may keep old, new, partial or no elements after an exception.
  // Its current dense associations must still agree with every sparse lookup.
  // An invalid key/value alignment must fail this check, not be overlooked.
  verify(m, snapshot(m), 256, probes);
}
}

TEST_CASE(
    "sparse_map extract leaves a coherent reusable source",
    "[common][sparse_map][extract][regression]")
{
  map<> source{{1u, 10}, {9u, 90}, {255u, 250}, {8000u, 80}};
  auto extracted = std::move(source).extract();
  REQUIRE(extracted.keys.size() == extracted.values.size());
  std::unordered_map<unsigned, int> contents;
  for (std::size_t i = 0; i < extracted.keys.size(); ++i)
    REQUIRE(contents.emplace(extracted.keys[i], extracted.values[i]).second);
  REQUIRE((contents == std::unordered_map<unsigned, int>{{1u, 10}, {9u, 90}, {255u, 250}, {8000u, 80}}));
  verify(source, {}, 256, {1u, 9u, 255u, 8000u});
  REQUIRE(source.try_emplace(1u, 111).second);
  REQUIRE(source.try_emplace(10000u, 100).second);
  verify(source, {{1u, 111}, {10000u, 100}}, 256, {9u, 255u, 8000u});
}

TEST_CASE(
    "sparse_map unequal allocator move assignment clears its source",
    "[common][sparse_map][move][allocator][regression]")
{
  auto log = std::make_shared<allocation_log>();
  {
    using M = tracked_map<false>;
    M source{M::allocator_type{log, 1}}, dest{M::allocator_type{log, 2}};
    source.try_emplace(1u, 10);
    source.try_emplace(255u, 20);
    source.try_emplace(900u, 30);
    dest.try_emplace(3u, 30);
    dest = std::move(source);
    verify(dest, {{1u, 10}, {255u, 20}, {900u, 30}}, 901, {3u});
    verify(source, {}, 901, {1u, 255u, 900u});
    REQUIRE(source.try_emplace(1u, 111).second);
    verify(dest, {{1u, 10}, {255u, 20}, {900u, 30}}, 901);
  }
  REQUIRE(log->balanced());
}

TEST_CASE(
    "sparse_map failed copy assignment preserves key/value/index invariants",
    "[common][sparse_map][exception][copy][regression]")
{
  using M = tracked_map<false>;
  for (std::size_t budget = 0; budget < 7; ++budget)
  {
    auto log = std::make_shared<allocation_log>();
    {
      M source{M::allocator_type{log, 1}}, dest{M::allocator_type{log, 2}};
      for (unsigned i = 0; i < 12; ++i)
        source.try_emplace(i * 16u, static_cast<int>(i));
      dest.try_emplace(7000u, 70);
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
      REQUIRE(source.size() == 12);
      REQUIRE(source.at(176u) == 11);
      if (failed)
        require_coherent_map(dest, {176u, 7000u});
      else
      {
        REQUIRE(dest.size() == 12);
        REQUIRE(dest.at(176u) == 11);
        REQUIRE_FALSE(dest.contains(7000u));
      }
      REQUIRE(dest.get_allocator().id == 2); // POCCA is false.
      dest.clear();
      REQUIRE(dest.try_emplace(8000u, 80).second);
      verify(dest, {{8000u, 80}}, 256, {176u, 7000u});
    }
    REQUIRE(log->balanced());
  }
}

TEST_CASE(
    "sparse_map propagating copy assignment failure leaves a reusable map",
    "[common][sparse_map][exception][allocator]")
{
  using M = tracked_map<true>;
  auto log = std::make_shared<allocation_log>();
  {
    M source{M::allocator_type{log, 1}}, dest{M::allocator_type{log, 2}};
    for (unsigned i = 0; i < 12; ++i)
      source.try_emplace(i * 8u, static_cast<int>(i));
    dest.try_emplace(7000u, 70);
    log->remaining = 3;
    REQUIRE_THROWS_AS(dest = source, std::bad_alloc);
    log->remaining = std::numeric_limits<std::size_t>::max();
    require_coherent_map(dest, {7000u, 80u});
    REQUIRE(source.size() == 12);
    REQUIRE(source.at(80u) == 10);
    dest.clear();
    REQUIRE(dest.try_emplace(8000u, 80).second);
    verify(dest, {{8000u, 80}}, 256, {7000u, 80u});
  }
  REQUIRE(log->balanced());
}

namespace
{
struct copy_relocation_probe
{
  int value;
  static inline bool reject_copy = false;
  explicit copy_relocation_probe(int x) : value(x) {}
  copy_relocation_probe(copy_relocation_probe const &rhs) : value(rhs.value)
  { if (reject_copy) throw std::runtime_error{"relocation copy"}; }
  copy_relocation_probe(copy_relocation_probe && rhs) noexcept(false) : value(rhs.value) {}
  copy_relocation_probe &operator= (copy_relocation_probe const &) = default;
  copy_relocation_probe &operator= (copy_relocation_probe && ) = default;
};
}

TEST_CASE(
    "sparse_map mapped vector growth can recover after an old-value copy throws",
    "[common][sparse_map][exception][mapped]")
{
  map<unsigned, copy_relocation_probe> m;
  m.try_emplace(1u, 10);
  m.try_emplace(2u, 20);
  copy_relocation_probe::reject_copy = true;
  bool failed = false;
  try
  {
    m.try_emplace(3u, 30);
  }
  catch (std::runtime_error const &)
  {
    failed = true;
  }
  copy_relocation_probe::reject_copy = false;
  REQUIRE(failed);
  REQUIRE(m.size() == 2);
  REQUIRE(m.at(1u).value == 10);
  REQUIRE(m.at(2u).value == 20);
  REQUIRE_FALSE(m.contains(3u));
  REQUIRE(m.try_emplace(3u, 30).second);
  REQUIRE(m.at(3u).value == 30);
}

#if defined(__cpp_lib_inplace_vector) && __cpp_lib_inplace_vector >= 202406L
TEST_CASE("sparse_map supports an allocator-free key container", "[common][sparse_map][generic][inplace]")
{
  using M = heim::sparse_map<unsigned, int, 8,
      std::inplace_vector<unsigned, 64>, std::vector<int>>;
  M m;
  REQUIRE(m.try_emplace(1u, 10).second);
  REQUIRE(m.try_emplace(17u, 170).second);
  REQUIRE(m.at(17u) == 170);
  M copied{m, M::allocator_type{}};
  REQUIRE(copied.at(1u) == 10);
  REQUIRE(copied.at(17u) == 170);
  M moved{std::move(copied), M::allocator_type{}};
  REQUIRE(moved.at(17u) == 170);
  REQUIRE(m.try_erase(1u));
  REQUIRE_FALSE(m.contains(1u));
}
#endif
