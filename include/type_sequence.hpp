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
          : 1 + type_sequence_index_v<type_sequence<Rest ...>, T>>
{ };



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



template<typename TypeSequence>
struct type_sequence_is_unique
  : bool_constant<
        std::is_same_v<
            TypeSequence,
            type_sequence_unique_t<TypeSequence>>>
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

  template<typename T>
  using expand_t = typename expand<T>::type;

public:
  using type
  = type_sequence_concatenate_t<
      expand_t<First>,
      type_sequence_flatten_t<type_sequence<Rest ...>>>;
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
      type_sequence_difference_t<
          type_sequence_erase_t<type_sequence<Ls ...>, RFirst>,
          type_sequence<RRest ...>>,
      type_sequence_difference_t<
          type_sequence<Ls ...>,
          type_sequence<RRest ...>>>;
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