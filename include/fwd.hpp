#ifndef HEIM_FWD_HPP
#define HEIM_FWD_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>

namespace heim
{
/*!
 * @brief A type representing a constant value.
 *
 * @tparam T   The value type.
 * @tparam val The value.
 */
template<
    typename T,
    T        val>
using constant
= std::integral_constant<T, val>;

template<bool val>
using bool_constant
= constant<bool, val>;

template<std::size_t val>
using size_constant
= constant<std::size_t, val>;



/*!
 * @brief Determines the unsigned integral type which is composed of at least the given number of
 *   bits.
 *
 * @tparam Digits The number of bits.
 */
template<int Digits>
struct unsigned_integral_for;

template<int Digits>
using unsigned_integral_for_t
= typename unsigned_integral_for<Digits>::type;



/*!
 * @brief Determines the type with a conditionally added @c const qualifier.
 *
 * @tparam T       The type to determine for.
 * @tparam IsConst The condition value.
 */
template<
    typename T,
    bool     IsConst>
struct maybe_const;

template<
    typename T,
    bool     IsConst>
using maybe_const_t
= typename maybe_const<T, IsConst>::type;



/*!
 * @brief Determines if the given type is unqualified (i.e. neither @c const, @c volatile nor a
 *   reference).
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct is_unqualified;

template<typename T>
inline constexpr
bool
is_unqualified_v
= is_unqualified<T>::value;



/*!
 * @brief Determines if a type passes as an allocator type.
 *
 * @tparam T The type to determine for.
 *
 * @note This type only adheres to what the std::allocator_traits type demands of an allocator
 *   type. Proper complete allocator types should try to adhere to the allocator requirements
 *   formulated on cppreference.com.
 */
template<typename T>
struct is_an_allocator;

template<typename T>
inline constexpr
bool
is_an_allocator_v
= is_an_allocator<T>::value;



/*!
 * @brief Determines whether a type passes as an allocator for a certain value type.
 *
 * @tparam A The type to determine the allocator characteristics of.
 * @tparam T The value type to determine for.
 */
template<
    typename A,
    typename T>
struct is_an_allocator_for;

template<
    typename A,
    typename T>
inline constexpr
bool
is_an_allocator_for_v
= is_an_allocator_for<A, T>::value;



/*!
 * @brief A type representing a pack of types.
 *
 * @details Implements what is typically referred to as a type list.
 *   Includes all existing expressed type sequence traits.
 *
 * @tparam Ts The pack of types.
 */
template<typename ...Ts>
struct type_sequence;

/*!
 * @brief Determines the number of types in a given type sequence.
 *
 * @tparam TypeSequence The type sequence.
 */
template<typename TypeSequence>
struct type_sequence_size;

template<typename TypeSequence>
static constexpr
std::size_t
type_sequence_size_v
= type_sequence_size<TypeSequence>::value;

/*!
 * @brief Determines the number of occurrences of a given type in a type sequence.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam T            The type to determine the count of.
 */
template<
    typename TypeSequence,
    typename T>
struct type_sequence_count;

template<
    typename TypeSequence,
    typename T>
inline constexpr
std::size_t
type_sequence_count_v
= type_sequence_count<TypeSequence, T>::value;

/*!
 * @brief Determines whether the given type is present in the type sequence.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam T            The type to determine the presence of.
 */
template<
    typename TypeSequence,
    typename T>
struct type_sequence_contains;

template<
    typename TypeSequence,
    typename T>
inline constexpr
bool
type_sequence_contains_v
= type_sequence_contains<TypeSequence, T>::value;

/*!
 * @brief Determines the index of the given type in the type sequence.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam T            The type to get the index of.
 *
 * @warning This trait is ill-formed when the type is not present in the type sequence.
 */
template<
    typename TypeSequence,
    typename T>
struct type_sequence_index;

template<
    typename TypeSequence,
    typename T>
inline constexpr
std::size_t
type_sequence_index_v
= type_sequence_index<TypeSequence, T>::value;

/*!
 * @brief Determines the type located the given index in the type sequence.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam Index        The index value.
 */
template<
    typename    TypeSequence,
    std::size_t Index>
struct type_sequence_get;

template<
    typename TypeSequence,
    std::size_t Index>
using type_sequence_get_t
= typename type_sequence_get<TypeSequence, Index>::type;

/*!
 * @brief Determines the type sequence obtained by concatenating the two given type sequences.
 *
 * @tparam Left  The type sequence on the left.
 * @tparam Right The type sequence on the right.
 */
template<
    typename Left,
    typename Right>
struct type_sequence_concatenate;

template<
    typename Left,
    typename Right>
using type_sequence_concatenate_t
= typename type_sequence_concatenate<Left, Right>::type;

/*!
 * @brief Determines the type sequence with the given pack of types added to its end.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam Ts           The pack of types to append.
 */
template<
    typename    TypeSequence,
    typename ...Ts>
struct type_sequence_append;

template<
    typename    TypeSequence,
    typename ...Ts>
using type_sequence_append_t
= typename type_sequence_append<TypeSequence, Ts ...>::type;

/*!
 * @brief Determines the type sequence with the first occurrence of the given type erased from it.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam T            The type to erase.
 */
template<
    typename TypeSequence,
    typename T>
struct type_sequence_erase;

template<
    typename TypeSequence,
    typename T>
using type_sequence_erase_t
= typename type_sequence_erase<TypeSequence, T>::type;

/*!
 * @brief Determines the type sequence obtained by keeping the types which verifies the given predicate.
 *
 * @tparam TypeSequence The type sequence.
 * @tparam Pred         The predicate type.
 *
 * @note The predicate type must expose a static constexpr bool value attribute.
 */
template<
    typename                    TypeSequence,
    template<typename> typename Pred>
struct type_sequence_filter;

template<
    typename                    TypeSequence,
    template<typename> typename Pred>
using type_sequence_filter_t
= typename type_sequence_filter<TypeSequence, Pred>::type;

/*!
 * @brief Determines the type sequence obtained all duplicates of present types in the given sequence.
 *
 * @tparam TypeSequence The type sequence.
 *
 * @note The second template parameter exists as an implementation requirement and should not be specialized manually.
 */
template<
    typename TypeSequence,
    typename VisitedSequence = type_sequence<>>
struct type_sequence_unique;

template<typename TypeSequence>
using type_sequence_unique_t
= typename type_sequence_unique<TypeSequence>::type;

/*!
 * @brief Determines whether the given type sequence is unique (i.e. contains no duplicated types).
 *
 * @tparam TypeSequence The type sequence.
 */
template<typename TypeSequence>
struct type_sequence_is_unique;

template<typename TypeSequence>
inline constexpr
bool
type_sequence_is_unique_v
= type_sequence_is_unique<TypeSequence>::value;

/*!
 * @brief Determines the type sequence obtained by replacing in the type sequence
 *   all present type sequences by their pack of types.
 *
 * @tparam TypeSequence The type sequence.
 *
 * @note Only "top-level" type sequences are replaced by their packs.
 *   That is, if the given type sequence contains a type sequence which itself contains a type sequence,
 *   the latter will remain a type sequence.
 */
template<typename TypeSequence>
struct type_sequence_flatten;

template<typename TypeSequence>
using type_sequence_flatten_t
= typename type_sequence_flatten<TypeSequence>::type;

/*!
 * @brief Determines the type sequence obtained by keeping only the type occurrences
 *   present in the left type sequence and absent in the right type sequence.
 *
 * @tparam Left  The type sequence to the left of the subtraction.
 * @tparam Right The type sequence to the right of the subtraction.
 *
 * @note This trait models a typical list difference with multiplicity and stable order.
 */
template<
    typename Left,
    typename Right>
struct type_sequence_difference;

template<
    typename Left,
    typename Right>
using type_sequence_difference_t
= typename type_sequence_difference<Left, Right>::type;

/*!
 * @brief Determines the std::tuple type with the same of types as the given type sequence.
 *
 * @tparam TypeSequence The type sequence.
 */
template<typename TypeSequence>
struct type_sequence_tuple;

template<typename TypeSequence>
using type_sequence_tuple_t
= typename type_sequence_tuple<TypeSequence>::type;

/*!
 * @brief Determines whether the given type is a specialization of type_sequence.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct specializes_type_sequence;

template<typename T>
inline constexpr
bool
specializes_type_sequence_v
= specializes_type_sequence<T>::value;



/*!
 * @brief An implementation of the entity type as described by the generic_entity-component-system
 *   pattern.
 *
 * @details Encapsulates an unsigned integral value and divides it into an index and a generation.
 *   This generation mechanism allows us to recycle index values and monitor the lifetime of
 *   entities themselves.
 *
 * @tparam UInt        The underlying value type.
 * @tparam IndexDigits The number of bits allocated to represent the index value.
 *
 * @note All remaining bits thar are not used for the index are used for its generation.
 */
template<
    typename UInt        = std::uintmax_t,
    int      IndexDigits = std::numeric_limits<UInt>::digits / 2>
class entity;

/*!
 * @brief Determines whether the given type is a specialization of entity.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct specializes_entity;

template<typename T>
inline constexpr
bool
specializes_entity_v
= specializes_entity<T>::value;



/*!
 * @brief An associative container specializing in the management of entities in the
 *   entity-component-system pattern.
 *
 * @details Implements a customized sparse set, that is each entity's position in the container is
 *   kept tracked by a complementary array. This structure allows for constant-time insertion,
 *   removal and access to entities, whilst containing the entities in contiguous memory for
 *   optimal iteration.
 *   Moreover, internally the dense array of entities of partitioned in two groups of valid and
 *   invalid entities. The invalid entities sit at the front of the vector and the valid ones at
 *   the back to accommodate for emplaced newly-generated entities.
 *
 * @tparam Entity    The entity type.
 * @tparam Allocator The allocator type.
 */
template<
    typename Entity,
    typename Allocator>
class entity_manager;

/*!
 * @brief Determines whether the given type is a specialization of entity_manager.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct specializes_entity_manager;

template<typename T>
inline constexpr
bool
specializes_entity_manager_v
= specializes_entity_manager<T>::value;



namespace sparse_set_based
{
/*!
 * @brief An associative container optimized for its use in the entity-component-system
 *   pattern.
 *
 * @details Implements a customized sparse set, that is each entity's position in the container is
 *   kept tracked by a complementary array, which is by default paginated to avoid significant
 *   memory overhead in most use cases. This structure allows for constant-time insertion, removal
 *   and access to elements, whilst containing values in contiguous memory for optimal iteration.
 *   Also, because of the structure-of-array (SoA) nature of the container, iterators expose a pair
 *   of references rather than a reference to a pair.
 *   Empty types, or types that are only considered valuable for their presence alongside an entity
 *   can be marked as tags. This option makes it so that they are not stored at all in the
 *   container nor exposed by iterators, which can constitute a significant memory saving.
 *
 * @tparam Component The component type.
 * @tparam Entity    The entity type.
 * @tparam Allocator The allocator type.
 * @tparam PageSize  The size of the internal pages of tracked positions.
 * @tparam TagValue  The value by which the component type is considered a tag type or not.
 *
 * @note Specializing the page size to be 0 causes the position container to not be paginated at
 *   all. This can be considered a good option if inserted entities are expected to have low enough
 *   index values.
 */
template<
    typename    Component,
    typename    Entity    = entity<>,
    typename    Allocator = std::allocator<Entity>,
    std::size_t PageSize  = 1024,
    bool        TagValue  = std::is_empty_v<Component>>
class pool;

/*!
 * @brief Determines whether the given type is a specialization of pool.
 *
 * @tparam T The type to determine for.
 */
template<typename T>
struct specializes_pool;

template<typename T>
inline constexpr
bool
specializes_pool_v
= specializes_pool<T>::value;


} // namespace sparse_set_based
} // namespace heim

#endif // HEIM_FWD_HPP