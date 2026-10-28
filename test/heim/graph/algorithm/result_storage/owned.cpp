#include "support.hpp"

#include <catch2/catch_test_macros.hpp>

#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>

using namespace heim_test::graph_algorithm::result;

static_assert(std::default_initializable<owned_storage<>>);
static_assert(std::move_constructible<owned_storage<>>);
static_assert(!std::copy_constructible<owned_storage<>>);
static_assert(hp::map_for<owned_storage<>::property_map_type &, int, int>);
static_assert(
    std::same_as<owned_storage<>::property_map_type,
                 hg::detail::deduced_storage_t<int, int, hg::default_property_map_t>::property_map_type>);

TEST_CASE("result_storage default map starts empty and accepts writes", "[graph][algorithm][result_storage]")
{
  owned_storage<> storage;
  auto map = storage.property_map();

  REQUIRE_THROWS_AS(hp::get(map, 9), std::out_of_range);
  hp::set(map, 9, 90);
  REQUIRE(hp::get(map, 9) == 90);
  hp::set(map, 9, 91);
  REQUIRE(hp::get(map, 9) == 91);
}

TEST_CASE("result_storage property map copies refer to the same owned storage",
          "[graph][algorithm][result_storage]")
{
  owned_storage<> storage;
  auto first = storage.property_map();
  auto second = storage.property_map();

  hp::set(first, 4, 40);
  REQUIRE(&hp::get(first, 4) == &hp::get(second, 4));
  hp::set(second, 5, 50);
  REQUIRE(hp::get(first, 5) == 50);
}

TEST_CASE("result_storage moves preserve maps already retrieved from owned storage",
          "[graph][algorithm][result_storage]")
{
  owned_storage<> original;
  auto map = original.property_map();
  hp::set(map, 2, 20);

  owned_storage<> moved{std::move(original)};
  auto after_move = moved.property_map();

  REQUIRE(hp::get(after_move, 2) == 20);
  REQUIRE(&hp::get(after_move, 2) == &hp::get(map, 2));
  hp::set(after_move, 3, 30);
  REQUIRE(hp::get(map, 3) == 30);
}

TEST_CASE("result_storage independent defaults allocate independent maps",
          "[graph][algorithm][result_storage]")
{
  owned_storage<> first;
  owned_storage<> second;
  auto first_map = first.property_map();
  auto second_map = second.property_map();

  hp::set(first_map, 8, 80);
  REQUIRE_THROWS_AS(hp::get(second_map, 8), std::out_of_range);
  hp::set(second_map, 8, 800);
  REQUIRE(hp::get(first_map, 8) == 80);
}

TEST_CASE("result_storage default map works with non-integral keys", "[graph][algorithm][result_storage]")
{
  owned_storage<std::size_t, unsigned> storage;
  auto map = storage.property_map();

  hp::set(map, std::size_t{1000}, 42u);
  REQUIRE(hp::get(map, std::size_t{1000}) == 42u);
}
