#ifndef HEIM_TYPE_SEQUENCE_HPP
#define HEIM_TYPE_SEQUENCE_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace heim
{
template<typename ...Ts>
struct type_sequence;



namespace internal
{
template<typename T, typename ...Ts>
struct type_sequence_index;

template<typename T>
struct type_sequence_index<T>
{
  constexpr
  static std::size_t value
  = 0;

};

template<typename T, typename First, typename ...Rest>
struct type_sequence_index<T, First, Rest...>
{
  constexpr
  static std::size_t value
  = std::is_same_v<T, First>
    ? 0
    : 1 + type_sequence_index<T, Rest ...>::value;

};



template<std::size_t I, typename ...Ts>
struct type_sequence_get;

template<typename First, typename ...Rest>
struct type_sequence_get<0, First, Rest...>
{
  using type
  = First;

};

template<std::size_t I, typename First, typename ...Rest>
struct type_sequence_get<I, First, Rest...>
{
  using type
  = typename type_sequence_get<I - 1, Rest...>::type;

};



template<typename TypeSeq, typename TailSeq>
struct type_sequence_concat;

template<typename ...Ts, typename ...Us>
struct type_sequence_concat<type_sequence<Ts ...>, type_sequence<Us ...>>
{
  using type
  = type_sequence<Ts ..., Us ...>;

};



template<typename SeenSeq, typename ...Ts>
struct type_sequence_unique;

template<typename ...Seen>
struct type_sequence_unique<type_sequence<Seen ...>>
{
  using type
  = type_sequence<Seen ...>;

};

template<typename ...Seen, typename First, typename ...Rest>
struct type_sequence_unique<type_sequence<Seen ...>, First, Rest...>
{
  using type
  = typename type_sequence_unique<
      std::conditional_t<
          (std::is_same_v<First, Seen> || ...),
          type_sequence<Seen ...>,
          type_sequence<Seen ..., First>>,
      Rest ...>::type;

};



template<typename ...Ts>
struct type_sequence_flat
{};

template<>
struct type_sequence_flat<>
{
  using type
  = type_sequence<>;

};

template<typename First, typename ...Rest>
struct type_sequence_flat<First, Rest...>
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
  = typename type_sequence_concat<
      typename expand<First>::type,
      typename type_sequence_flat<Rest ...>::type>::type;

};



template<template<typename> typename Pred, typename ...Ts>
struct type_sequence_filter;

template<template<typename> typename Pred>
struct type_sequence_filter<Pred>
{
  using type
  = type_sequence<>;

};

template<template<typename> typename Pred, typename First, typename ...Rest>
struct type_sequence_filter<Pred, First, Rest...>
{
  using type
  = std::conditional_t<
      Pred<First>::value,
      typename type_sequence_concat<
          type_sequence<First>,
          typename type_sequence_filter<Pred, Rest ...>::type>::type,
      typename type_sequence_filter<Pred, Rest ...>::type>;

};



template<std::size_t I, typename ...Ts>
struct type_sequence_remove;

template<std::size_t I>
struct type_sequence_remove<I>
{
  using type
  = type_sequence<>;

};

template<typename First, typename ...Rest>
struct type_sequence_remove<0, First, Rest...>
{
  using type
  = type_sequence<Rest ...>;
};

template<std::size_t I, typename First, typename ...Rest>
struct type_sequence_remove<I, First, Rest...>
{
  using type
  = typename type_sequence_concat<
      type_sequence<First>,
      typename type_sequence_remove<I - 1, Rest...>::type>::type;

};



template<typename T, typename... Ts>
struct type_sequence_erase;

template<typename T>
struct type_sequence_erase<T>
{
  using type
  = type_sequence<>;

};

template<typename T, typename First, typename ...Rest>
struct type_sequence_erase<T, First, Rest...>
{
  using type
  = std::conditional_t<
      std::is_same_v<T, First>,
      type_sequence<Rest ...>,
      typename type_sequence_concat<
          type_sequence<First>,
          typename type_sequence_erase<T, Rest ...>::type>::type>;

};



template<typename DiffSeq, typename TypeSeq>
struct type_sequence_difference;

template<typename ...Ts>
struct type_sequence_difference<
    type_sequence<>,
    type_sequence<Ts ...>>
{
  using type
  = type_sequence<Ts ...>;

};

template<typename First, typename ...Rest, typename ...Ts>
struct type_sequence_difference<
    type_sequence<First, Rest ...>,
    type_sequence<Ts ...>>
{
  using type
  = std::conditional_t<
      type_sequence<Ts ...>::template contains_v<First>,
      typename type_sequence_difference<
          type_sequence<Rest ...>,
          typename type_sequence<Ts ...>::template erase_t<First>>::type,
      typename type_sequence_difference<
          type_sequence<Rest ...>,
          type_sequence<Ts ...>>::type>;

};



template<typename DiffSeq, typename TypeSeq>
struct type_sequence_intersect;

template<typename ...Ts>
struct type_sequence_intersect<
    type_sequence<>,
    type_sequence<Ts ...>>
{
  using type
  = type_sequence<>;

};

template<typename First, typename ...Rest, typename ...Ts>
struct type_sequence_intersect<
    type_sequence<First, Rest ...>,
    type_sequence<Ts ...>>
{
  using type
  = std::conditional_t<
      type_sequence<Ts ...>::template contains_v<First>,
      typename type_sequence_concat<
          type_sequence<First>,
          typename type_sequence_intersect<
              type_sequence<Rest ...>,
              typename type_sequence<Ts ...>::template
                  erase_t<First>>::type>::type,
      typename type_sequence_intersect<
          type_sequence<Rest ...>,
          type_sequence<Ts ...>>::type>;

};



template<
    std::size_t N,
    typename    Seq,
    bool        IsZero = (N == 0)>
struct type_sequence_subsequences_internal;

template<typename ...Ts>
struct type_sequence_subsequences_internal<0, type_sequence<Ts ...>, true>
{
  using type
  = type_sequence<type_sequence<>>;

};

template<std::size_t N>
struct type_sequence_subsequences_internal<N, type_sequence<>, false>
{
  static_assert(N > 0, "heim::type_sequence::subsequences: incoherent type");

  using type
  = type_sequence<>;

};

template<
    std::size_t N,
    typename    First,
    typename ...Rest>
struct type_sequence_subsequences_internal<
    N,
    type_sequence<First, Rest ...>, false>
{
private:
  template<typename Subset>
  struct prefix;

  template<typename ...Us>
  struct prefix<type_sequence<Us ...>>
  {
    using type
    = type_sequence<First, Us ...>;

  };

  using first_t
  = typename type_sequence_subsequences_internal<
      N - 1,
      type_sequence<Rest...>>::type::template
      map_t<prefix>;

  using rest_t
  = typename type_sequence_subsequences_internal<
      N,
      type_sequence<Rest...>>::type;

public:
  using type
  = typename type_sequence_concat<first_t, rest_t>::type;

};

template<std::size_t N, typename Seq>
struct type_sequence_subsequences
{
  using type
  = typename type_sequence_subsequences_internal<N, Seq>::type;

};




template<std::size_t N, typename TypeSeq>
struct type_sequence_power_sequence;

template<typename ...Ts>
struct type_sequence_power_sequence<0, type_sequence<Ts ...>>
{
  using type
  = typename type_sequence_subsequences<0, type_sequence<Ts ...>>::type;

};

template<std::size_t N, typename ...Ts>
struct type_sequence_power_sequence<N, type_sequence<Ts ...>>
{
  using type
  = typename type_sequence_concat<
      typename type_sequence_power_sequence<
          N - 1,
          type_sequence<Ts ...>>::type,
      typename type_sequence_subsequences<
          N,
          type_sequence<Ts ...>>::type>::type;

};



template<typename RefSeq, typename ...Ts>
struct type_sequence_induce_order;

template<typename ...Us, typename ...Ts>
struct type_sequence_induce_order<type_sequence<Us ...>, Ts ...>
{
  using type
  = typename type_sequence<std::conditional_t<
      type_sequence<Ts ...>::template contains_v<Us>,
      Us,
      void> ...>::dense_t;

};

}



