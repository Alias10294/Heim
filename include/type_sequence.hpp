#ifndef HEIM_TYPE_SEQUENCE_HPP
#define HEIM_TYPE_SEQUENCE_HPP

#include <tuple>
#include "fwd.hpp"

namespace heim
{
template<typename ...Ts>
struct type_sequence
{
  static constexpr
  std::size_t
  size
  = type_sequence_size<type_sequence>::value;

  template<typename T>
  static constexpr
  std::size_t
  count
  = type_sequence_count<type_sequence, T>::value;

  template<typename T>
  static constexpr
  bool
  contains
  = type_sequence_contains<type_sequence, T>::value;

  template<typename T>
  static constexpr
  std::size_t
  index
  = type_sequence_index<type_sequence, T>::value;

  template<std::size_t Index>
  using get
  = typename type_sequence_get<type_sequence, Index>::type;

  template<typename TypeSequence>
  using concatenate
  = typename type_sequence_concatenate<type_sequence, TypeSequence>::type;

  template<typename ...Us>
  using append
  = typename type_sequence_append<type_sequence, Us ...>::type;

  template<typename T>
  using erase
  = typename type_sequence_erase<type_sequence, T>::type;

  template<template<typename> typename Pred>
  using filter
  = typename type_sequence_filter<type_sequence, Pred>::type;

  using unique
  = typename type_sequence_unique<type_sequence>::type;

  static constexpr
  bool
  is_unique
  = type_sequence_is_unique<type_sequence>::value;

  using flatten
  = typename type_sequence_flatten<type_sequence>::type;

  template<typename TypeSequence>
  using difference
  = typename type_sequence_difference<type_sequence, TypeSequence>::type;

  using tuple
  = typename type_sequence_tuple<type_sequence>::type;
};



template<typename>
struct type_sequence_size
{ };

template<typename ...Ts>
struct type_sequence_size<type_sequence<Ts ...>>
  : size_constant<sizeof...(Ts)>
{ };



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
          : 1 + type_sequence_index<type_sequence<Rest ...>, T>::value>
{
  static_assert(
      type_sequence_contains_v<type_sequence<First, Rest ...>, T>,
      "The given type must be present in the type sequence.");
};



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
  = typename type_sequence_get<type_sequence<Rest ...>, Index - 1>::type;
};



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



template<
    typename    TypeSequence,
    typename ...Ts>
struct type_sequence_append
{
  using type
  = type_sequence_concatenate_t<TypeSequence, type_sequence<Ts ...>>;
};



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
          typename type_sequence_erase<type_sequence<Rest ...>, T>::type>>;
};



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
      typename type_sequence_concatenate<
          type_sequence<First>,
          typename type_sequence_filter<
              type_sequence<Rest ...>,
              Pred>
              ::type>
          ::type,
      typename type_sequence_filter<
          type_sequence<Rest ...>,
          Pred>
          ::type>;
};



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
  = typename type_sequence_unique<
      type_sequence<Rest ...>,
      std::conditional_t<
          (std::is_same_v<First, Visited> || ...),
          type_sequence<Visited ...>,
          type_sequence<Visited ..., First>>>
      ::type;
};



template<typename TypeSequence>
struct type_sequence_is_unique
  : bool_constant<
        std::is_same_v<
            TypeSequence,
            typename type_sequence_unique<TypeSequence>::type>>
{ };



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

public:
  using type
  = type_sequence_concatenate_t<
      typename expand<First>::type,
      typename type_sequence_flatten<type_sequence<Rest ...>>::type>;
};



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
      typename type_sequence_difference<
          type_sequence_erase_t<type_sequence<Ls ...>, RFirst>,
          type_sequence<RRest ...>>
          ::type,
      typename type_sequence_difference<
          type_sequence<Ls ...>,
          type_sequence<RRest ...>>
          ::type>;
};



template<typename TypeSequence>
struct type_sequence_tuple
{ };

template<typename ...Ts>
struct type_sequence_tuple<type_sequence<Ts ...>>
{
  using type
  = std::tuple<Ts ...>;
};



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