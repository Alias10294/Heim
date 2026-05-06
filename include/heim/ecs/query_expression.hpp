#ifndef HEIM_ECS_QUERY_EXPRESSION_HPP
#define HEIM_ECS_QUERY_EXPRESSION_HPP

#include <type_traits>
#include "heim/lib/type_sequence.hpp"
#include "component.hpp"

namespace heim
{
/*!
 * \brief
 *   Determines whether the specializing type is a valid query expression.
 */
template<typename>
struct is_query_expression;

template<typename T>
inline constexpr
bool
is_query_expression_v
= is_query_expression<T>::value;

template<typename T>
concept query_expression
= is_query_expression_v<T>;


struct conjunction_tag
{ };

struct disjunction_tag
{ };

struct negation_tag
{ };


template<query_expression ...Expressions>
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

template<query_expression ...Expressions>
struct is_specialization_of_conjunction<
    conjunction<Expressions ...>>
  : std::true_type
{ };


template<query_expression ...Expressions>
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

template<query_expression ...Expressions>
struct is_specialization_of_disjunction<
    disjunction<Expressions ...>>
  : std::true_type
{ };


template<query_expression Expression>
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

template<query_expression Expression>
struct is_specialization_of_negation<
    negation<Expression>>
  : std::true_type
{ };


template<typename>
struct is_query_expression
  : std::false_type
{ };

template<component C>
struct is_query_expression<C>
  : std::true_type
{ };

template<query_expression ...Expressions>
struct is_query_expression<conjunction<Expressions ...>>
  : std::bool_constant<(is_query_expression_v<Expressions> && ...)>
{ };

template<query_expression ...Expressions>
struct is_query_expression<disjunction<Expressions ...>>
  : std::bool_constant<(is_query_expression_v<Expressions> && ...)>
{ };

template<query_expression Expression>
struct is_query_expression<negation<Expression>>
  : is_query_expression<Expression>
{ };


template<typename>
struct guaranteed;

template<query_expression E>
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


template<component C>
struct guaranteed<C>
  : std::type_identity<type_sequence<C>>
{ };

template<query_expression ...Expressions>
struct guaranteed<conjunction<Expressions ...>>
  : std::type_identity<typename type_sequence<guaranteed_t<Expressions> ...>::join::unique>
{ };

template<query_expression ...Expressions>
struct guaranteed<disjunction<Expressions ...>>
  : detail::guaranteed_disjunction<guaranteed_t<Expressions> ...>
{ };

template<query_expression Expression>
struct guaranteed<negation<Expression>>
  : std::type_identity<type_sequence<>>
{ };

template<query_expression Expression>
struct guaranteed<negation<negation<Expression>>>
  : guaranteed<Expression>
{ };

template<query_expression ...Expressions>
struct guaranteed<negation<conjunction<Expressions ...>>>
  : guaranteed<disjunction<negation<Expressions> ...>>
{ };

template<query_expression ...Expressions>
struct guaranteed<negation<disjunction<Expressions ...>>>
  : guaranteed<conjunction<negation<Expressions> ...>>
{ };


}

#endif // HEIM_ECS_QUERY_EXPRESSION_HPP
