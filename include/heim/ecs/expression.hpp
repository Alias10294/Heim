#ifndef HEIM_ECS_EXPRESSION_HPP
#define HEIM_ECS_EXPRESSION_HPP

#include <type_traits>
#include "heim/lib/type_sequence.hpp"
#include "component.hpp"

namespace heim
{
/*!
 * \brief
 *   Determines whether the specializing type is a valid expression type.
 *
 * \details
 *   An expression type, in the context of the entity-component-system (ECS) pattern, is a type describing
 *   an expression to which an entity can match to depending on what component(s) it possesses. \n
 *   Such expressions can either be: a singular component type; the conjunction or disjunction of multiple
 *   sub-expressions; or the negation of a sub-expression.
 */
template<typename>
struct is_expression;

template<typename T>
inline constexpr
bool
is_expression_v
= is_expression<T>::value;

template<typename T>
concept expression
= is_expression_v<T>;


struct conjunction_tag { };
struct disjunction_tag { };
struct negation_tag    { };

/*!
 * \brief
 *   An expression type describing a conjunction of sub-expressions.
 *
 * \details
 *   For an entity to match a conjunction of sub-expressions, it must match all the sub-expressions.
 */
template<typename ...Expressions>
requires (sizeof...(Expressions) > 0 && (expression<Expressions> && ...))
using conjunction
= type_sequence<conjunction_tag, Expressions ...>;

/*!
 * \brief
 *   Determines whether the specializing type is a specialization of conjunction.
 */
template<typename T>
struct is_specialization_of_conjunction;

template<typename T>
inline constexpr
bool
is_specialization_of_conjunction_v
= is_specialization_of_conjunction<T>::value;

template<typename T>
concept specialization_of_conjunction
= is_specialization_of_conjunction_v<T>;

template<typename>
struct is_specialization_of_conjunction
  : std::false_type
{ };

template<typename ...Expressions>
struct is_specialization_of_conjunction<
    conjunction<Expressions ...>>
  : std::true_type
{ };


/*!
 * \brief
 *   An expression type describing a disjunction of sub-expressions.
 *
 * \details
 *   For an entity to match a disjunction of sub-expressions, it must match at least one of the sub-expressions.
 */
template<typename ...Expressions>
requires (sizeof...(Expressions) > 0 && (expression<Expressions> && ...))
using disjunction
= type_sequence<disjunction_tag, Expressions ...>;

/*!
 * \brief
 *   Determines whether the specializing type is a specialization of disjunction.
 */
template<typename T>
struct is_specialization_of_disjunction;

template<typename T>
inline constexpr
bool
is_specialization_of_disjunction_v
= is_specialization_of_disjunction<T>::value;

template<typename T>
concept specialization_of_disjunction
= is_specialization_of_disjunction_v<T>;

template<typename>
struct is_specialization_of_disjunction
  : std::false_type
{ };

template<typename ...Expressions>
struct is_specialization_of_disjunction<
    disjunction<Expressions ...>>
  : std::true_type
{ };


/*!
 * \brief
 *   An expression type describing the negation of a sub-expression.
 *
 * \details
 *   For an entity to match the negation of a sub-expression, it must not match the sub-expression.
 */
template<typename Expression>
requires expression<Expression>
using negation
= type_sequence<negation_tag, Expression>;

/*!
 * \brief
 *   Determines whether the specializing type is a specialization of negation.
 */
template<typename T>
struct is_specialization_of_negation;

template<typename T>
inline constexpr
bool
is_specialization_of_negation_v
= is_specialization_of_negation<T>::value;

template<typename T>
concept specialization_of_negation
= is_specialization_of_negation_v<T>;

template<typename>
struct is_specialization_of_negation
  : std::false_type
{ };

template<typename Expression>
struct is_specialization_of_negation<
    negation<Expression>>
  : std::true_type
{ };


template<typename>
struct is_expression
  : std::false_type
{ };

template<typename C>
requires component<C>
struct is_expression<C>
  : std::true_type
{ };

template<typename ...Expressions>
struct is_expression<conjunction<Expressions ...>>
  : std::bool_constant<(is_expression_v<Expressions> && ...)>
{ };

template<typename ...Expressions>
struct is_expression<disjunction<Expressions ...>>
  : std::bool_constant<(is_expression_v<Expressions> && ...)>
{ };

template<typename Expression>
struct is_expression<negation<Expression>>
  : is_expression<Expression>
{ };


/*!
 * \brief
 *   Determines the sequence of component types guaranteed to be possessed by an entity matching
 *   the specializing expression type.
 */
template<typename>
struct guaranteed
{ };

template<typename E>
requires expression<E>
using guaranteed_t
= typename guaranteed<E>::type;

namespace detail
{
template<typename ...Guaranteed>
struct guaranteed_disjunction_meta
{
  template<typename T>
  struct predicate
    : std::bool_constant<(type_sequence_contains_v<Guaranteed, T> && ...)>
  { };
};


template<typename ...>
struct guaranteed_disjunction;

template<>
struct guaranteed_disjunction<>
  : std::type_identity<type_sequence<>>
{ };

template<typename Guaranteed>
struct guaranteed_disjunction<Guaranteed>
  : std::type_identity<Guaranteed>
{ };

template<typename First, typename ...Rest>
struct guaranteed_disjunction<First, Rest ...>
  : type_sequence_filter<
        typename First::unique,
        guaranteed_disjunction_meta<typename Rest::unique ...>::template predicate>
{ };

} // namespace detail


template<typename C>
requires component<C>
struct guaranteed<
    C>
  : std::type_identity<
        type_sequence<C>>
{ };

template<typename ...Expressions>
struct guaranteed<
    conjunction<Expressions ...>>
  : std::type_identity<
        typename type_sequence<guaranteed_t<Expressions> ...>::join::unique>
{ };

template<typename ...Expressions>
struct guaranteed<
    disjunction<Expressions ...>>
  : detail::guaranteed_disjunction<guaranteed_t<Expressions> ...>
{ };

template<typename Expression>
struct guaranteed<
    negation<Expression>>
  : std::type_identity<
        type_sequence<>>
{ };

template<typename Expression>
struct guaranteed<
    negation<negation<Expression>>>
  : guaranteed<Expression>
{ };

template<typename ...Expressions>
struct guaranteed<
    negation<conjunction<Expressions ...>>>
  : guaranteed<disjunction<negation<Expressions> ...>>
{ };

template<typename ...Expressions>
struct guaranteed<
    negation<disjunction<Expressions ...>>>
  : guaranteed<conjunction<negation<Expressions> ...>>
{ };

} // namespace heim

#endif // HEIM_ECS_EXPRESSION_HPP
