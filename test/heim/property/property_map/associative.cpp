#include <catch2/catch_test_macros.hpp>
#include <heim/property.hpp>

#include <concepts>
#include <flat_map>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace
{
namespace prop = heim::properties;

template<typename Container>
void verify_mutable_associative_container()
{
  Container container;
  container.insert_or_assign(1, 10);
  auto tagged = heim::make_property_map(heim::associative_container_arg, container);
  auto automatic = heim::make_property_map(container);

  static_assert(prop::map_for<decltype(tagged) &, int, int>);
  static_assert(prop::map_for<decltype(automatic) &, int, int>);
  REQUIRE(&prop::get(tagged, 1) == &container.at(1));
  REQUIRE(prop::get(automatic, 1) == 10);
  prop::set(tagged, 1, 20);
  REQUIRE(container.at(1) == 20);
  prop::set(automatic, 3, 30);
  REQUIRE(container.at(3) == 30);
  REQUIRE_THROWS_AS(prop::get(tagged, 99), std::out_of_range);
  REQUIRE_THROWS_AS(prop::get(automatic, 99), std::out_of_range);
}

template<typename Container>
void verify_const_associative_container()
{
  Container mutable_container;
  mutable_container.insert_or_assign(1, 10);
  Container const &container = mutable_container;
  auto tagged = heim::make_property_map(heim::associative_container_arg, container);
  auto automatic = heim::make_property_map(container);
  static_assert(prop::readable_map_for<decltype(tagged) &, int>);
  static_assert(prop::readable_map_for<decltype(automatic) &, int>);
  static_assert(!prop::writable_map_for<decltype(tagged) &, int, int>);
  static_assert(!prop::writable_map_for<decltype(automatic) &, int, int>);
  static_assert(std::same_as<decltype(prop::get(tagged, 1)), int const &>);
  REQUIRE(&prop::get(tagged, 1) == &container.at(1));
  REQUIRE(prop::get(automatic, 1) == 10);
}
}

TEST_CASE("map adapter reads, inserts, and replaces", "[property][property_map][associative]")
{
  verify_mutable_associative_container<std::map<int, int>>();
  verify_const_associative_container<std::map<int, int>>();
}

TEST_CASE("unordered_map adapter reads, inserts, and replaces", "[property][property_map][associative]")
{
  verify_mutable_associative_container<std::unordered_map<int, int>>();
  verify_const_associative_container<std::unordered_map<int, int>>();
}

TEST_CASE("flat_map adapter reads, inserts, and replaces", "[property][property_map][associative]")
{
  verify_mutable_associative_container<std::flat_map<int, int>>();
  verify_const_associative_container<std::flat_map<int, int>>();
}

TEST_CASE("associative adapter forwards mapped values without copying", "[property][property_map][associative]")
{
  std::map<int, std::string> container{{1, "before"}};
  auto map = heim::make_property_map(container);
  prop::get(map, 1) += "-updated";
  REQUIRE(container.at(1) == "before-updated");
  prop::set(map, 2, "new");
  REQUIRE(container.at(2) == "new");
}

TEST_CASE("associative adapter propagates container exceptions", "[property][property_map][associative]")
{
  std::unordered_map<int, int> container;
  auto map = heim::make_property_map(container);
  REQUIRE_THROWS_AS(prop::get(map, -1), std::out_of_range);
  prop::set(map, -1, 5);
  REQUIRE(prop::get(map, -1) == 5);
}
