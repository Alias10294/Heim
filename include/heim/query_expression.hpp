#ifndef HEIM_QUERY_EXPRESSION_HPP
#define HEIM_QUERY_EXPRESSION_HPP

#include <type_traits>
#include "component.hpp"
#include "identifier.hpp"
#include "type_sequence.hpp"
#include "utility.hpp"

namespace heim
{
/*!
 * @brief A type describing a query's components' properties.
 *
 * @details Uses multiple type sequences in order to describe the query.
 *
 * @tparam IncludeSeq The sequence of component types included in   the query.
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
      "heim::query_expression: include_sequence must be a specialization of type_sequence.");
  static_assert(
      specializes_type_sequence_v<exclude_sequence>,
      "heim::query_expression: exclude_sequence must be a specialization of type_sequence.");
  static_assert(
      include_sequence::is_unique
   && exclude_sequence::is_unique
   && include_sequence::template difference<exclude_sequence>::size == include_sequence::size,
      "heim::query_expression: include_sequence and exclude_sequence must be sequences of different "
      "unique component types");
  static_assert(
      std::is_same_v<
          include_sequence,
          typename include_sequence::template map<std::remove_reference>>,
      "heim::query_expression: include_sequence must not contain reference types.");
  static_assert(
      std::is_same_v<
          exclude_sequence,
          typename exclude_sequence::template map<std::remove_reference>>,
      "heim::query_expression: exclude_sequence must not contain reference types.");


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

private:
  template<typename Identifier>
  struct meta
  {
    static_assert(
        specializes_identifier_v<Identifier>,
        "heim::query_expression::meta: Identifier must be a specialization of identifier.");


    using bare_include_sequence = typename include_sequence::template map<std::remove_const>;
    using bare_exclude_sequence = typename exclude_sequence::template map<std::remove_const>;

    template<typename Component>
    struct not_tagged
      : bool_constant<!component_tag_value_v<Component>>
    { };

    using value_type_sequence
    = typename bare_include_sequence::template filter<not_tagged>;

    using reference_sequence
    = typename include_sequence
        ::template filter<not_tagged>
        ::template map   <std::add_lvalue_reference>;

    using const_reference_sequence
    = typename include_sequence
        ::template filter<not_tagged>
        ::template map   <std::add_const>
        ::template map   <std::add_lvalue_reference>;



    using value_type
    = typename type_sequence<Identifier>
        ::template concatenate<value_type_sequence>
        ::tuple;

    using reference
    = typename type_sequence<Identifier const &>
        ::template concatenate<reference_sequence>
        ::tuple;

    using const_reference
    = typename type_sequence<Identifier const &>
        ::template concatenate<const_reference_sequence>
        ::tuple;
  };

public:
  template<typename Identifier> using value_type      = typename meta<Identifier>::value_type;
  template<typename Identifier> using reference       = typename meta<Identifier>::reference;
  template<typename Identifier> using const_reference = typename meta<Identifier>::const_reference;
};


/*!
 * @brief Determines whether the given type is a specialization of query_expression.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct specializes_query_expression;

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

template<typename T>
inline constexpr
bool
specializes_query_expression_v
= specializes_query_expression<T>::value;


}

#endif // HEIM_QUERY_EXPRESSION_HPP