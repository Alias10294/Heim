#ifndef HEIM_TEST_META_TYPE_SEQUENCE_SUPPORT_HPP
#define HEIM_TEST_META_TYPE_SEQUENCE_SUPPORT_HPP

#include <heim/meta/type_sequence.hpp>

#include <array>
#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace heim_meta_test
{
template<class ...Ts>
using seq = heim::type_sequence<Ts...>;

template<class Actual, class Expected>
inline constexpr bool same = std::same_as<Actual, Expected>;

struct a {};
struct b {};
struct c {};
struct d {};
struct e {};
struct separator {};
struct incomplete;

using empty = seq<>;
using single = seq<a>;
using triple = seq<a, b, c>;
using repeated = seq<a, b, a, c, b, a>;

template<int N>
using number = std::integral_constant<int, N>;

template<class T>
struct add_pointer : std::type_identity<T*> {};

template<class T>
struct is_pointer : std::is_pointer<T> {};

template<class T>
struct is_const : std::is_const<T> {};

template<class T>
struct always_false : std::false_type {};

template<class T>
struct always_true : std::true_type {};

template<class Left, class Right>
struct make_pair_sequence : std::type_identity<seq<Left, Right>> {};

template<class Left, class Right>
struct sequence_cat : heim::type_sequence_concat<Left, Right> {};

template<class Left, class Right>
struct less_number : std::bool_constant<(Left::value < Right::value)> {};

template<class Left, class Right>
struct greater_number : std::bool_constant<(Left::value > Right::value)> {};

template<class S, std::size_t I>
concept has_get = requires { typename heim::type_sequence_get<S, I>::type; };

template<class S>
concept has_front = requires { typename heim::type_sequence_front<S>::type; };

template<class S>
concept has_back = requires { typename heim::type_sequence_back<S>::type; };

template<class S, std::size_t N>
concept has_chunk = requires { typename heim::type_sequence_chunk<S, N>::type; };

template<class S, std::size_t N>
concept has_elements = requires { typename heim::type_sequence_elements<S, N>::type; };

template<class S, std::size_t N>
concept has_take = requires { typename heim::type_sequence_take<S, N>::type; };

template<class S, std::size_t N>
concept has_drop = requires { typename heim::type_sequence_drop<S, N>::type; };
}

#endif // HEIM_TEST_META_TYPE_SEQUENCE_SUPPORT_HPP
