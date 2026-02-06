#ifndef HEIM_TYPE_SEQUENCE_HPP
#define HEIM_TYPE_SEQUENCE_HPP

#include <tuple>
#include "utility.hpp"
#include "test/doctest.h"

namespace heim
{
/*!
 * @brief A type representing a pack of types.
 *
 * @details Implements what is typically referred to as a type list.
 *   Includes all existing expressed type sequence traits.
 *
 * @tparam Ts The pack of types.
 */
template<typename ...Ts>
struct type_sequence;

/*!
 * @brief Determines the number of types in a given type sequence.
 *
 * @tparam TypeSequence The type sequence.
 */
template<typename TypeSequence>
struct type_sequence_size;

template<typename TypeSequence>
static constexpr
std::size_t
type_sequence_size_v
= type_sequence_size<TypeSequence>::value;

template<typename>
struct type_sequence_size
{ };

template<typename ...Ts>
struct type_sequence_size<type_sequence<Ts ...>>
  : size_constant<sizeof...(Ts)>
{ };



/*!
 * @brief Determines the number of occurrences of a given type in a type sequence.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam T            The type to determine the count of.
 */
template<
    typename TypeSequence,
    typename T>
struct type_sequence_count;

template<
    typename TypeSequence,
    typename T>
inline constexpr
std::size_t
type_sequence_count_v
= type_sequence_count<TypeSequence, T>::value;

template<
    typename TypeSequence,
    typename T>
struct type_sequence_count
{ };

template<
    typename ...Ts,
    typename    T>
struct type_sequence_count<
    type_sequence<Ts ...>,
    T>
  : size_constant<(0 + ... + (std::is_same_v<Ts, T> ? 1 : 0))>
{ };



/*!
 * @brief Determines whether the given type is present in the type sequence.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam T            The type to determine the presence of.
 */
template<
    typename TypeSequence,
    typename T>
struct type_sequence_contains;

template<
    typename TypeSequence,
    typename T>
inline constexpr
bool
type_sequence_contains_v
= type_sequence_contains<TypeSequence, T>::value;

template<
    typename TypeSequence,
    typename T>
struct type_sequence_contains
{ };

template<
    typename ...Ts,
    typename    T>
struct type_sequence_contains<
    type_sequence<Ts ...>,
    T>
  : bool_constant<(type_sequence_count_v<type_sequence<Ts ...>, T> > 0)>
{ };



/*!
 * @brief Determines the index of the given type in the type sequence.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam T            The type to get the index of.
 *
 * @warning This trait is ill-formed when the type is not present in the type sequence.
 */
template<
    typename TypeSequence,
    typename T>
struct type_sequence_index;

template<
    typename TypeSequence,
    typename T>
inline constexpr
std::size_t
type_sequence_index_v
= type_sequence_index<TypeSequence, T>::value;

template<
    typename TypeSequence,
    typename T>
struct type_sequence_index
{ };

template<typename T>
struct type_sequence_index<
    type_sequence<>,
    T>
  : size_constant<0>
{ };

template<
    typename    First,
    typename ...Rest,
    typename    T>
struct type_sequence_index<
    type_sequence<First, Rest ...>,
    T>
  : size_constant<
        std::is_same_v<First, T>
          ? 0
          : 1 + type_sequence_index_v<type_sequence<Rest ...>, T>>
{ };



/*!
 * @brief Determines the type located the given index in the type sequence.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam Index        The index value.
 */
template<
    typename    TypeSequence,
    std::size_t Index>
struct type_sequence_get;

template<
    typename TypeSequence,
    std::size_t Index>
using type_sequence_get_t
= typename type_sequence_get<TypeSequence, Index>::type;

template<
    typename    TypeSequence,
    std::size_t Index>
struct type_sequence_get
{ };

template<
    typename    First,
    typename ...Rest>
struct type_sequence_get<
    type_sequence<First, Rest ...>,
    0>
{
  using type
  = First;
};

template<
    typename    First,
    typename ...Rest,
    std::size_t Index>
struct type_sequence_get<
    type_sequence<First, Rest ...>,
    Index>
{
  static_assert(
      Index < type_sequence_size_v<type_sequence<First, Rest ...>>,
      "The index must be within the type sequence's size.");

  using type
  = type_sequence_get_t<type_sequence<Rest ...>, Index - 1>;
};



/*!
 * @brief Determines the type sequence obtained by concatenating the two given type sequences.
 *
 * @tparam Left  The type sequence on the left.
 * @tparam Right The type sequence on the right.
 */
template<
    typename Left,
    typename Right>
struct type_sequence_concatenate;

template<
    typename Left,
    typename Right>
using type_sequence_concatenate_t
= typename type_sequence_concatenate<Left, Right>::type;

template<
    typename Left,
    typename Right>
struct type_sequence_concatenate
{ };

template<
    typename ...Ls,
    typename ...Rs>
struct type_sequence_concatenate<
    type_sequence<Ls ...>,
    type_sequence<Rs ...>>
{
  using type
  = type_sequence<Ls ..., Rs ...>;
};



/*!
 * @brief Determines the type sequence with the given pack of types added to its end.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam Ts           The pack of types to append.
 */
template<
    typename    TypeSequence,
    typename ...Ts>
struct type_sequence_append;

template<
    typename    TypeSequence,
    typename ...Ts>
using type_sequence_append_t
= typename type_sequence_append<TypeSequence, Ts ...>::type;

template<
    typename    TypeSequence,
    typename ...Ts>
struct type_sequence_append
{
  using type
  = type_sequence_concatenate_t<TypeSequence, type_sequence<Ts ...>>;
};



/*!
 * @brief Sets the type located the given index in the type sequence to the given type.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam Index        The index value.
 * @tparam T            The set type.
 */
template<
    typename    TypeSequence,
    std::size_t Index,
    typename    T>
struct type_sequence_set;

template<
    typename    TypeSequence,
    std::size_t Index,
    typename    T>
using type_sequence_set_t
= type_sequence_set<TypeSequence, Index, T>::type;

template<
    typename    TypeSequence,
    std::size_t Index,
    typename    T>
struct type_sequence_set
{ };

template<
    typename    First,
    typename ...Rest,
    typename    T>
struct type_sequence_set<
    type_sequence<First, Rest ...>,
    0,
    T>
{
  using type
  = type_sequence<T, Rest ...>;
};

template<
    typename    First,
    typename ...Rest,
    std::size_t Index,
    typename    T>
struct type_sequence_set<
    type_sequence<First, Rest ...>,
    Index,
    T>
{
  static_assert(
      Index < type_sequence_size_v<type_sequence<First, Rest ...>>,
      "The index must be within the type sequence's size.");

  using type
  = type_sequence_concatenate_t<
      type_sequence<First>,
      type_sequence_set_t<type_sequence<Rest ...>, Index - 1, T>>;
};



/*!
 * @brief Determines the type sequence with the first occurrence of the given type erased from it.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam T            The type to erase.
 */
template<
    typename TypeSequence,
    typename T>
struct type_sequence_erase;

template<
    typename TypeSequence,
    typename T>
using type_sequence_erase_t
= typename type_sequence_erase<TypeSequence, T>::type;

template<
    typename TypeSequence,
    typename T>
struct type_sequence_erase
{ };

template<
    typename T>
struct type_sequence_erase<
    type_sequence<>,
    T>
{
  using type
  = type_sequence<>;
};

template<
    typename    First,
    typename ...Rest,
    typename    T>
struct type_sequence_erase<
    type_sequence<First, Rest ...>,
    T>
{
  using type
  = std::conditional_t<
      std::is_same_v<T, First>,
      type_sequence<Rest ...>,
      type_sequence_concatenate_t<
          type_sequence<First>,
          type_sequence_erase_t<
              type_sequence<Rest ...>,
              T>>>;
};



/*!
 * @brief Determines the type sequence obtained by keeping the types which verifies the given
 *   predicate.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam Pred         The predicate type.
 *
 * @note The predicate type must expose a static constexpr bool value attribute.
 */
template<
    typename                    TypeSequence,
    template<typename> typename Pred>
struct type_sequence_filter;

template<
    typename                    TypeSequence,
    template<typename> typename Pred>
using type_sequence_filter_t
= typename type_sequence_filter<TypeSequence, Pred>::type;

template<
    typename                    TypeSequence,
    template<typename> typename Pred>
struct type_sequence_filter
{ };

template<template<typename> typename Pred>
struct type_sequence_filter<type_sequence<>, Pred>
{
  using type
  = type_sequence<>;
};

template<
    typename                    First,
    typename                 ...Rest,
    template<typename> typename Pred>
struct type_sequence_filter<type_sequence<First, Rest ...>, Pred>
{
  static_assert(
      std::is_same_v<decltype(Pred<First>::value), bool>,
      "The predicate must expose a static constexpr bool value attribute");

