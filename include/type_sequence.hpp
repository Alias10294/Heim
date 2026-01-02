#ifndef HEIM_TYPE_SEQUENCE_HPP
#define HEIM_TYPE_SEQUENCE_HPP

#include <cstddef>
#include <type_traits>
#include "utility.hpp"

namespace heim
{
/*!
 * @brief A type describing a sequence of types.
 *
 * @tparam Ts The pack of types.
 */
template<
    typename ...Ts>
struct type_sequence;


/*!
 * @brief Determines the size of the type sequence TypeSequence.
 *
 * @tparam TypeSequence The type sequence.
 */
template<
    typename TypeSequence>
struct type_sequence_size
{ };

template<
    typename ...Ts>
struct type_sequence_size<type_sequence<Ts ...>>
  : size_constant<
        sizeof...(Ts)>
{ };

template<
    typename TypeSequence>
static constexpr
std::size_t
type_sequence_size_v
= type_sequence_size<TypeSequence>::value();


/*!
 * @brief Determines the number of times T appears inTypeSequence.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam T            The type to count.
 */
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
  : size_constant<
        (0 + ... + (std::is_same_v<Ts, T> ? 1 : 0))>
{ };

template<
    typename TypeSequence,
    typename T>
inline constexpr
std::size_t
type_sequence_count_v
= type_sequence_count<TypeSequence, T>::value();


/*!
 * @brief Determines whether T appears at least once in TypeSequence.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam T            The type to check for.
 */
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
  : bool_constant<
        (type_sequence_count_v<type_sequence<Ts ...>, T> > 0)>
{ };

template<
    typename TypeSequence,
    typename T>
inline constexpr
bool
type_sequence_contains_v
= type_sequence_contains<TypeSequence, T>::value();


/*!
 * @brief Determines the index of T in TypeSequence.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam T            The type to get the index of.
 */
template<
    typename TypeSequence,
    typename T>
struct type_sequence_index
{ };

template<
    typename T>
struct type_sequence_index<
    type_sequence<>,
    T>
  : size_constant<
        0>
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
          : 1 + type_sequence_index<type_sequence<Rest ...>, T>::value()>
{
  static_assert(
      type_sequence_contains_v<type_sequence<First, Rest ...>, T>);
};

template<
    typename TypeSequence,
    typename T>
inline constexpr
std::size_t
type_sequence_index_v
= type_sequence_index<TypeSequence, T>::value();


/*!
 * @brief Determines the type in TypeSequence at index Index.
 *
 * @tparam TypeSequence The type sequence
 * @tparam Index        The index to get the type of.
 */
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
    std::size_t{0}>
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
      Index < type_sequence_size_v<type_sequence<First, Rest ...>>);

  using type
  = typename type_sequence_get<
      type_sequence<Rest ...>,
      Index - 1>
      ::type;
};

template<
    typename TypeSequence,
    std::size_t Index>
using type_sequence_get_t
= typename type_sequence_get<TypeSequence, Index>::type;


/*!
 * @brief Determines the type sequence resulting in the concatenation of
 *   type sequences Left and Right.
 *
 * @tparam Left  The type sequence to the left  of the concatenation.
 * @tparam Right The type sequence to the right of the concatenation.
 */
template<
    typename Left,
    typename Right>
struct type_sequence_concatenate;

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
    typename Left,
    typename Right>
using type_sequence_concatenate_t
= typename type_sequence_concatenate<Left, Right>::type;


/*!
 * @brief Determines the type sequence resulting in the appending of the pack
 *   of types Ts ... to TypeSequence.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam Ts           The pack of types to append.
 */
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
    typename ...Ts>
using type_sequence_append_t
= typename type_sequence_append<TypeSequence, Ts ...>::type;


/*!
 * @brief Determines the type sequence resulting in the erasure of the first
 *   type T in order from TypeSequence.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam T            The type to erase.
 */
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
    typename TypeSequence,
    typename T>
using type_sequence_erase_t
= typename type_sequence_erase<TypeSequence, T>::type;


/*!
 * @brief Determines the type sequence where types that do not satisfy the
 *   predicate Pred are removed.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam Pred         The predicate to use.
 *
 * @warning Pred must expose a boolean consteval methode named ::value().
 */
template<
    typename TypeSequence,
    template<typename> typename Pred>
struct type_sequence_filter
{ };

template<
    template<typename> typename Pred>
struct type_sequence_filter<type_sequence<>, Pred>
{
  using type
  = type_sequence<>;
};

template<
    typename    First,
    typename ...Rest,
    template<typename> typename Pred>
