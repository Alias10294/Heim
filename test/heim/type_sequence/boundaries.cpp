#include "support.hpp"

using namespace heim_meta_test;

// Missing elements yield no ::type; they are not an error until instantiated.
static_assert(!has_front<empty>);
static_assert(!has_back<empty>);
static_assert(!has_get<empty, 0>);
static_assert(!has_get<triple, 3>);
static_assert(!has_get<triple, 100>);
static_assert(has_get<triple, 2>);
static_assert(has_take<empty, 0>);
static_assert(has_take<empty, 100>);
static_assert(has_drop<empty, 0>);
static_assert(has_drop<empty, 100>);
static_assert(!has_chunk<single, 0>);
static_assert(has_chunk<empty, 1>);
static_assert(has_chunk<triple, 1>);

// Empty and zero-argument operations have intentionally distinct semantics.
static_assert(same<heim::type_sequence_concat_t<empty>, empty>);
static_assert(same<heim::type_sequence_zip_t<>, empty>);
static_assert(same<heim::type_sequence_cartesian_product_t<>, seq<empty>>);
static_assert(same<heim::type_sequence_cartesian_product_t<empty>, empty>);
static_assert(same<heim::type_sequence_split_t<empty, separator>, seq<empty>>);
static_assert(same<heim::type_sequence_array_t<a, 0>, empty>);

// A type is an identity, independent of value category or cv-qualification.
using qualified = seq<a, const a, volatile a, a&, const a&, a && , a*>;
static_assert(heim::type_sequence_size_v<qualified> == 7);
static_assert(heim::type_sequence_is_unique_v<qualified>);
static_assert(same<heim::type_sequence_unique_t<seq<a&, a && , a&, const a&>>, seq<a&, a && , const a&>>);
static_assert(same<heim::type_sequence_reverse_t<seq<a&, const a&, a && >>, seq<a && , const a&, a&>>);
static_assert(same<heim::type_sequence_filter_t<qualified, is_const>, seq<const a, const a&>> == false);
// std::is_const is false for reference types, including const a&.
static_assert(same<heim::type_sequence_filter_t<qualified, is_const>, seq<const a>>);

// Metafunctions must not require the referenced types to be complete.
static_assert(same<heim::type_sequence_reverse_t<seq<incomplete, incomplete*>>, seq<incomplete*,
    incomplete>>);
static_assert(same<heim::type_sequence_unique_t<seq<incomplete, incomplete>>, seq<incomplete>>);