  using type
  = std::conditional_t<
      Pred<First>::value,
      type_sequence_concatenate_t<
          type_sequence<First>,
          type_sequence_filter_t<
              type_sequence<Rest ...>,
              Pred>>,
      type_sequence_filter_t<
          type_sequence<Rest ...>,
          Pred>>;
};



/*!
 * @brief Determines the type sequence obtained by applying the given transformation to each type.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam Meta         The transformation to apply to each type.
 *
 * @note The transformation type must expose a type alias named 'type'.
 */
template<
    typename                    TypeSequence,
    template<typename> typename Meta>
struct type_sequence_map;

template<
    typename                    TypeSequence,
    template<typename> typename Meta>
using type_sequence_map_t
= typename type_sequence_map<TypeSequence, Meta>::type;

template<
    typename                    TypeSequence,
    template<typename> typename Meta>
struct type_sequence_map
{ };

template<
    typename ...                Ts,
    template<typename> typename Meta>
struct type_sequence_map<
    type_sequence<Ts ...>,
    Meta>
{
  using type = type_sequence<typename Meta<Ts>::type ...>;
};



/*!
 * @brief Determines the type sequence obtained all duplicates of present types in the given
 *   sequence.
 *
 * @tparam TypeSequence The type sequence.
 *
 * @note The second template parameter exists as an implementation requirement and should not be
 *   specialized manually.
 */
template<
    typename TypeSequence,
    typename VisitedSequence = type_sequence<>>
struct type_sequence_unique;

template<
    typename TypeSequence,
    typename VisitedSequence = type_sequence<>>
using type_sequence_unique_t
= typename type_sequence_unique<TypeSequence, VisitedSequence>::type;

template<
    typename TypeSequence,
    typename VisitedSequence>
struct type_sequence_unique
{ };

template<typename ...Visited>
struct type_sequence_unique<
    type_sequence<>,
    type_sequence<Visited ...>>
{
  using type
  = type_sequence<Visited ...>;
};

template<
    typename    First,
    typename ...Rest,
    typename ...Visited>
struct type_sequence_unique<
    type_sequence<First, Rest ...>,
    type_sequence<Visited ...>>
{
  using type
  = type_sequence_unique_t<
      type_sequence<Rest ...>,
      std::conditional_t<
          (std::is_same_v<First, Visited> || ...),
          type_sequence<Visited ...>,
          type_sequence<Visited ..., First>>>;
};



/*!
 * @brief Determines whether the given type sequence is unique (i.e. contains no duplicated types).
 *
 * @tparam TypeSequence The type sequence.
 */
template<typename TypeSequence>
struct type_sequence_is_unique;

template<typename TypeSequence>
inline constexpr
bool
type_sequence_is_unique_v
= type_sequence_is_unique<TypeSequence>::value;

template<typename TypeSequence>
struct type_sequence_is_unique
  : bool_constant<
        std::is_same_v<
            TypeSequence,
            type_sequence_unique_t<TypeSequence>>>
{ };



/*!
 * @brief Determines the type sequence obtained by replacing in the type sequence
 *   all present type sequences by their pack of types.
 *
 * @tparam TypeSequence The type sequence.
 *
 * @note Only "top-level" type sequences are replaced by their packs.
 *   That is, if the given type sequence contains a type sequence which itself contains a type
 *   sequence, the latter will remain a type sequence.
 */
template<typename TypeSequence>
struct type_sequence_flatten;

template<typename TypeSequence>
using type_sequence_flatten_t
= typename type_sequence_flatten<TypeSequence>::type;

template<typename TypeSequence>
struct type_sequence_flatten
{ };

template<>
struct type_sequence_flatten<type_sequence<>>
{
  using type
  = type_sequence<>;
};

template<
    typename    First,
    typename ...Rest>
struct type_sequence_flatten<
    type_sequence<First, Rest ...>>
{
private:
  template<typename T>
  struct expand
  {
    using type
    = type_sequence<T>;
  };

  template<typename ...Ts>
  struct expand<type_sequence<Ts ...>>
  {
    using type
    = type_sequence<Ts ...>;
  };

  template<typename T>
  using expand_t = typename expand<T>::type;

public:
  using type
  = type_sequence_concatenate_t<
      expand_t<First>,
      type_sequence_flatten_t<type_sequence<Rest ...>>>;
};



/*!
 * @brief Determines the type sequence obtained by keeping only the type occurrences
 *   present in the left type sequence and absent in the right type sequence.
 *
 * @tparam Left  The type sequence to the left of the subtraction.
 * @tparam Right The type sequence to the right of the subtraction.
 *
 * @note This trait models a typical list difference with multiplicity and stable order.
 */
template<
    typename Left,
    typename Right>
struct type_sequence_difference;

template<
    typename Left,
    typename Right>
using type_sequence_difference_t
= typename type_sequence_difference<Left, Right>::type;

template<
    typename Left,
    typename Right>
struct type_sequence_difference
{ };

template<typename ...Ls>
struct type_sequence_difference<
    type_sequence<Ls ...>,
    type_sequence<>>
{
  using type
  = type_sequence<Ls ...>;
};

template<
    typename ...Ls,
    typename    RFirst,
    typename ...RRest>
struct type_sequence_difference<
    type_sequence<Ls ...>,
    type_sequence<RFirst, RRest ...>>
{
  using type
  = std::conditional_t<
      type_sequence_contains_v<
          type_sequence<Ls ...>,
          RFirst>,
      type_sequence_difference_t<
          type_sequence_erase_t<type_sequence<Ls ...>, RFirst>,
          type_sequence<RRest ...>>,
      type_sequence_difference_t<
          type_sequence<Ls ...>,
          type_sequence<RRest ...>>>;
};



/*!
 * @brief Determines the std::tuple type with the same of types as the given type sequence.
 *
 * @tparam TypeSequence The type sequence.
 */
template<typename TypeSequence>
struct type_sequence_tuple;

template<typename TypeSequence>
using type_sequence_tuple_t
= typename type_sequence_tuple<TypeSequence>::type;

template<typename TypeSequence>
struct type_sequence_tuple
{ };

template<typename ...Ts>
struct type_sequence_tuple<type_sequence<Ts ...>>
{
  using type
  = std::tuple<Ts ...>;
};



template<typename ...Ts>
struct type_sequence
{
  static constexpr
  std::size_t
  size
  = type_sequence_size_v<type_sequence>;

