#include "support.hpp"

using namespace heim_test::sparse_index;

TEST_CASE("contiguous index initializes every gap during resize", "[common][sparse_index][contiguous]")
{
  index_type<unsigned, 0> idx{std::allocator<unsigned>{}};
  idx.assure(128u) = 7u;
  for (unsigned k = 0; k < 128; ++k)
    check_missing(idx, k);
  check(idx, 128u, 7u);
}

TEST_CASE("contiguous index survives many capacity expansions", "[common][sparse_index][contiguous]")
{
  index_type<unsigned, 0> idx{std::allocator<unsigned>{}};
  for (unsigned k = 0; k < 1000; k += 3)
    idx.assure(k) = 2u * k;
  for (unsigned k = 0; k < 1000; ++k)
    if (k % 3 == 0)
      check(idx, k, 2u * k);
    else check_missing(idx, k);
}

TEST_CASE("contiguous index supports forward and reverse filling", "[common][sparse_index][contiguous]")
{
  index_type<unsigned, 0> idx{std::allocator<unsigned>{}};
  for (unsigned k = 250; k > 0; --k)
    idx.assure(k - 1u) = k + 3u;
  for (unsigned k = 0; k < 250; ++k)
    check(idx, k, k + 4u);
}

TEST_CASE(
    "contiguous index sentinel changes are visible through const get",
    "[common][sparse_index][contiguous]")
{
  index_type<unsigned, 0> idx{std::allocator<unsigned>{}};
  index_type<unsigned, 0> const &cidx = idx;
  idx.assure(20u) = 4u;
  check(cidx, 20u, 4u);
  idx.assure(20u) = null_value<decltype(idx)>;
  check_missing(cidx, 20u);
  idx.assure(20u) = 0u;
  check(cidx, 20u, 0u);
}

TEST_CASE("contiguous index reset and clear discard old mappings", "[common][sparse_index][contiguous]")
{
  index_type<unsigned, 0> idx{std::allocator<unsigned>{}};
  idx.assure(0u) = 1u;
  idx.assure(15u) = 3u;
  idx.clear();
  check_missing(idx, 0u);
  check_missing(idx, 15u);
  idx.assure(15u) = 5u;
  check(idx, 15u, 5u);
  check_missing(idx, 0u);
}

TEST_CASE("contiguous small-integer index handles its maximum key", "[common][sparse_index][contiguous]")
{
  index_type<std::uint8_t, 0> idx{std::allocator<std::uint8_t>{}};
  idx.assure(static_cast<std::uint8_t>(255)) = static_cast<std::uint8_t>(42);
  idx.assure(static_cast<std::uint8_t>(0)) = static_cast<std::uint8_t>(1);
  check(idx, static_cast<std::uint8_t>(255), static_cast<std::uint8_t>(42));
  check(idx, static_cast<std::uint8_t>(0), static_cast<std::uint8_t>(1));
  check_missing(idx, static_cast<std::uint8_t>(254));
}