template<typename ...Ts>
struct type_sequence
{
private:
  template<typename T>
  struct is_not_void
  {
    constexpr
    static bool value
    = !std::is_same_v<T, void>;

  };

public:
  struct size
  {
    constexpr
    static std::size_t value
    = sizeof...(Ts);
  };

  constexpr
  static std::size_t size_v
  = size::value;



  struct empty
  {
    constexpr
    static bool value
    = size_v == 0;

  };

  constexpr
  static bool empty_v
  = empty::value;



  template<typename T>
  struct count
  {
    constexpr
    static std::size_t value
    = (0 + ... + (std::is_same_v<T, Ts> ? 1 : 0));

  };

  template<typename T>
  constexpr
  static std::size_t count_v
  = count<T>::value;



  template<typename T>
  struct contains
  {
    constexpr
    static bool value
    = count_v<T> > 0;

  };

  template<typename T>
  constexpr
  static bool contains_v
  = contains<T>::value;



  template<typename T>
  using index
  = internal::type_sequence_index<T, Ts...>;

  template<typename T>
  constexpr
  static std::size_t index_v
  = index<T>::value;



  template<std::size_t I>
  using get
  = internal::type_sequence_get<I, Ts ...>;

  template<std::size_t I>
  using get_t
  = typename get<I>::type;



