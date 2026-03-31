#ifndef HEIM_NEW_QUERY_EXPRESSION_HPP
#define HEIM_NEW_QUERY_EXPRESSION_HPP

#include "component.hpp"
#include "identifier.hpp"
#include "type_sequence.hpp"

namespace heim::expression
{
//! @brief A tag representing a conjunction query expression.
struct conjunction_tag
{ };

//! @brief A tag representing a disjunction query expression.
struct disjunction_tag
{ };

//! @brief A tag representing a negated query expression.
struct negation_tag
{ };

template<typename ...Expressions> using conjunction = type_sequence<conjunction_tag, Expressions ...>;
template<typename ...Expressions> using disjunction = type_sequence<disjunction_tag, Expressions ...>;

template<typename Expression>
using negation
= type_sequence<negation_tag, Expression>;


/*!
 * @brief Determines the sequence of guaranteed component types present in a query of the given expression.
 *
 * @tparam Expression The expression type.
 */
template<typename Expression>
struct guaranteed;

template<typename Component>
struct guaranteed
{
  using type
  = type_sequence<Component>;
};

template<typename ...Expressions>
struct guaranteed<conjunction<Expressions ...>>
{
  using type
  = type_sequence_unite_t<typename guaranteed<Expressions>::type ...>::unique;
};

template<typename ...Expressions>
struct guaranteed<disjunction<Expressions ...>>
{
  using type
  = type_sequence_intersect_t<typename guaranteed<Expressions>::type ...>::unique;
};

template<typename Expression>
struct guaranteed<negation<Expression>>
{
  using type
  = type_sequence<>;
};

template<typename Expression>
struct guaranteed<negation<negation<Expression>>>
{
  using type
  = typename guaranteed<Expression>::type;
};

template<typename Expression>
using guaranteed_t
= typename guaranteed<Expression>::type;


/*!
 * @brief Determines the sequence of mentioned component types in the given expression.
 *
 * @tparam Expression The expression type.
 */
template<typename Expression>
struct mentioned;

template<typename Component>
struct mentioned
{
  using type
  = type_sequence<Component>;
};

template<typename ...Expressions>
struct mentioned<conjunction<Expressions ...>>
{
  using type
  = type_sequence<typename mentioned<Expressions>::type ...>
      ::flatten
      ::unique;
};

template<typename ...Expressions>
struct mentioned<disjunction<Expressions ...>>
{
  using type
  = type_sequence<typename mentioned<Expressions>::type ...>
      ::flatten
      ::unique;
};

template<typename Expression>
struct mentioned<negation<Expression>>
{
  using type
  = typename mentioned<Expression>::type;
};

template<typename Expression>
using mentioned_t
= typename mentioned<Expression>::type;


}

namespace heim
{
template<
    typename    Expression,
    typename ...Components>
struct query_descriptor;

template<
    typename    Expression,
    typename ...Components>
struct query_descriptor
{
public:
  using expression_type    = Expression;
  using component_sequence = type_sequence<Components ...>;

  static_assert(
      component_sequence::template is_equal<typename component_sequence::template map<std::remove_reference>>,
      "heim::query_descriptor: all component types must either be unqualified or const-qualified.");
  static_assert(
      component_sequence
          ::template map<std::remove_const>
          ::is_unique,
      "heim::query_descriptor: all component types must be different from each other.");
  static_assert(
      component_sequence
          ::template map       <std::remove_const>
          ::template difference<expression::guaranteed_t<expression_type>>
          ::empty,
      "heim::query_descriptor: all component types must be guaranteed present by expression_type.");
  static_assert(
      component_sequence
          ::template filter<component_tag_value>
          ::empty,
      "heim::query_descriptor: all component types must have a tag value of false.");

private:
  template<typename Identifier>
  struct meta
  {
    using identifier_type
    = Identifier;

    static_assert(
        specializes_identifier_v<identifier_type>,
        "heim::query_descriptor::meta: identifier_type must be a specialization of identifier.");

    using value_type
    = typename type_sequence<identifier_type>
        ::template append<std::remove_const_t<Components> ...>
        ::tuple;

    using reference
    = typename type_sequence<identifier_type const &>
        ::template append<std::add_lvalue_reference_t<Components> ...>
        ::tuple;

    using const_reference
    = typename type_sequence<identifier_type const &>
        ::template append<std::add_lvalue_reference_t<std::add_const_t<Components>> ...>
        ::tuple;
  };

public:
  template<typename Identifier> using value_type      = typename meta<Identifier>::value_type;
  template<typename Identifier> using reference       = typename meta<Identifier>::reference;
  template<typename Identifier> using const_reference = typename meta<Identifier>::const_reference;
};


template<typename T>
struct specializes_query_descriptor;

template<typename T>
struct specializes_query_descriptor
  : bool_constant<false>
{ };

template<
    typename    Expression,
    typename ...Components>
struct specializes_query_descriptor<
    query_descriptor<Expression, Components ...>>
  : bool_constant<true>
{ };

template<typename T>
inline constexpr
bool
specializes_query_descriptor_v
= specializes_query_descriptor<T>::value;


}

#endif // HEIM_NEW_QUERY_EXPRESSION_HPP