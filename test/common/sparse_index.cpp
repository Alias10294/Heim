#include <catch2/catch_test_macros.hpp>
#include <heim/common/sparse.hpp>
#include <array>
#include <limits>
#include <utility>

namespace {
using paged = heim::detail::sparse_index<unsigned, 8>;
using contiguous = heim::detail::sparse_index<unsigned, 0>;
}

TEST_CASE("paged sparse_index only allocates pages on demand", "[common][sparse_index]")
{
  paged index{std::allocator<unsigned>{}};
  REQUIRE(index.get(0u) == nullptr);
  REQUIRE(index.get(400u) == nullptr);
  index.assure(400u) = 3u;
  REQUIRE(index.get(400u) != nullptr);
  REQUIRE(*index.get(400u) == 3u);
  REQUIRE(index.get(399u) == nullptr);
  REQUIRE(index.get(0u) == nullptr);
  index.assure(0u) = 6u;
  REQUIRE(*index.get(0u) == 6u);
  REQUIRE(*index.get(400u) == 3u);
}

TEST_CASE("sparse_index copy and assignment do not alias pages", "[common][sparse_index]")
{
  paged index{std::allocator<unsigned>{}};
  index.assure(2u) = 5u;
  index.assure(2049u) = 7u;
  paged copied{index};
  REQUIRE(*copied.get(2u) == 5u);
  REQUIRE(*copied.get(2049u) == 7u);
  copied.assure(2u) = 42u;
  REQUIRE(*index.get(2u) == 5u);
  paged assigned{std::allocator<unsigned>{}};
  assigned = index;
  REQUIRE(*assigned.get(2049u) == 7u);
  assigned.clear();
  REQUIRE(assigned.get(2u) == nullptr);
  REQUIRE(*index.get(2u) == 5u);
}

TEST_CASE("contiguous sparse_index preserves unset markers", "[common][sparse_index]")
{
  contiguous index{std::allocator<unsigned>{}};
  REQUIRE(index.get(9u) == nullptr);
  index.assure(9u) = 17u;
  REQUIRE(index.get(8u) == nullptr);
  REQUIRE(*index.get(9u) == 17u);
  auto copied = index;
  copied.assure(9u) = 18u;
  REQUIRE(*index.get(9u) == 17u);
  index.clear();
  REQUIRE(index.get(9u) == nullptr);
}
