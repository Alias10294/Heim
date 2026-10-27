#include "support.hpp"
#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>

using M = heim_test::sparse_map::map<>;
static_assert(std::input_iterator<M::iterator>);
static_assert(std::input_iterator<M::const_iterator>);
static_assert(std::sentinel_for<M::iterator, M::iterator>);
static_assert(std::sentinel_for<M::const_iterator, M::const_iterator>);
static_assert(std::ranges::input_range<M>);
static_assert(std::ranges::input_range<M const>);
static_assert(std::random_access_iterator<M::iterator>);
static_assert(std::random_access_iterator<M::const_iterator>);
static_assert(std::ranges::random_access_range<M>);
static_assert(std::ranges::random_access_range<M const>);
