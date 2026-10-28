#include "support.hpp"

using namespace heim_test::sparse_index;

template<typename I>
void basic_semantics()
{
  using UInt = key_type<I>;
  I idx{typename I::allocator_type{}};
  I const &cidx = idx;
  check_missing(idx, UInt{0});
  check_missing(cidx, UInt{0});
  check_missing(idx, UInt{7});
  check_missing(idx, UInt{100});
  idx.assure(UInt{7}) = UInt{13};
  check(idx, UInt{7}, UInt{13});
  check(cidx, UInt{7}, UInt{13});
  check_missing(idx, UInt{0});
  check_missing(idx, UInt{8});
  REQUIRE(&idx.assure(UInt{7}) == idx.get(UInt{7}));
  REQUIRE(&idx.assure(UInt{7}) == cidx.get(UInt{7}));
  check(idx, UInt{7}, UInt{13});
  idx.assure(UInt{7}) = UInt{0};
  check(idx, UInt{7}, UInt{0});
  idx.assure(UInt{7}) = UInt{91};
  check(idx, UInt{7}, UInt{91});
  idx.clear();
  check_missing(idx, UInt{7});
  check_missing(cidx, UInt{7});
  idx.clear();
  idx.assure(UInt{7}) = UInt{25};
  check(idx, UInt{7}, UInt{25});
}

TEST_CASE("paged index basic lookup and overwrite", "[common][sparse_index][basic]")
{
  basic_semantics<index_type<unsigned, 8>>();
}

TEST_CASE("contiguous index basic lookup and overwrite", "[common][sparse_index][basic]")
{
  basic_semantics<index_type<unsigned, 0>>();
}

TEST_CASE("page-size-one index basic lookup and overwrite", "[common][sparse_index][basic]")
{
  basic_semantics<index_type<unsigned, 1>>();
}

TEST_CASE("odd-page index basic lookup and overwrite", "[common][sparse_index][basic]")
{
  basic_semantics<index_type<unsigned, 7>>();
}

TEST_CASE(
    "sparse_index reserves the maximum unsigned value as a missing sentinel",
    "[common][sparse_index][basic]")
{
  index_type<unsigned, 8> paged{std::allocator<unsigned>{}};
  index_type<unsigned, 0> contiguous{std::allocator<unsigned>{}};
  paged.assure(3u) = null_value<decltype(paged)>;
  contiguous.assure(3u) = null_value<decltype(contiguous)>;
  check_missing(paged, 3u);
  check_missing(contiguous, 3u);
  REQUIRE(paged.assure(3u) == std::numeric_limits<unsigned>::max());
  REQUIRE(contiguous.assure(3u) == std::numeric_limits<unsigned>::max());
  paged.assure(3u) = 5u;
  contiguous.assure(3u) = 5u;
  check(paged, 3u, 5u);
  check(contiguous, 3u, 5u);
}

TEST_CASE("sparse_index distinguishes zero from the missing sentinel", "[common][sparse_index][basic]")
{
  index_type<unsigned, 8> paged{std::allocator<unsigned>{}};
  paged.assure(2u) = 0u;
  REQUIRE(paged.get(2u) != nullptr);
  REQUIRE(*paged.get(2u) == 0u);
  check_missing(paged, 1u);
}

TEST_CASE(
    "sparse_index assure preserves values until explicitly overwritten",
    "[common][sparse_index][basic]")
{
  index_type<unsigned, 8> idx{std::allocator<unsigned>{}};
  idx.assure(1u) = 12u;
  auto *const ptr = idx.get(1u);
  for (int n = 0; n < 50; ++n)
  {
    REQUIRE(&idx.assure(1u) == ptr);
    REQUIRE(*ptr == 12u);
  }
  idx.assure(1u) = 99u;
  REQUIRE(*ptr == 99u);
}

TEST_CASE(
    "sparse_index separates data from missing entries across repeated clears",
    "[common][sparse_index][basic]")
{
  index_type<unsigned, 8> idx{std::allocator<unsigned>{}};
  for (unsigned iteration = 0; iteration < 20; ++iteration)
  {
    check_missing(idx, 33u);
    idx.assure(33u) = iteration;
    check(idx, 33u, iteration);
    idx.clear();
  }
}
