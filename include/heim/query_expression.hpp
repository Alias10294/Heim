#ifndef HEIM_QUERY_EXPRESSION_HPP
#define HEIM_QUERY_EXPRESSION_HPP

#include "type_sequence.hpp"

namespace heim
{
/*!
 * @brief A type describing a query's components' properties.
 *
 * @details Uses multiple type sequences in order to describe the query.
 *
 * @tparam IncludeSeq The sequence of component types included in the query.
 * @tparam ExcludeSeq The sequence of component types excluded from the query.
 */
template<
    typename IncludeSeq = type_sequence<>,
    typename ExcludeSeq = type_sequence<>>
struct query_expression;

template<
    typename IncludeSeq,
    typename ExcludeSeq>
struct query_expression
{
  using include_sequence = IncludeSeq;
  using exclude_sequence = ExcludeSeq;

  static_assert(
      specializes_type_sequence_v<include_sequence>,
      "include_sequence must be a specialization of type_sequence.");
  static_assert(
      specializes_type_sequence_v<exclude_sequence>,
      "exclude_sequence must be a specialization of type_sequence.");
  static_assert(
      include_sequence::is_unique
   && exclude_sequence::is_unique
   && include_sequence::template difference<exclude_sequence>::size == include_sequence::size,
      "include_sequence and exclude_sequence must be sequences of different unique component types");
  static_assert(
      std::is_same_v<
          include_sequence,
          typename include_sequence::template map<std::remove_reference>>,
      "include_sequence must not contain reference types.");
  static_assert(
      std::is_same_v<
          exclude_sequence,
          typename exclude_sequence::template map<std::remove_reference>>,
      "exclude_sequence must not contain reference types.");


  template<typename ...Components>
  using include
  = query_expression<
      typename include_sequence::template append<Components ...>,
      exclude_sequence>;

  template<typename ...Components>
  using exclude
  = query_expression<
      include_sequence,
      typename exclude_sequence::template append<Components ...>>;
};



/*!
 * @brief Determines whether the given type is a specialization of query_expression.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct specializes_query_expression;

template<typename T>
inline constexpr
bool
specializes_query_expression_v
= specializes_query_expression<T>::value;

template<typename T>
struct specializes_query_expression
  : bool_constant<false>
{ };

template<
    typename IncludeSeq,
    typename ExcludeSeq>
struct specializes_query_expression<
    query_expression<IncludeSeq, ExcludeSeq>>
  : bool_constant<true>
{ };


}

#endif // HEIM_QUERY_EXPRESSION_HPP