  template<typename T>
  static constexpr
  std::size_t
  count
  = type_sequence_count_v<type_sequence, T>;

  template<typename T>
  static constexpr
  bool
  contains
  = type_sequence_contains_v<type_sequence, T>;

  template<typename T>
  static constexpr
  std::size_t
  index
  = type_sequence_index_v<type_sequence, T>;

  template<std::size_t Index>
  using get
  = type_sequence_get_t<type_sequence, Index>;

  template<typename TypeSequence>
  using concatenate
  = type_sequence_concatenate_t<type_sequence, TypeSequence>;

  template<typename ...Us>
  using append
  = type_sequence_append_t<type_sequence, Us ...>;

  template<
      std::size_t Index,
      typename    T>
  using set
  = type_sequence_set_t<type_sequence, Index, T>;

  template<typename T>
  using erase
  = type_sequence_erase_t<type_sequence, T>;

  template<template<typename> typename Pred>
  using filter
  = type_sequence_filter_t<type_sequence, Pred>;

  template<template<typename> typename Meta>
  using map
  = type_sequence_map_t<type_sequence, Meta>;

  using unique
  = type_sequence_unique_t<type_sequence>;

  static constexpr
  bool
  is_unique
  = type_sequence_is_unique_v<type_sequence>;

  using flatten
  = type_sequence_flatten_t<type_sequence>;

  template<typename TypeSequence>
  using difference
  = type_sequence_difference_t<type_sequence, TypeSequence>;

  using tuple
  = type_sequence_tuple_t<type_sequence>;
};



/*!
 * @brief Determines the type sequence with the same of types as the given std::tuple.
 *
 * @tparam Tuple The tuple.
 */
template<typename Tuple>
struct tuple_to_type_sequence;

template<typename Tuple>
struct tuple_to_type_sequence
{ };

template<typename ...Ts>
struct tuple_to_type_sequence<std::tuple<Ts ...>>
{
  using type
  = type_sequence<Ts ...>;
};



/*!
 * @brief Determines whether the given type is a specialization of type_sequence.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct specializes_type_sequence;

template<typename T>
inline constexpr
bool
specializes_type_sequence_v
= specializes_type_sequence<T>::value;

template<typename T>
struct specializes_type_sequence
  : bool_constant<false>
{ };

template<typename ...Ts>
struct specializes_type_sequence<
    type_sequence<Ts ...>>
  : bool_constant<true>
{ };


}

#endif // HEIM_TYPE_SEQUENCE_HPP