#include "support.hpp"

using namespace heim_test::sparse_index;

template<typename UInt, std::size_t PageSize>
void integral_limits()
{
  index_type<UInt, PageSize> idx{std::allocator<UInt>{}};
  auto const hi = std::numeric_limits<UInt>::max();
  auto const penultimate = static_cast<UInt>(hi - 1);
  idx.assure(UInt{0}) = static_cast<UInt>(1);
  idx.assure(penultimate) = static_cast<UInt>(2);
  idx.assure(hi) = static_cast<UInt>(3);
  check(idx, UInt{0}, static_cast<UInt>(1));
  check(idx, penultimate, static_cast<UInt>(2));
  check(idx, hi, static_cast<UInt>(3));
  check_missing(idx, static_cast<UInt>(hi - 2));
}

TEST_CASE("uint8_t contiguous index handles maximum key", "[common][sparse_index][limits]")
{
  integral_limits<std::uint8_t, 0>();
}

TEST_CASE("uint8_t paged index handles maximum key", "[common][sparse_index][limits]")
{
  integral_limits<std::uint8_t, 8>();
}

TEST_CASE("uint8_t page-size-one index handles maximum key", "[common][sparse_index][limits]")
{
  integral_limits<std::uint8_t, 1>();
}

TEST_CASE("uint16_t paged index handles maximum key and page boundary", "[common][sparse_index][limits]")
{
  integral_limits<std::uint16_t, 8>();
}

TEST_CASE("uint16_t contiguous index handles maximum key", "[common][sparse_index][limits]")
{
  integral_limits<std::uint16_t, 0>();
}

TEST_CASE(
    "uint64_t paged index supports sparse high keys without dense allocation",
    "[common][sparse_index][limits]")
{
  index_type<std::uint64_t, 8> idx{std::allocator<std::uint64_t>{}};
  constexpr std::uint64_t high = 100000;
  idx.assure(high) = 5;
  check(idx, high, std::uint64_t{5});
  check_missing(idx, high - 1);
  check_missing(idx, std::uint64_t{0});
}

TEST_CASE(
    "uint8_t reserved null marker can be overwritten by a valid value",
    "[common][sparse_index][limits]")
{
  index_type<std::uint8_t, 8> idx{std::allocator<std::uint8_t>{}};
  auto const null = std::numeric_limits<std::uint8_t>::max();
  idx.assure(std::uint8_t{8}) = null;
  check_missing(idx, std::uint8_t{8});
  idx.assure(std::uint8_t{8}) = std::uint8_t{254};
  check(idx, std::uint8_t{8}, std::uint8_t{254});
}

TEST_CASE("high absent keys are safe to query without growing storage", "[common][sparse_index][limits]")
{
  auto state = std::make_shared<allocation_log>();
  observed_index<8> idx{tracked_allocator<unsigned>{state}};
  for (unsigned key : {0u, 1u, 5000u, 100000u})
  {
    check_missing(idx, key);
    REQUIRE(state->outstanding.empty());
  }
}
