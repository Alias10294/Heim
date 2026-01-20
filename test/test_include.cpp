#include "doctest.h"
#include "type_sequence.hpp"
#include "storage/sparse_set/pool.hpp"
#include <string>

TEST_CASE("test")
{
  using test_t
  = heim::type_sequence<short, int, long>;

  CHECK_EQ(test_t::size, 3);
  CHECK_EQ(test_t::template contains<short >, true );
  CHECK_EQ(test_t::template contains<int   >, true );
  CHECK_EQ(test_t::template contains<long  >, true );
  CHECK_EQ(test_t::template contains<float >, false);
  CHECK_EQ(test_t::template contains<double>, false);
  CHECK_EQ(test_t::is_unique, true);

  heim::sparse_set_based::pool<std::string> p;
  CHECK_EQ(p.size (), 0);
  CHECK_EQ(p.empty(), true);
  CHECK_EQ(decltype(p)::page_size, 1024 );
  CHECK_EQ(decltype(p)::tag_value, false);
  CHECK_EQ(p.begin (), p.end ());
  CHECK_EQ(p.cbegin(), p.cend());
}