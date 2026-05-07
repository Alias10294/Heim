#ifndef HEIM_TYPE_SEQUENCE_HPP
#define HEIM_TYPE_SEQUENCE_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace heim
{
/*!
 * \brief
 *   A type describing a sequence of specializing types.
 */
template<typename ...Ts>
struct type_sequence;


/*!
 * \brief
 *   Determines the number of types specializing the specializing type sequence.
 */
template<typename>
struct type_sequence_size;

template<typename T>
inline constexpr
std::size_t
type_sequence_size_v
= type_sequence_size<T>::value;

template<typename>
struct type_sequence_size
{ };

template<typename ...Ts>
struct type_sequence_size<
    type_sequence<Ts ...>>
  : std::integral_constant<std::size_t, sizeof...(Ts)>
{ };


/*!
 * \brief
 *   Determines whether the specializing type sequence is specialized by no type.
 */
template<typename>
struct type_sequence_empty;

template<typename T>
inline constexpr
bool
type_sequence_empty_v
= type_sequence_empty<T>::value;

template<typename>
struct type_sequence_empty
{ };

template<>
struct type_sequence_empty<
    type_sequence<>>
  : std::true_type
{ };

template<typename T, typename ...Ts>
struct type_sequence_empty<
    type_sequence<
        T,
        Ts ...>>
  : std::false_type
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by concatenating the specializing type sequences.
 */
template<typename ...>
struct type_sequence_concat;

template<typename ...Ts>
using type_sequence_concat_t
= typename type_sequence_concat<Ts ...>::type;

template<typename ...>
struct type_sequence_concat
{ };

template<typename ...Ts>
struct type_sequence_concat<
    type_sequence<Ts ...>>
  : std::type_identity<type_sequence<Ts ...>>
{ };

template<
    typename ...As,
    typename ...Bs,
    typename ...Rest>
struct type_sequence_concat<
    type_sequence<As ...>,
    type_sequence<Bs ...>,
    Rest ...>
  : type_sequence_concat<
        type_sequence<As ..., Bs ...>,
        Rest ...>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by appending the specializing type pack to the specializing
 *   type sequence.
 */
template<
    typename,
    typename ...>
struct type_sequence_append;

template<
    typename    T,
    typename ...Ts>
using type_sequence_append_t
= typename type_sequence_append<T, Ts ...>::type;

template<
    typename,
    typename ...>
struct type_sequence_append
{ };

template<
    typename ...Ts,
    typename ...Us>
struct type_sequence_append<
    type_sequence<Ts ...>,
    Us ...>
  : std::type_identity<type_sequence<Ts ..., Us ...>>
{ };


/*!
 * \brief
 *   Determines the type sequence by applying the specializing meta-function to each type of the specializing
 *   type sequence.
 *
 * \note
 *   The specializing meta-function must expose a \code ::type\endcode alias.
 */
template<
    typename,
    template<typename>
    typename>
struct type_sequence_transform;

template<
    typename,
    template<typename>
    typename>
struct type_sequence_transform
{ };

template<
    typename ...Ts,
    template<typename>
    typename    Transform>
struct type_sequence_transform<
    type_sequence<Ts ...>,
    Transform>
  : std::type_identity<type_sequence<typename Transform<Ts>::type ...>>
{ };

template<
    typename T,
    template<typename>
    typename Meta>
using type_sequence_transform_t
= typename type_sequence_transform<T, Meta>::type;


/*!
 * \brief
 *   Determines the type sequence by applying the specializing meta-function predicate to each type
 *   of the specializing type sequence.
 *
 * \note
 *   The specializing meta-function must expose a \code ::value\endcode attribute.
 */
template<
    typename,
    template<typename>
    typename>
struct type_sequence_filter;

template<
    typename T,
    template<typename>
    typename Pred>
using type_sequence_filter_t
= typename type_sequence_filter<T, Pred>::type;

template<
    typename,
    template<typename>
    typename>
struct type_sequence_filter
{ };

template<
    template<typename>
    typename Pred>
struct type_sequence_filter<
    type_sequence<>,
    Pred>
  : std::type_identity<type_sequence<>>
{ };

template<
    typename    First,
    typename ...Rest,
    template<typename>
    typename    Pred>
struct type_sequence_filter<
    type_sequence<First, Rest ...>,
    Pred>
  : type_sequence_concat<
        std::conditional_t<
            Pred<First>::value,
            type_sequence<First>,
            type_sequence<>>,
        type_sequence_filter_t<
            type_sequence<Rest ...>,
            Pred>>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by selecting the first specializing number of types from
 *   the specializing type sequence.
 */
template<
    typename,
    std::size_t>
struct type_sequence_take;

template<
    typename    T,
    std::size_t N>
using type_sequence_take_t
= typename type_sequence_take<T, N>::type;

template<
    typename,
    std::size_t>
struct type_sequence_take
{ };

template<typename ...Ts>
struct type_sequence_take<
    type_sequence<Ts ...>,
    0>
  : std::type_identity<type_sequence<>>
{ };

template<std::size_t N>
struct type_sequence_take<
    type_sequence<>,
    N>
  : std::type_identity<type_sequence<>>
{ };

template<
    typename    First,
    typename ...Rest,
    std::size_t N>
struct type_sequence_take<
    type_sequence<First, Rest ...>,
    N>
  : type_sequence_concat<
        type_sequence<First>,
        type_sequence_take_t<
            type_sequence<Rest ...>,
            N - 1>>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by excluding the first specializing number of types from
 *   the specializing type sequence.
 */
template<
    typename,
    std::size_t>
struct type_sequence_drop;

template<
    typename    T,
    std::size_t N>
using type_sequence_drop_t
= typename type_sequence_drop<T, N>::type;

template<
    typename,
    std::size_t>
struct type_sequence_drop
{ };

template<typename ...Ts>
struct type_sequence_drop<
    type_sequence<Ts ...>,
    0>
  : std::type_identity<type_sequence<Ts ...>>
{ };

template<std::size_t N>
requires (N > 0)
struct type_sequence_drop<
    type_sequence<>,
    N>
  : std::type_identity<type_sequence<>>
{ };

template<
    typename    First,
    typename ...Rest,
    std::size_t N>
requires (N > 0)
struct type_sequence_drop<
    type_sequence<First, Rest ...>,
    N>
  : type_sequence_drop<
        type_sequence<Rest ...>,
        N - 1>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by flattening the specializing type sequence's specializing
 *   type sequences.
 */
template<typename>
struct type_sequence_join;

template<typename T>
using type_sequence_join_t
= typename type_sequence_join<T>::type;

template<typename>
struct type_sequence_join
{ };

template<>
struct type_sequence_join<
    type_sequence<>>
  : std::type_identity<type_sequence<>>
{ };

template<
    typename    First,
    typename ...Rest>
struct type_sequence_join<
    type_sequence<First, Rest ...>>
  : type_sequence_concat<
        type_sequence<First>,
        type_sequence_join_t<type_sequence<Rest ...>>>
{ };

template<
    typename ...First,
    typename ...Rest>
struct type_sequence_join<
    type_sequence<type_sequence<First ...>, Rest ...>>
  : type_sequence_concat<
        type_sequence<First ...>,
        type_sequence_join_t<
            type_sequence<Rest ...>>>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by removing all repeating types from the specializing type
 *   sequence.
 */
template<typename>
struct type_sequence_unique;

template<typename T>
using type_sequence_unique_t
= typename type_sequence_unique<T>::type;

namespace detail
{
template<
    typename,
    typename = type_sequence<>>
struct type_sequence_unique_impl;

template<
    typename,
    typename>
struct type_sequence_unique_impl
{ };

template<
    typename ...Visited>
struct type_sequence_unique_impl<
    type_sequence<>,
    type_sequence<Visited...>>
  : std::type_identity<type_sequence<Visited...>>
{ };

template<
    typename    First,
    typename ...Rest,
    typename ...Visited>
struct type_sequence_unique_impl<
    type_sequence<First, Rest ...>,
    type_sequence<Visited ...>>
  : type_sequence_unique_impl<
        type_sequence<Rest ...>,
        std::conditional_t<
            (std::is_same_v<First, Visited> || ...),
            type_sequence<Visited ...>,
            type_sequence<Visited ..., First>>>
{ };


}

template<typename T>
struct type_sequence_unique
  : detail::type_sequence_unique_impl<T>
{ };


/*!
 * \brief
 *   Determines whether the specializing type sequence is specialized by a set of types.
 */
template<typename>
struct type_sequence_is_unique;

template<typename T>
inline constexpr
bool
type_sequence_is_unique_v
= type_sequence_is_unique<T>::value;

template<typename T>
struct type_sequence_is_unique
  : std::is_same<T, type_sequence_unique_t<T>>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by reversing the specializing type sequence.
 */
template<typename>
struct type_sequence_reverse;

template<typename T>
using type_sequence_reverse_t
= typename type_sequence_reverse<T>::type;

template<typename>
struct type_sequence_reverse
{ };

template<>
struct type_sequence_reverse<
    type_sequence<>>
  : std::type_identity<type_sequence<>>
{ };

template<
    typename    First,
    typename ...Rest>
struct type_sequence_reverse<
    type_sequence<First, Rest ...>>
  : type_sequence_concat<
        type_sequence_reverse_t<type_sequence<Rest ...>>,
        type_sequence<First>>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by splitting the specializing type sequence by a delimiter
 *   type.
 */
template<
    typename,
    typename>
struct type_sequence_split;

template<
    typename T,
    typename Pattern>
using type_sequence_split_t
= typename type_sequence_split<T, Pattern>::type;

namespace detail
{
template<
    typename,
    typename,
    typename = type_sequence<>>
struct type_sequence_split_impl;

template<
    typename,
    typename,
    typename>
struct type_sequence_split_impl
{ };

template<
    typename    Pattern,
    typename ...Current>
struct type_sequence_split_impl<
    type_sequence<>,
    Pattern,
    type_sequence<Current ...>>
  : std::type_identity<type_sequence<type_sequence<Current ...>>>
{ };

template<
    typename ...Rest,
    typename    Pattern,
    typename ...Current>
struct type_sequence_split_impl<
    type_sequence<Pattern, Rest ...>,
    Pattern,
    type_sequence<Current ...>>
  : type_sequence_concat<
        type_sequence<type_sequence<Current ...>>,
        type_sequence_split_t<
            type_sequence<Rest ...>,
            Pattern>>
{ };

template<
    typename    First,
    typename ...Rest,
    typename    Pattern,
    typename ...Current>
struct type_sequence_split_impl<
    type_sequence<First, Rest ...>,
    Pattern,
    type_sequence<Current ...>>
  : type_sequence_split_impl<
        type_sequence<Rest ...>,
        Pattern,
        type_sequence<Current ..., First>>
{ };


}

template<
    typename T,
    typename Pattern>
struct type_sequence_split
  : detail::type_sequence_split_impl<T, Pattern>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by taking the type at the specializing integral index in
 *   each of the specializing type sequence's specializing type sequences.
 */
template<
    typename,
    std::size_t>
struct type_sequence_elements;

template<
    typename    T,
    std::size_t N>
using type_sequence_elements_t
= typename type_sequence_elements<T, N>::type;

template<
    typename,
    std::size_t>
struct type_sequence_elements
{ };

template<std::size_t N>
struct type_sequence_elements<
    type_sequence<>,
    N>
  : std::type_identity<type_sequence<>>
{ };

template<
    typename ...First,
    typename ...Rest,
    std::size_t N>
struct type_sequence_elements<
    type_sequence<type_sequence<First ...>, Rest ...>,
    N>
  : type_sequence_concat<
        type_sequence_take_t<type_sequence_drop_t<type_sequence<First ...>, N>, 1>,
        type_sequence_elements_t<type_sequence<Rest ...>, N>>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by joining the specializing type sequences together.
 */
template<typename ...>
struct type_sequence_zip;

template<typename ...Ts>
using type_sequence_zip_t
= typename type_sequence_zip<Ts ...>::type;

namespace detail
{
template<
    typename,
    std::size_t>
struct type_sequence_zip_impl;

template<
    typename,
    std::size_t>
struct type_sequence_zip_impl
{ };

template<typename ...Ts>
struct type_sequence_zip_impl<
    type_sequence<Ts ...>,
    0>
  : std::type_identity<type_sequence<>>
{ };

template<
    typename ...Ts,
    std::size_t N>
struct type_sequence_zip_impl<
    type_sequence<Ts ...>,
    N>
  : type_sequence_append<
        typename type_sequence_zip_impl<
            type_sequence<Ts ...>,
            N - 1>
            ::type,
        type_sequence_elements_t<
            type_sequence<Ts ...>,
            N - 1>>
{ };


}

template<typename ...Ts>
struct type_sequence_zip
  : detail::type_sequence_zip_impl<
        type_sequence<Ts ...>,
        std::min({type_sequence_size_v<Ts>...})>
{ };

template<>
struct type_sequence_zip<>
  : std::type_identity<type_sequence<>>
{ };


/*!
 * \brief
 *   Determines the type obtained by segmenting the specializing type sequence into chunks of the specializing
 *   number size.
 */
template<
    typename,
    std::size_t>
struct type_sequence_chunk;

template<
    typename    T,
    std::size_t N>
using type_sequence_chunk_t
= typename type_sequence_chunk<T, N>::type;

template<
    typename,
    std::size_t>
struct type_sequence_chunk
{ };

template<typename ...Ts>
struct type_sequence_chunk<
    type_sequence<Ts ...>,
    0>
{ };

template<std::size_t N>
struct type_sequence_chunk<
    type_sequence<>,
    N>
  : std::type_identity<type_sequence<>>
{ };

template<
    typename ...Ts,
    std::size_t N>
struct type_sequence_chunk<
    type_sequence<Ts ...>,
    N>
  : type_sequence_concat<
        type_sequence        <type_sequence_take_t<type_sequence<Ts ...>, N>>,
        type_sequence_chunk_t<type_sequence_drop_t<type_sequence<Ts ...>, N>, N>>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by selecting from the specializing type sequence the types
 *   by advancing the specializing number of times at a time.
 */
template<
    typename,
    std::size_t>
struct type_sequence_stride;

template<
    typename    T,
    std::size_t N>
using type_sequence_stride_t
= typename type_sequence_stride<T, N>::type;

template<
    typename    T,
    std::size_t N>
struct type_sequence_stride
  : type_sequence_elements<type_sequence_chunk_t<T, N>, 0>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by doing the cartesian product of the specializing type sequences.
 */
template<typename ...>
struct type_sequence_cartesian_product;

template<typename ...Ts>
using type_sequence_cartesian_product_t
= typename type_sequence_cartesian_product<Ts ...>::type;

namespace detail
{
template<
    typename,
    typename>
struct type_sequence_cartesian_product_impl;

template<typename ...Products>
struct type_sequence_cartesian_product_impl<
    type_sequence<>,
    type_sequence<Products ...>>
  : std::type_identity<type_sequence<>>
{ };

template<
    typename    First,
    typename ...Rest,
    typename ...Products>
struct type_sequence_cartesian_product_impl<
    type_sequence<First, Rest ...>,
    type_sequence<Products ...>>
  : type_sequence_concat<
        type_sequence<
            type_sequence_concat_t<
                type_sequence<First>,
                Products>
            ...>,
        typename type_sequence_cartesian_product_impl<
            type_sequence<Rest     ...>,
            type_sequence<Products ...>>
            ::type>
{ };


}

template<>
struct type_sequence_cartesian_product<>
  : std::type_identity<type_sequence<type_sequence<>>>
{ };

template<typename ...Ts>
struct type_sequence_cartesian_product<
    type_sequence<Ts ...>>
  : std::type_identity<type_sequence<type_sequence<Ts> ...>>
{ };

template<
    typename    First,
    typename    Second,
    typename ...Rest>
struct type_sequence_cartesian_product<
    First,
    Second,
    Rest ...>
  : detail::type_sequence_cartesian_product_impl<
        First,
        type_sequence_cartesian_product_t<
            Second,
            Rest ...>>
{ };


/*!
 * \brief
 *   Determines the index of the specializing type in the specializing type sequence.
 */
template<
    typename,
    typename>
struct type_sequence_index;

template<
    typename T,
    typename U>
inline constexpr
std::size_t
type_sequence_index_v
= type_sequence_index<T, U>::value;

template<
    typename,
    typename>
struct type_sequence_index
{ };

template<typename T>
struct type_sequence_index<
    type_sequence<>,
    T>
  : std::integral_constant<std::size_t, 0>
{ };

template<
    typename    First,
    typename ...Rest,
    typename    T>
struct type_sequence_index<
    type_sequence<First, Rest ...>,
    T>
  : std::integral_constant<
        std::size_t,
        std::is_same_v<First, T>
          ? 0
          : 1 + type_sequence_index_v<type_sequence<Rest ...>, T>>
{ };


/*!
 * \brief
 *   Determines the number of occurrences of the specializing type in the specializing type sequence.
 */
template<
    typename,
    typename>
struct type_sequence_count;

template<
    typename T,
    typename U>
inline constexpr
std::size_t
type_sequence_count_v
= type_sequence_count<T, U>::value;

template<
    typename,
    typename>
struct type_sequence_count
{ };

template<
    typename ...Ts,
    typename    T>
struct type_sequence_count<
    type_sequence<Ts ...>,
    T>
  : std::integral_constant<
        std::size_t,
        (0 + ... + (std::is_same_v<Ts, T> ? 1 : 0))>
{ };


/*!
 * \brief
 *   Determines whether the specializing type is in the specializing type sequence.
 */
template<
    typename,
    typename>
struct type_sequence_contains;

template<
    typename T,
    typename U>
inline constexpr
bool
type_sequence_contains_v
= type_sequence_contains<T, U>::value;

template<
    typename T,
    typename U>
struct type_sequence_contains
  : std::bool_constant<type_sequence_count_v<T, U> != 0>
{ };


/*!
 * \brief
 *   Determines the first type in the specializing type sequence.
 */
template<typename>
struct type_sequence_front;

template<typename T>
using type_sequence_front_t
= typename type_sequence_front<T>::type;

template<>
struct type_sequence_front<
    type_sequence<>>
{ };

template<
    typename    T,
    typename ...Ts>
struct type_sequence_front<
    type_sequence<T, Ts...>>
  : std::type_identity<T>
{ };


/*!
 * \brief
 *   Determines the last type in the specializing type sequence.
 */
template<typename>
struct type_sequence_back;

template<typename T>
using type_sequence_back_t
= typename type_sequence_back<T>::type;

template<typename T>
struct type_sequence_back
  : type_sequence_front<type_sequence_reverse_t<T>>
{ };


/*!
 * \brief
 *   Determines the type specializing the specializing type sequence at the specializing index.
 */
template<
    typename,
    std::size_t>
struct type_sequence_get;

template<
    typename    T,
    std::size_t N>
using type_sequence_get_t
= typename type_sequence_get<T, N>::type;

template<
    typename    T,
    std::size_t N>
struct type_sequence_get
  : type_sequence_front<type_sequence_drop_t<T, N>>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by inserting at the specializing index the specializing type
 *   in the specializing type sequence.
 */
template<
    typename,
    std::size_t,
    typename ...>
struct type_sequence_insert;

template<
    typename    T,
    std::size_t N,
    typename ...Ts>
using type_sequence_insert_t
= typename type_sequence_insert<T, N, Ts...>::type;

template<
    typename    T,
    std::size_t N,
    typename ...Ts>
struct type_sequence_insert
  : type_sequence_concat<
        type_sequence_append_t<
            type_sequence_take_t<T, N>,
            Ts ...>,
        type_sequence_drop_t<T, N>>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by selecting the types at the specializing indices in the
 *   specializing type sequence.
 */
template<
    typename,
    std::size_t ...>
struct type_sequence_select;

template<
    typename       T,
    std::size_t ...Ns>
using type_sequence_select_t
= typename type_sequence_select<T, Ns...>::type;

template<
    typename       T,
    std::size_t ...Ns>
struct type_sequence_select
  : std::type_identity<type_sequence<type_sequence_get_t<T, Ns> ...>>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by removing all types that satisfy the specializing meta-function
 *   predicate from the specializing type sequence.
 */
template<
    typename,
    template<typename>
    typename>
struct type_sequence_remove_if;

template<
    typename T,
    template<typename>
    typename Pred>
using type_sequence_remove_if_t
= typename type_sequence_remove_if<T, Pred>::type;

template<
    typename,
    template<typename>
    typename>
struct type_sequence_remove_if
{ };

template<
    template<typename>
    typename Pred>
struct type_sequence_remove_if<
    type_sequence<>,
    Pred>
  : std::type_identity<type_sequence<>>
{ };

template<
    typename    First,
    typename ...Rest,
    template<typename>
    typename    Pred>
struct type_sequence_remove_if<
    type_sequence<First, Rest ...>,
    Pred>
  : type_sequence_concat<
        std::conditional_t<
            Pred<First>::value,
            type_sequence<>,
            type_sequence<First>>,
        type_sequence_remove_if_t<
            type_sequence<Rest ...>,
            Pred>>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by removing all occurrences of the specializing type from
 *   the specializing type sequence.
 */
template<
    typename,
    typename>
struct type_sequence_remove;

template<
    typename T,
    typename Pred>
using type_sequence_remove_t
= typename type_sequence_remove<T, Pred>::type;

namespace detail
{
template<typename>
struct type_sequence_remove_meta;

template<typename T>
struct type_sequence_remove_meta
{
  template<typename U>
  struct predicate
    : std::is_same<T, U>
  { };
};


}

template<
    typename T,
    typename U>
struct type_sequence_remove
  : type_sequence_remove_if<
        T,
        detail::type_sequence_remove_meta<U>::template predicate>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by removing the types at the specializing indices in the
 *   specializing type sequence.
 */
template<
    typename,
    std::size_t ...>
struct type_sequence_erase;

template<
    typename       T,
    std::size_t ...Ns>
using type_sequence_erase_t
= typename type_sequence_erase<T, Ns...>::type;

namespace detail
{
template<
    std::size_t,
    typename,
    std::size_t ...>
struct type_sequence_erase_impl;

template<
    std::size_t    Current,
    typename       T,
    std::size_t ...Ns>
struct type_sequence_erase_impl
{ };

template<
    std::size_t    Current,
    std::size_t ...Ns>
struct type_sequence_erase_impl<
    Current,
    type_sequence<>,
    Ns ...>
  : std::type_identity<type_sequence<>>
{ };

template<
    std::size_t    Current,
    typename       First,
    typename    ...Rest,
    std::size_t ...Ns>
struct type_sequence_erase_impl<
    Current,
    type_sequence<First, Rest ...>,
    Ns ...>
  : type_sequence_concat<
        std::conditional_t<
            ((Current == Ns) || ...),
            type_sequence<>,
            type_sequence<First>>,
        typename type_sequence_erase_impl<
            Current + 1,
            type_sequence<Rest ...>,
            Ns ...>
            ::type>
{ };


}

template<
    typename       T,
    std::size_t ...Ns>
struct type_sequence_erase
  : detail::type_sequence_erase_impl<0, T, Ns ...>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by prepending the specializing type pack to the specializing
 *   type sequence.
 */
template<
    typename,
    typename ...>
struct type_sequence_prepend;

template<
    typename    T,
    typename ...Ts>
using type_sequence_prepend_t
= typename type_sequence_prepend<T, Ts ...>::type;

template<
    typename,
    typename ...>
struct type_sequence_prepend
{ };

template<
    typename ...Ts,
    typename ...Us>
struct type_sequence_prepend<
    type_sequence<Ts ...>,
    Us ...>
  : std::type_identity<type_sequence<Us ..., Ts ...>>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by removing the first type in the specializing type sequence.
 */
template<typename>
struct type_sequence_pop_front;

template<typename T>
using type_sequence_pop_front_t
= typename type_sequence_pop_front<T>::type;

template<typename>
struct type_sequence_pop_front
{ };

template<
    typename    T,
    typename ...Ts>
struct type_sequence_pop_front<
    type_sequence<T, Ts ...>>
  : std::type_identity<type_sequence<Ts ...>>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by removing the last type in the specializing type sequence.
 */
template<typename>
struct type_sequence_pop_back;

template<typename T>
using type_sequence_pop_back_t
= typename type_sequence_pop_back<T>::type;

template<typename T>
struct type_sequence_pop_back
  : type_sequence_reverse<type_sequence_pop_front_t<type_sequence_reverse_t<T>>>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by replacing all types in the specializing type sequence
 *   by the specializing type.
 */
template<
    typename,
    typename>
struct type_sequence_fill;

template<
    typename T,
    typename U>
using type_sequence_fill_t
= typename type_sequence_fill<T, U>::type;

namespace detail
{
template<typename>
struct type_sequence_fill_meta;

template<typename T>
struct type_sequence_fill_meta
{
  template<typename>
  struct transform
    : std::type_identity<T>
  { };
};


}

template<
    typename,
    typename>
struct type_sequence_fill
{ };

template<
    typename ...Ts,
    typename    T>
struct type_sequence_fill<
    type_sequence<Ts ...>,
    T>
  : type_sequence_transform<
        type_sequence<Ts ...>,
        detail::type_sequence_fill_meta<T>::template transform>
{ };


/*!
 * \brief
 *   Determines the tuple corresponding to the specializing type sequence.
 */
template<typename>
struct type_sequence_tuple;

template<typename T>
using type_sequence_tuple_t
= typename type_sequence_tuple<T>::type;

template<typename>
struct type_sequence_tuple
{ };

template<typename ...Ts>
struct type_sequence_tuple<
    type_sequence<Ts ...>>
  : std::type_identity<std::tuple<Ts ...>>
{ };


/*!
 * \brief
 *   Determines the type sequence obtained by replacing the type at the specializing index number in
 *   the specializing type sequence by the specializing types.
 */
template<
    typename,
    std::size_t,
    typename ...>
struct type_sequence_replace;

template<
    typename    T,
    std::size_t N,
    typename ...Ts>
using type_sequence_replace_t
= typename type_sequence_replace<T, N, Ts ...>::type;

template<
    typename    T,
    std::size_t N,
    typename ...Us>
struct type_sequence_replace
  : type_sequence_concat<
        type_sequence_take_t<T, N>,
        type_sequence       <Us ...>,
        type_sequence_drop_t<T, N + 1>>
{ };


/*!
 * \brief
 *   Determines the type obtained by slicing the specializing type sequence at the specializing indices.
 */
template<
    typename,
    std::size_t,
    std::size_t>
struct type_sequence_slice;

template<
    typename    T,
    std::size_t M,
    std::size_t N>
using type_sequence_slice_t
= typename type_sequence_slice<T, M, N>::type;

template<
    typename    T,
    std::size_t M,
    std::size_t N>
struct type_sequence_slice
  : type_sequence_drop<type_sequence_take_t<T, N>, M>
{ };


/*!
 * \brief
 *   Determines the type sequence by swapping the types at the specializing indices in the specializing
 *   type sequence.
 */
template<
    typename,
    std::size_t,
    std::size_t>
struct type_sequence_swap;

template<
    typename    T,
    std::size_t M,
    std::size_t N>
using type_sequence_swap_t
= typename type_sequence_swap<T, M, N>::type;

template<
    typename    T,
    std::size_t M,
    std::size_t N>
struct type_sequence_swap
  : type_sequence_replace<
        type_sequence_replace_t<
            T,
            M,
            type_sequence_get_t<T, N>>,
        N,
        type_sequence_get_t<T, M>>
{ };


/*!
 * \brief
 *   Determines the type sequence by sorting the specializing type sequence using the specializing
 *   comparator meta-function.
 */
template<
    typename,
    template<typename, typename>
    typename>
struct type_sequence_sort;

template<
    typename T,
    template<typename, typename>
    typename Comp>
using type_sequence_sort_t
= typename type_sequence_sort<T, Comp>::type;

namespace detail
{
template<
    typename,
    typename,
    template<typename, typename>
    typename>
struct type_sequence_sort_merge;

template<
    typename,
    typename,
    template<typename, typename>
    typename>
struct type_sequence_sort_merge
{ };

template<
    template<typename, typename>
    typename Comp>
struct type_sequence_sort_merge<
    type_sequence<>,
    type_sequence<>,
    Comp>
  : std::type_identity<type_sequence<>>
{ };

template<
    typename ...Rs,
    template<typename, typename>
    typename    Comp>
struct type_sequence_sort_merge<
    type_sequence<>,
    type_sequence<Rs ...>,
    Comp>
  : std::type_identity<type_sequence<Rs ...>>
{ };

template<
    typename ...Ls,
    template<typename, typename>
    typename    Comp>
struct type_sequence_sort_merge<
    type_sequence<Ls ...>,
    type_sequence<>,
    Comp>
  : std::type_identity<type_sequence<Ls ...>>
{ };

template<
    typename    LFirst,
    typename ...LRest,
    typename    RFirst,
    typename ...RRest,
    template<typename, typename>
    typename    Comp>
struct type_sequence_sort_merge<
    type_sequence<LFirst, LRest ...>,
    type_sequence<RFirst, RRest ...>,
    Comp>
  : std::conditional_t<
        Comp<RFirst, LFirst>::value,
        type_sequence_prepend<
            typename type_sequence_sort_merge<
                type_sequence<LFirst, LRest ...>,
                type_sequence<RRest ...>,
                Comp>
                ::type,
            RFirst>,
        type_sequence_prepend<
            typename type_sequence_sort_merge<
                type_sequence<LRest ...>,
                type_sequence<RFirst, RRest ...>,
                Comp>
                ::type,
            LFirst>>
{ };


}

template<
    typename,
    template<typename, typename>
    typename>
struct type_sequence_sort
{ };

template<
    template<typename, typename>
    typename Comp>
struct type_sequence_sort<
    type_sequence<>,
    Comp>
  : std::type_identity<type_sequence<>>
{ };

template<
    typename T,
    template<typename, typename>
    typename Comp>
struct type_sequence_sort<
    type_sequence<T>,
    Comp>
  : std::type_identity<type_sequence<T>>
{ };

template<
    typename ...Ts,
    template<typename, typename>
    typename Comp>
struct type_sequence_sort<
    type_sequence<Ts ...>,
    Comp>
  : detail::type_sequence_sort_merge<
        type_sequence_sort_t<
            type_sequence_take_t<
                type_sequence<Ts ...>,
                type_sequence_size_v<type_sequence<Ts ...>> / 2>,
            Comp>,
        type_sequence_sort_t<
            type_sequence_drop_t<
                type_sequence<Ts ...>,
                type_sequence_size_v<type_sequence<Ts ...>> / 2>,
            Comp>,
        Comp>
{ };


/*!
 * \brief
 *   A type describing a non-empty pack of types.
 */
template<typename T, typename ...Ts>
struct type_sequence<T, Ts ...>
{
  static constexpr std::size_t size      = type_sequence_size_v     <type_sequence>;
  static constexpr bool        empty     = type_sequence_empty_v    <type_sequence>;
  static constexpr bool        is_unique = type_sequence_is_unique_v<type_sequence>;

  template<typename U> static constexpr std::size_t index    = type_sequence_index_v   <type_sequence, U>;
  template<typename U> static constexpr std::size_t count    = type_sequence_count_v   <type_sequence, U>;
  template<typename U> static constexpr bool        contains = type_sequence_contains_v<type_sequence, U>;

  using join      = type_sequence_join_t     <type_sequence>;
  using unique    = type_sequence_unique_t   <type_sequence>;
  using reverse   = type_sequence_reverse_t  <type_sequence>;
  using front     = type_sequence_front_t    <type_sequence>;
  using back      = type_sequence_back_t     <type_sequence>;
  using pop_front = type_sequence_pop_front_t<type_sequence>;
  using pop_back  = type_sequence_pop_back_t <type_sequence>;
  using tuple     = type_sequence_tuple_t    <type_sequence>;

  template<typename U> using split  = type_sequence_split_t <type_sequence, U>;
  template<typename U> using remove = type_sequence_remove_t<type_sequence, U>;
  template<typename U> using fill   = type_sequence_fill_t  <type_sequence, U>;

  template<std::size_t N> using take     = type_sequence_take_t    <type_sequence, N>;
  template<std::size_t N> using drop     = type_sequence_drop_t    <type_sequence, N>;
  template<std::size_t N> using elements = type_sequence_elements_t<type_sequence, N>;
  template<std::size_t N> using chunk    = type_sequence_chunk_t   <type_sequence, N>;
  template<std::size_t N> using stride   = type_sequence_stride_t  <type_sequence, N>;
  template<std::size_t N> using get      = type_sequence_get_t     <type_sequence, N>;

  template<typename ...Us> using concat            = type_sequence_concat_t           <type_sequence, Us ...>;
  template<typename ...Us> using append            = type_sequence_append_t           <type_sequence, Us ...>;
  template<typename ...Us> using prepend           = type_sequence_prepend_t          <type_sequence, Us ...>;
  template<typename ...Us> using zip               = type_sequence_zip_t              <type_sequence, Us ...>;
  template<typename ...Us> using cartesian_product = type_sequence_cartesian_product_t<type_sequence, Us ...>;

  template<std::size_t ...Ns> using select = type_sequence_select_t<type_sequence, Ns ...>;
  template<std::size_t ...Ns> using erase  = type_sequence_erase_t <type_sequence, Ns ...>;

  template<template<typename> typename Meta> using transform = type_sequence_transform_t<type_sequence, Meta>;
  template<template<typename> typename Pred> using filter    = type_sequence_filter_t   <type_sequence, Pred>;
  template<template<typename> typename Pred> using remove_if = type_sequence_remove_if_t<type_sequence, Pred>;

  template<
      template<typename, typename>
      typename Comp>
  using sort
  = type_sequence_sort_t<type_sequence, Comp>;

  template<std::size_t M, std::size_t N> using slice = type_sequence_slice_t<type_sequence, M, N>;
  template<std::size_t M, std::size_t N> using swap  = type_sequence_swap_t <type_sequence, M, N>;

  template<std::size_t N, typename ...Us> using insert  = type_sequence_insert_t <type_sequence, N, Us ...>;
  template<std::size_t N, typename ...Us> using replace = type_sequence_replace_t<type_sequence, N, Us ...>;
};


/*!
 * \brief
 *   A type describing an empty pack of types.
 */
template<>
struct type_sequence<>
{
  static constexpr std::size_t size      = type_sequence_size_v     <type_sequence>;
  static constexpr bool        empty     = type_sequence_empty_v    <type_sequence>;
  static constexpr bool        is_unique = type_sequence_is_unique_v<type_sequence>;

  template<typename U> static constexpr std::size_t index    = type_sequence_index_v   <type_sequence, U>;
  template<typename U> static constexpr std::size_t count    = type_sequence_count_v   <type_sequence, U>;
  template<typename U> static constexpr bool        contains = type_sequence_contains_v<type_sequence, U>;

  using join      = type_sequence_join_t     <type_sequence>;
  using unique    = type_sequence_unique_t   <type_sequence>;
  using reverse   = type_sequence_reverse_t  <type_sequence>;
  using tuple     = type_sequence_tuple_t    <type_sequence>;

  template<typename U> using split  = type_sequence_split_t <type_sequence, U>;
  template<typename U> using remove = type_sequence_remove_t<type_sequence, U>;
  template<typename U> using fill   = type_sequence_fill_t  <type_sequence, U>;

  template<std::size_t N> using take     = type_sequence_take_t    <type_sequence, N>;
  template<std::size_t N> using drop     = type_sequence_drop_t    <type_sequence, N>;
  template<std::size_t N> using elements = type_sequence_elements_t<type_sequence, N>;
  template<std::size_t N> using chunk    = type_sequence_chunk_t   <type_sequence, N>;
  template<std::size_t N> using stride   = type_sequence_stride_t  <type_sequence, N>;
  template<std::size_t N> using get      = type_sequence_get_t     <type_sequence, N>;

  template<typename ...Us> using concat            = type_sequence_concat_t           <type_sequence, Us ...>;
  template<typename ...Us> using append            = type_sequence_append_t           <type_sequence, Us ...>;
  template<typename ...Us> using prepend           = type_sequence_prepend_t          <type_sequence, Us ...>;
  template<typename ...Us> using zip               = type_sequence_zip_t              <type_sequence, Us ...>;
  template<typename ...Us> using cartesian_product = type_sequence_cartesian_product_t<type_sequence, Us ...>;

  template<std::size_t ...Ns> using select = type_sequence_select_t<type_sequence, Ns ...>;
  template<std::size_t ...Ns> using erase  = type_sequence_erase_t <type_sequence, Ns ...>;

  template<template<typename> typename Meta> using transform = type_sequence_transform_t<type_sequence, Meta>;
  template<template<typename> typename Pred> using filter    = type_sequence_filter_t   <type_sequence, Pred>;
  template<template<typename> typename Pred> using remove_if = type_sequence_remove_if_t<type_sequence, Pred>;

  template<
      template<typename, typename>
      typename Comp>
  using sort
  = type_sequence_sort_t<type_sequence, Comp>;

  template<std::size_t M, std::size_t N> using slice = type_sequence_slice_t<type_sequence, M, N>;
  template<std::size_t M, std::size_t N> using swap  = type_sequence_swap_t <type_sequence, M, N>;

  template<std::size_t N, typename ...Us> using insert  = type_sequence_insert_t <type_sequence, N, Us ...>;
  template<std::size_t N, typename ...Us> using replace = type_sequence_replace_t<type_sequence, N, Us ...>;
};


/*!
 * \brief
 *   Determines whether the specializing type is a specialization of type_sequence.
 */
template<typename>
struct is_specialization_of_type_sequence;

template<typename>
struct is_specialization_of_type_sequence
  : std::false_type
{ };

template<typename ...Ts>
struct is_specialization_of_type_sequence<
    type_sequence<Ts ...>>
  : std::true_type
{ };

template<typename T>
inline constexpr
bool
is_specialization_of_type_sequence_v
= is_specialization_of_type_sequence<T>::value;

template<typename T>
concept specialization_of_type_sequence
= is_specialization_of_type_sequence_v<T>;


/*!
 * \brief
 *   Determines the type sequence obtained by repeating the specializing type the specializing number
 *   of times.
 */
template<
    typename,
    std::size_t>
struct type_sequence_array;

template<
    typename    T,
    std::size_t N>
using type_sequence_array_t
= typename type_sequence_array<T, N>::type;

namespace detail
{
template<
    typename,
    std::size_t,
    std::size_t = 0>
struct type_sequence_array_impl;

template<
    typename    T,
    std::size_t N>
struct type_sequence_array_impl<
    T,
    N,
    N>
  : std::type_identity<type_sequence<>>
{ };

template<
    typename    T,
    std::size_t N,
    std::size_t Current>
struct type_sequence_array_impl
  : type_sequence_concat<
        type_sequence<T>,
        typename type_sequence_array_impl<
            T,
            N,
            Current + 1>
            ::type>
{ };

} // namespace detail


template<
    typename    T,
    std::size_t N>
struct type_sequence_array
  : detail::type_sequence_array_impl<T, N>
{ };


/*!
 * \brief
 *   Determines the type sequence corresponding to the specializing type.
 */
template<typename>
struct to_type_sequence;

template<typename T>
using to_type_sequence_t
= typename to_type_sequence<T>::type;

template<typename>
struct to_type_sequence
{ };

template<typename ...Ts>
struct to_type_sequence<
    std::tuple<Ts ...>>
  : std::type_identity<type_sequence<Ts ...>>
{ };

template<
    typename T1,
    typename T2>
struct to_type_sequence<
    std::pair<T1, T2>>
  : std::type_identity<type_sequence<T1, T2>>
{ };

template<
    typename    T,
    std::size_t N>
struct to_type_sequence<
    std::array<T, N>>
  : type_sequence_array<T, N>
{ };

} // namespace heim

#endif // HEIM_TYPE_SEQUENCE_HPP
