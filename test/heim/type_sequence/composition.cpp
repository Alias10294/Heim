#include "support.hpp"

using namespace heim_meta_test;

static_assert(same<heim::type_sequence_concat_t<empty>, empty>);
static_assert(same<heim::type_sequence_concat_t<triple>, triple>);
static_assert(same<heim::type_sequence_concat_t<empty, empty>, empty>);
static_assert(same<heim::type_sequence_concat_t<empty, single>, single>);
static_assert(same<heim::type_sequence_concat_t<single, empty>, single>);
static_assert(same<heim::type_sequence_concat_t<seq<a>, seq<b>, seq<c>>, triple>);
static_assert(same<heim::type_sequence_concat_t<seq<a, b>, seq<c, d>>, seq<a, b, c, d>>);
static_assert(same<heim::type_sequence_append_t<empty>, empty>);
static_assert(same<heim::type_sequence_append_t<seq<a>>, single>);
static_assert(same<heim::type_sequence_append_t<seq<a>, b, c>, triple>);
static_assert(same<heim::type_sequence_prepend_t<empty>, empty>);
static_assert(same<heim::type_sequence_prepend_t<seq<c>, a, b>, triple>);
static_assert(same<triple::append<d, e>, seq<a, b, c, d, e>>);
static_assert(same<triple::prepend<d, e>, seq<d, e, a, b, c>>);
static_assert(same<triple::concat<seq<d, e>>, seq<a, b, c, d, e>>);

static_assert(same<heim::type_sequence_join_t<empty>, empty>);
static_assert(same<heim::type_sequence_join_t<seq<a>>, seq<a>>);
static_assert(same<heim::type_sequence_join_t<seq<seq<a, b>, c>>, triple>);
static_assert(same<heim::type_sequence_join_t<seq<seq<a>, seq<>, seq<b, c>>>, triple>);
static_assert(same<heim::type_sequence_join_t<seq<seq<seq<a>>, b>>, seq<seq<a>, b>>);
static_assert(same<heim::type_sequence_join_t<seq<seq<>, seq<>>>, empty>);
static_assert(same<seq<seq<a, b>, c>::join, triple>);

static_assert(same<heim::type_sequence_reverse_t<empty>, empty>);
static_assert(same<heim::type_sequence_reverse_t<single>, single>);
static_assert(same<heim::type_sequence_reverse_t<triple>, seq<c, b, a>>);
static_assert(same<heim::type_sequence_reverse_t<seq<a, b, a, c>>, seq<c, a, b, a>>);
static_assert(same<triple::reverse, seq<c, b, a>>);
static_assert(same<triple::reverse::reverse, triple>);
