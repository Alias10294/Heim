#ifndef HEIM_LIB_TYPE_SEQUENCE_HPP
#define HEIM_LIB_TYPE_SEQUENCE_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace heim
{
/*!
 * @brief A type used to represent a sequence of types.
 *
 * @tparam Ts The types in the sequence.
 *
 * @details Makes most use of variadic templates and partial specialization to
 *   provide compile-time logic and algorithms on types.
 */
template<typename ...Ts>
struct type_sequence;


namespace detail
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
struct type_sequence_index<T, First, Rest ...>
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
struct type_sequence_get<0, First, Rest ...>
{
  using type
  = First;

};

template<std::size_t I, typename First, typename ...Rest>
struct type_sequence_get<I, First, Rest ...>
{
private:
  static_assert(
      I < 1 + sizeof...(Rest),
      "heim::detail::type_sequence_get<I, First, Rest ...>: "
          "I < 1 + sizeof...(Rest);");

public:
  using type
  = type_sequence_get<I - 1, Rest ...>::type;

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
struct type_sequence_unique<type_sequence<Seen ...>, First, Rest ...>
{
  using type
  = type_sequence_unique<
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
  = type_sequence_concat<
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
struct type_sequence_filter<Pred, First, Rest ...>
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
struct type_sequence_remove<0, First, Rest ...>
{
  using type
  = type_sequence<Rest ...>;
};

template<std::size_t I, typename First, typename ...Rest>
struct type_sequence_remove<I, First, Rest ...>
{
private:
  static_assert(
      I < 1 + sizeof...(Rest),
      "heim::type_sequence_remove<I, First, Rest ...>: "
          "I < 1 + sizeof...(Rest);");

public:
  using type
  = type_sequence_concat<
      type_sequence<First>,
      typename type_sequence_remove<I - 1, Rest ...>::type>::type;

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
struct type_sequence_erase<T, First, Rest ...>
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
      type_sequence<Ts ...>::template contains<First>,
      typename type_sequence_difference<
          type_sequence<Rest ...>,
          typename type_sequence<Ts ...>::template erase<First>>::type,
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
      type_sequence<Ts ...>::template contains<First>,
      typename type_sequence_concat<
          type_sequence<First>,
          typename type_sequence_intersect<
              type_sequence<Rest ...>,
              typename type_sequence<Ts ...>::template
                  erase<First>>::type>::type,
      typename type_sequence_intersect<
          type_sequence<Rest ...>,
          type_sequence<Ts ...>>::type>;

};



template<
    std::size_t N,
    typename    Seq,
    bool        IsZero = (N == 0)>
struct type_sequence_subsequences_detail;

template<typename ...Ts>
struct type_sequence_subsequences_detail<0, type_sequence<Ts ...>, true>
{
  using type
  = type_sequence<type_sequence<>>;

};

template<std::size_t N>
struct type_sequence_subsequences_detail<N, type_sequence<>, false>
{
  static_assert(
      N > 0,
      "heim::detail::type_sequence_subsequences_detail<N, Seq, IsZero>: "
          "N > 0;");

  using type
  = type_sequence<>;

};

template<
    std::size_t N,
    typename    First,
    typename ...Rest>
struct type_sequence_subsequences_detail<
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
  = type_sequence_subsequences_detail<
      N - 1,
      type_sequence<Rest ...>>::type::template
      map<prefix>;

  using rest_t
  = type_sequence_subsequences_detail<
      N,
      type_sequence<Rest ...>>::type;

public:
  using type
  = type_sequence_concat<first_t, rest_t>::type;

};

template<std::size_t N, typename Seq>
struct type_sequence_subsequences
{
  using type
  = type_sequence_subsequences_detail<N, Seq>::type;

};




template<std::size_t N, typename TypeSeq>
struct type_sequence_power_sequence;

template<typename ...Ts>
struct type_sequence_power_sequence<0, type_sequence<Ts ...>>
{
  using type
  = type_sequence_subsequences<0, type_sequence<Ts ...>>::type;

};

template<std::size_t N, typename ...Ts>
struct type_sequence_power_sequence<N, type_sequence<Ts ...>>
{
  using type
  = type_sequence_concat<
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
  = type_sequence<std::conditional_t<
      type_sequence<Ts ...>::template contains<Us>,
      Us,
      void> ...>::dense;

};


} // namespace detail


template<typename ...Ts>
struct type_sequence
{
private:
  //! @cond INTERNAL

  /*!
   * @brief Determines whether @code T@endcode is not void.
   *
   * @tparam T The type to determine for.
   */
  template<typename T>
  struct is_not_void
  {
    constexpr
    static bool value
    = !std::is_same_v<T, void>;

  };

public:
  /*!
   * @brief The size of the sequence.
   */
  static constexpr std::size_t
  size
  = sizeof...(Ts);

  /*!
   * @brief Whether the sequence is empty.
   */
  static constexpr bool
  empty
  = size == 0;


  /*!
   * @brief The number of times @code T@endcode appears in the
   *   sequence.
   *
   * @tparam T The type to count.
   */
  template<typename T>
  static constexpr std::size_t
  count
  = (0 + ... + (std::is_same_v<T, Ts> ? 1 : 0));



  /*!
   * @brief Whether @code T@endcode is present in the sequence.
   *
   * @tparam T The type to check for.
   */
  template<typename T>
  static constexpr bool
  contains
  = count<T> > 0;


  /*!
   * @brief The index in the sequence of @code T@endcode.
   *
   * @tparam T The type to get the index of.
   * @note If @code contains_v<T> == false@endcode then
   *    @code index<T>::value = size_v@endcode.
   */
  template<typename T>
  static constexpr std::size_t
  index
  = detail::type_sequence_index<T, Ts...>::value;


  /*!
   * @brief The type located at index @code I@endcode in the
   *   sequence.
   *
   * @tparam I The index to get the corresponding type of.
   */
  template<std::size_t I>
  using get
  = detail::type_sequence_get<I, Ts ...>::type;



  /*!
   * @brief The sequence produced by concatenation of this sequence
   *   and @code TypeSeq@endcode.
   *
   * @tparam TypeSeq The type sequence to concatenate.
   */
  template<typename TypeSeq>
  using concat
  = detail::type_sequence_concat<type_sequence, TypeSeq>::type;


  /*!
   * @brief The sequence produced by extending it with
   *   @code Us ...@endcode.
   */
  template<typename ...Us>
  using extend
  = concat<type_sequence<Us ...>>;



  /*!
   * @brief The sequence produced when filtering out duplicates of
   *   the same types.
   */
  using unique
  = detail::type_sequence_unique<type_sequence<>, Ts ...>::type;

  /*!
   * @brief Whether the sequence is the same as its unique type.
   */
  constexpr static bool
  is_unique = std::is_same_v<type_sequence, unique>;


  /*!
   * @brief The sequence produced by flattening once all
   *   type_sequence types in its sequence.
   */
  using flat
  = detail::type_sequence_flat<Ts ...>::type;


  /*!
   * @brief The sequence produced by filtering out types that verify
   *   @code Pred@endcode.
   *
   * @tparam Pred The predicate type used to filter types.
   * @pre @code Pred@endcode must expose a
   *   @code constexpr static bool value@endcode attribute.
   */
  template<template<typename> typename Pred>
  using filter
  = detail::type_sequence_filter<Pred, Ts ...>::type;


  /*!
   * @brief The sequence produced by filtering out all void types.
   */
  using dense
  = filter<is_not_void>;



  /*!
   * @brief The sequence produced after the type at index
   *   @code I@endcode is removed.
   *
   * @tparam I The index of the type to remove.
   */
  template<std::size_t I>
  using remove
  = detail::type_sequence_remove<I, Ts ...>::type;


  /*!
   * @brief The sequence produced after the first @code T@endcode is
   *   erased from it.
   *
   * @tparam T The type to erase from the sequence.
   * @note If @code contains_v<T> == false@endcode, then
   *   @code std::is_same_v<type_sequence, erase<T>::type> == true@endcode.
   */
  template<typename T>
  using erase
  = detail::type_sequence_erase<T, Ts ...>::type;



  /*!
   * @brief The sequence of the types only contained in this
   *   sequence.
   *
   * @tparam TypeSeq The sequence to "subtract" to this sequence.
   */
  template<typename TypeSeq>
  using difference
  = detail::type_sequence_difference<
      TypeSeq,
      type_sequence>::type;


  /*!
   * @brief The sequence of types contained in both this sequence
   *   and @code TypeSeq@endcode.
   *
   * @tparam TypeSeq The sequence to intersect with.
   */
  template<typename TypeSeq>
  using intersect
  = detail::type_sequence_intersect<
      TypeSeq,
      type_sequence>::type;



  /*!
   * @brief The sequence of types after each type is applied
   *   @code Meta@endcode.
   *
   * @tparam Meta The type used to map the types.
   * @pre @code Meta@endcode must expose a typename @code type@endcode.
   */
  template<template<typename> typename Meta>
  using map
  = type_sequence<typename Meta<Ts>::type ...>;



  /*!
   * @brief The sequence of all subsequences of size
   *   @code N@endcode.
   *
   * @tparam N The size of the subsequences to produce.
   */
  template<std::size_t N>
  using subsequences
  = detail::type_sequence_subsequences<N, type_sequence>::type;


  /*!
   * @brief The sequence produced by reordering it in the same order
   *   as @code TypeSeq@endcode. All types not in @code TypeSeq@endcode are
   *   erased.
   *
   * @tparam TypeSeq The sequence to follow the order of.
   */
  template<typename TypeSeq>
  using induce_order
  = detail::type_sequence_induce_order<TypeSeq, Ts ...>::type;



  /*!
   * @brief The corresponding std::tuple specialization for this
   *   type_sequence.
   */
  using to_tuple
  = std::tuple<Ts ...>;


};



namespace detail
{
template<typename>
struct is_type_sequence
  : std::false_type
{};

template<typename ...Ts>
struct is_type_sequence<type_sequence<Ts ...>>
  : std::true_type
{ };


}


/*!
 * @brief Determines whether @code T@endcode specializes type_sequence or not.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct is_type_sequence
{
  constexpr
  static bool value
  = detail::is_type_sequence<std::remove_cvref_t<T>>::value;

};


template<typename T>
constexpr
inline bool is_type_sequence_v
= is_type_sequence<T>::value;



/*!
 * @brief Determines the corresponding type_sequence specialization to the
 *   tuple @code T@endcode.
 *
 * @tparam Tuple The tuple to extract the sequence of types of.
 */
template<typename Tuple>
struct to_type_sequence;

template<typename Tuple>
using to_type_sequence_t
= to_type_sequence<Tuple>::type;

template<typename ...Ts>
struct to_type_sequence<std::tuple<Ts ...>>
{
  using type
  = type_sequence<Ts ...>;

};


} // namespace heim


#endif // HEIM_LIB_TYPE_SEQUENCE_HPP
