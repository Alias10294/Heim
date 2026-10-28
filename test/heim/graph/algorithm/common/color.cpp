#include <heim/graph/algorithm/color.hpp>

#include <catch2/catch_test_macros.hpp>

#include <concepts>
#include <cstdint>
#include <type_traits>

namespace
{
namespace hg = heim::graphs;

enum class custom_color : unsigned
{
  white = 17,
  gray = 42,
  black = 99
};

static_assert(std::same_as<std::underlying_type_t<hg::color>, std::uint8_t>);
static_assert(hg::color::white != hg::color::gray);
static_assert(hg::color::gray != hg::color::black);
static_assert(hg::colors::white_v<hg::color> == hg::color::white);
static_assert(hg::colors::gray_v<hg::color> == hg::color::gray);
static_assert(hg::colors::black_v<hg::color> == hg::color::black);
static_assert(std::same_as<decltype(hg::colors::white_v<custom_color>), custom_color const>);
static_assert(hg::colors::white_v<custom_color> == custom_color::white);
static_assert(hg::colors::gray_v<custom_color> == custom_color::gray);
static_assert(hg::colors::black_v<custom_color> == custom_color::black);
static_assert(hg::colors::white<custom_color>::value == custom_color::white);
static_assert(hg::colors::gray<custom_color>::value == custom_color::gray);
static_assert(hg::colors::black<custom_color>::value == custom_color::black);
}

TEST_CASE("graph colors have distinct, documented default values", "[graph][algorithm][common][color]")
{
  REQUIRE(static_cast<unsigned>(hg::color::white) == 0u);
  REQUIRE(static_cast<unsigned>(hg::color::black) == 1u);
  REQUIRE(static_cast<unsigned>(hg::color::gray) == 2u);
}

TEST_CASE("graph colors accept a custom enum with different values", "[graph][algorithm][common][color]")
{
  REQUIRE(hg::colors::white_v<custom_color> == custom_color::white);
  REQUIRE(hg::colors::black_v<custom_color> == custom_color::black);
  REQUIRE(hg::colors::gray_v<custom_color> == custom_color::gray);
}
