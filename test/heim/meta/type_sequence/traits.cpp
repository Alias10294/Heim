#include "support.hpp"
using namespace heim_meta_test;

static_assert(heim::type_sequence_size_v<empty> == 0);
static_assert(heim::type_sequence_size_v<single> == 1);
static_assert(heim::type_sequence_size_v<triple> == 3);
static_assert(heim::type_sequence_size_v<repeated> == 6);
static_assert(heim::type_sequence_empty_v<empty>);
static_assert(!heim::type_sequence_empty_v<single>);
static_assert(!heim::type_sequence_empty_v<repeated>);
static_assert(heim::type_sequence_is_unique_v<empty>);
static_assert(heim::type_sequence_is_unique_v<single>);
static_assert(heim::type_sequence_is_unique_v<triple>);
static_assert(!heim::type_sequence_is_unique_v<repeated>);
static_assert(heim::type_sequence_is_unique_v<seq<a, const a, a&, a&&>>);
static_assert(!heim::type_sequence_is_unique_v<seq<a, a>>);

static_assert(heim::type_sequence_index_v<empty, a> == 0);
static_assert(heim::type_sequence_index_v<single, a> == 0);
static_assert(heim::type_sequence_index_v<single, b> == 1);
static_assert(heim::type_sequence_index_v<triple, a> == 0);
static_assert(heim::type_sequence_index_v<triple, b> == 1);
static_assert(heim::type_sequence_index_v<triple, c> == 2);
static_assert(heim::type_sequence_index_v<triple, d> == 3);
static_assert(heim::type_sequence_index_v<repeated, a> == 0);
static_assert(heim::type_sequence_index_v<repeated, b> == 1);
static_assert(heim::type_sequence_index_v<repeated, c> == 3);
static_assert(heim::type_sequence_index_v<repeated, d> == 6);

static_assert(heim::type_sequence_count_v<empty, a> == 0);
static_assert(heim::type_sequence_count_v<single, a> == 1);
static_assert(heim::type_sequence_count_v<repeated, a> == 3);
static_assert(heim::type_sequence_count_v<repeated, b> == 2);
static_assert(heim::type_sequence_count_v<repeated, c> == 1);
static_assert(heim::type_sequence_count_v<repeated, d> == 0);
static_assert(heim::type_sequence_count_v<seq<a, const a, a&, a&&>, a> == 1);
static_assert(heim::type_sequence_contains_v<repeated, a>);
static_assert(heim::type_sequence_contains_v<repeated, c>);
static_assert(!heim::type_sequence_contains_v<repeated, d>);
static_assert(!heim::type_sequence_contains_v<empty, a>);
static_assert(!heim::type_sequence_contains_v<seq<a>, const a>);

static_assert(empty::size == 0 && empty::empty && empty::is_unique);
static_assert(single::size == 1 && !single::empty && single::is_unique);
static_assert(repeated::size == 6 && !repeated::is_unique);
static_assert(repeated::index<a> == 0);
static_assert(repeated::index<c> == 3);
static_assert(repeated::index<d> == 6);
static_assert(repeated::count<a> == 3);
static_assert(repeated::count<d> == 0);
static_assert(repeated::contains<b>);
static_assert(!repeated::contains<d>);