struct type_sequence_filter<type_sequence<First, Rest ...>, Pred>
{
  using type
  = std::conditional_t<
      Pred<First>::value(),
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
    template<typename> typename Pred>
using type_sequence_filter_t
= typename type_sequence_filter<TypeSequence, Pred>::type;


/*!
 * @brief Determines the type sequence resulting in the removal of duplicate
 *   types in TypeSequence.
 *
 * @tparam TypeSequence    The type sequence.
 * @tparam VisitedSequence The type sequence of visited types.
 *
 * @warning this struct should be used without specializing VisitedSequence.
 */
template<
    typename TypeSequence,
    typename VisitedSequence = type_sequence<>>
struct type_sequence_unique
{ };

template<
    typename ...Visited>
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

template<
    typename TypeSequence>
using type_sequence_unique_t
= typename type_sequence_unique<TypeSequence>::type;


/*!
 * @brief Determines whether TypeSequence describes a sequence with no
 *   duplicate types.
 *
 * @tparam TypeSequence The type sequence.
 */
template<
    typename TypeSequence>
struct type_sequence_is_unique
  : bool_constant<
        std::is_same_v<
            TypeSequence,
            typename type_sequence_unique<TypeSequence>::type>>
{ };

template<
    typename TypeSequence>
inline constexpr bool
type_sequence_is_unique_v
= type_sequence_is_unique<TypeSequence>::value();


/*!
 * @brief Determines the type sequence when type sequences in TypeSequence are
 *   replaced by their pack of types.
 *
 * @tparam TypeSequence The type sequence.
 */
template<
    typename TypeSequence>
struct type_sequence_flatten
{ };

template<>
struct type_sequence_flatten<
    type_sequence<>>
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
  template<
      typename T>
  struct expand
  {
    using type
    = type_sequence<T>;
  };

  template<
      typename ...Ts>
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
    typename TypeSequence>
using type_sequence_flatten_t
= typename type_sequence_flatten<TypeSequence>::type;


/*!
 * @brief Determines the sequence resulting in the difference between the types
 *   in type sequences Left and Right.
 *
 * @tparam Left  The type sequence to erase types from.
 * @tparam Right The type sequence of types to try to erase.
 */
template<
    typename Left,
    typename Right>
struct type_sequence_difference
{ };

template<
    typename ...Ls>
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

template<
    typename Left,
    typename Right>
using type_sequence_difference_t
= typename type_sequence_difference<Left, Right>::type;


/*!
 * @brief Determines the std::tuple type equivalent to TypeSequence.
 *
 * @tparam TypeSequence The type sequence.
 */
template<
    typename TypeSequence>
struct type_sequence_tuple
{ };

template<
    typename ...Ts>
struct type_sequence_tuple<
    type_sequence<Ts ...>>
{
  using type
  = std::tuple<Ts ...>;
};

template<
    typename TypeSequence>
using type_sequence_tuple_t
= typename type_sequence_tuple<TypeSequence>::type;





template<
    typename ...Ts>
struct type_sequence
{
  static consteval
  std::size_t
  size()
  noexcept
  {
    return type_sequence_size_v<type_sequence>;
  }

  template<
      typename T>
  static consteval
  std::size_t
  count()
  noexcept
  {
    return type_sequence_count_v<type_sequence, T>;
  }

  template<
      typename T>
  static consteval
  bool
  contains()
  noexcept
  {
    return type_sequence_contains_v<type_sequence, T>;
  }


  template<
      typename T>
  static consteval
  std::size_t
  index()
  noexcept
  {
    return type_sequence_index_v<type_sequence, T>;
  }

  template<
      std::size_t Index>
  using get
  = type_sequence_get_t<type_sequence, Index>;


  template<
      typename TypeSequence>
  using concatenate
  = type_sequence_concatenate_t<
      type_sequence,
      TypeSequence>;

  template<
      typename ...Us>
  using append
  = type_sequence_append_t<
      type_sequence,
      Us ...>;

  template<
      typename T>
  using erase
  = type_sequence_erase_t<
      type_sequence,
      T>;


  template<
      template<typename> typename Pred>
  using filter
  = type_sequence_filter_t<
      type_sequence,
      Pred>;

  using unique
  = type_sequence_unique_t<type_sequence>;

  static consteval
  bool
  is_unique()
  noexcept
  {
    return type_sequence_is_unique_v<type_sequence>;
  }

  using flatten
  = type_sequence_flatten_t<type_sequence>;

  template<
      typename TypeSequence>
  using difference
  = type_sequence_difference_t<
      type_sequence,
      TypeSequence>;


  using tuple
  = type_sequence_tuple_t<type_sequence>;
};



/*!
 * @brief Determines whether T is a specialization of type_sequence.
 *
 * @tparam T The type to determine for.
 */
template<
    typename T>
struct is_type_sequence
  : bool_constant<false>
{ };

template<
    typename T>
inline constexpr bool
is_type_sequence_v = is_type_sequence<T>::value;

template<
    typename ...Ts>
struct is_type_sequence<type_sequence<Ts ...>>
  : bool_constant<true>
{ };


} // namespace heim

#endif // HEIM_TYPE_SEQUENCE_HPP
