#include <catch2/catch_test_macros.hpp>

#include <heim/meta.hpp>

#include "support.hpp"

TEST_CASE("type_sequence public traits match corresponding metafunctions", "[meta][type_sequence]")
{
  using sequence = heim_meta_test::repeated;
  REQUIRE(sequence::size == heim::type_sequence_size_v<sequence>);
  REQUIRE(sequence::empty == heim::type_sequence_empty_v<sequence>);
  REQUIRE(sequence::is_unique == heim::type_sequence_is_unique_v<sequence>);
  REQUIRE((sequence::index<heim_meta_test::c> == heim::type_sequence_index_v<sequence, heim_meta_test::c>));
  REQUIRE((sequence::count<heim_meta_test::a> == heim::type_sequence_count_v<sequence, heim_meta_test::a>));
  REQUIRE((sequence::contains<heim_meta_test::b> == heim::type_sequence_contains_v<sequence,
      heim_meta_test::b>));
}
