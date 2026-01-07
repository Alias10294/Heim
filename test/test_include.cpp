#include "doctest.h"
#include "type_sequence.hpp"
#include "container.hpp"
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

  heim::container<std::string> c;
  CHECK_EQ(c.size (), std::size_t{0});
  CHECK_EQ(c.empty(), true          );
  CHECK_EQ(decltype(c)::page_size, 1024 );
  CHECK_EQ(decltype(c)::tag_value, false);
  CHECK_EQ(c.begin (), c.end ());
  CHECK_EQ(c.cbegin(), c.cend());
}