  template<typename TypeSeq>
  struct concat;

  template<typename TypeSeq>
  using concat_t
  = typename concat<TypeSeq>::type;

  template<typename ...Us>
  struct concat<type_sequence<Us ...>>
  {
    using type
    = type_sequence<Ts ..., Us ...>;

  };



  template<typename ...Us>
  struct extend
  {
    using type
    = type_sequence<Ts ..., Us ...>;

  };

  template<typename ...Us>
  using extend_t
  = typename extend<Us ...>::type;



  using unique
  = internal::type_sequence_unique<type_sequence<>, Ts ...>;

  using unique_t
  = typename unique::type;



  using flat
  = internal::type_sequence_flat<Ts ...>;

  using flat_t
  = typename flat::type;



  template<template<typename> typename Pred>
  using filter
  = internal::type_sequence_filter<Pred, Ts ...>;

  template<template<typename> typename Pred>
  using filter_t
  = typename filter<Pred>::type;



  using dense
  = filter<is_not_void>;

  using dense_t
  = typename dense::type;



  template<std::size_t I>
  using remove
  = internal::type_sequence_remove<I, Ts ...>;

  template<std::size_t I>
  using remove_t
  = typename remove<I>::type;



  template<typename T>
  using erase
  = internal::type_sequence_erase<T, Ts ...>;

  template<typename T>
  using erase_t
  = typename erase<T>::type;



  template<typename TypeSeq>
  using difference
  = internal::type_sequence_difference<
      TypeSeq,
      type_sequence>;

  template<typename TypeSeq>
  using difference_t
  = typename difference<TypeSeq>::type;



  template<typename TypeSeq>
  using intersect
  = internal::type_sequence_intersect<
      TypeSeq,
      type_sequence>;

  template<typename TypeSeq>
  using intersect_t
  = typename intersect<TypeSeq>::type;



  template<template<typename> typename Meta>
  struct map
  {
    using type
    = type_sequence<typename Meta<Ts>::type ...>;

  };

  template<template<typename> typename Meta>
  using map_t
  = typename map<Meta>::type;



  template<std::size_t N>
  using subsequences
  = internal::type_sequence_subsequences<N, type_sequence>;

  template<std::size_t N>
  using subsequences_t
  = typename subsequences<N>::type;



  template<typename TypeSeq>
  using induce_order
  = internal::type_sequence_induce_order<TypeSeq, Ts ...>;

  template<typename TypeSeq>
  using induce_order_t
  = typename induce_order<TypeSeq>::type;

};



namespace internal
{
template<typename>
struct is_type_sequence
  : std::false_type
{};

template<typename ...Ts>
struct is_type_sequence<
    type_sequence<Ts...>>
  : std::true_type
{ };


}


template<typename T>
struct is_type_sequence
{
  constexpr
  static bool value
  = internal::is_type_sequence<std::remove_cvref_t<T>>::value;

};


template<typename T>
constexpr
inline bool is_type_sequence_v
= is_type_sequence<T>::value;



template<typename Tuple>
struct to_type_sequence;

template<typename Tuple>
using to_type_sequence_t
= typename to_type_sequence<Tuple>::type;

template<typename ...Ts>
struct to_type_sequence<std::tuple<Ts...>>
{
  using type
  = type_sequence<Ts ...>;

};


}

#endif // HEIM_TYPE_SEQUENCE_HPP
