#include "support.hpp"

#include <catch2/catch_test_macros.hpp>

#include <type_traits>
#include <unordered_map>
#include <utility>

using namespace heim_test::graph_algorithm::result;

static_assert(std::default_initializable<external_map>);
static_assert(std::copy_constructible<external_storage>);
static_assert(hp::map_for<external_storage::property_map_type &, int, int>);
static_assert(std::same_as<external_storage::property_map_type, external_map>);

TEST_CASE("result_storage uses a supplied map rather than its storage type",
          "[graph][algorithm][result_storage]")
{
  std::unordered_map<int, int> values;
  external_storage storage{external_map{&values}};
  auto map = storage.property_map();

  hp::set(map, 4, 40);
  REQUIRE(values.at(4) == 40);
  values.insert_or_assign(5, 50);
  REQUIRE(hp::get(map, 5) == 50);
}

TEST_CASE("result_storage external map keeps its caller-owned storage", "[graph][algorithm][result_storage]")
{
  std::unordered_map<int, int> values{{1, 10}};
  {
    external_storage storage{external_map{&values}};
    auto map = storage.property_map();
    REQUIRE(hp::get(map, 1) == 10);
    hp::set(map, 2, 20);
  }
  REQUIRE(values.at(1) == 10);
  REQUIRE(values.at(2) == 20);
}

TEST_CASE("result_storage external copies share their explicit map", "[graph][algorithm][result_storage]")
{
  std::unordered_map<int, int> values;
  external_storage first{external_map{&values}};
  external_storage second{first};

  auto first_map = first.property_map();
  auto second_map = second.property_map();
  hp::set(first_map, 7, 70);
  REQUIRE(hp::get(second_map, 7) == 70);
  REQUIRE(&hp::get(first_map, 7) == &hp::get(second_map, 7));
}

TEST_CASE("result_storage external map propagates lookup exceptions", "[graph][algorithm][result_storage]")
{
  std::unordered_map<int, int> values;
  external_storage storage{external_map{&values}};
  REQUIRE_THROWS_AS(hp::get(storage.property_map(), 999), std::out_of_range);
}

TEST_CASE("result_storage accepts a default-constructed custom map", "[graph][algorithm][result_storage]")
{
  using holder = hg::detail::result_storage<empty_storage, external_map>;
  holder storage;
  REQUIRE(storage.property_map().values == nullptr);
}
