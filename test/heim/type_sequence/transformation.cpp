#include "support.hpp"

using namespace heim_meta_test;

static_assert(same<heim::type_sequence_transform_t<empty, add_pointer>, empty>);
static_assert(same<heim::type_sequence_transform_t<seq<a, b>, add_pointer>, seq<a*, b*>>);
static_assert(same<heim::type_sequence_transform_t<seq<int, int>, std::add_const>, seq<const int,
    const int>>);
static_assert(same<seq<a, b>::transform<add_pointer>, seq<a*, b*>>);

static_assert(same<heim::type_sequence_filter_t<empty, is_pointer>, empty>);
static_assert(same<heim::type_sequence_filter_t<seq<a, a*, b*, const b, c*>, is_pointer>, seq<a*, b*, c*>>);
static_assert(same<heim::type_sequence_filter_t<triple, always_true>, triple>);
static_assert(same<heim::type_sequence_filter_t<triple, always_false>, empty>);
static_assert(same<seq<a, a*, b*>::filter<is_pointer>, seq<a*, b*>>);

static_assert(same<heim::type_sequence_remove_if_t<empty, is_pointer>, empty>);
static_assert(same<heim::type_sequence_remove_if_t<seq<a, a*, b*, const b, c*>, is_pointer>, seq<a,
    const b>>);
static_assert(same<heim::type_sequence_remove_if_t<triple, always_true>, empty>);
static_assert(same<heim::type_sequence_remove_if_t<triple, always_false>, triple>);
static_assert(same<seq<a, a*, b*>::remove_if<is_pointer>, seq<a>>);

static_assert(same<heim::type_sequence_remove_t<empty, a>, empty>);
static_assert(same<heim::type_sequence_remove_t<repeated, a>, seq<b, c, b>>);
static_assert(same<heim::type_sequence_remove_t<seq<a, const a, a&>, a>, seq<const a, a&>>);
static_assert(same<heim::type_sequence_remove_t<triple, d>, triple>);
static_assert(same<repeated::remove<b>, seq<a, a, c, a>>);

static_assert(same<heim::type_sequence_fill_t<empty, c>, empty>);
static_assert(same<heim::type_sequence_fill_t<triple, d>, seq<d, d, d>>);
static_assert(same<heim::type_sequence_fill_t<repeated, a>, seq<a, a, a, a, a, a>>);
static_assert(same<triple::fill<e>, seq<e, e, e>>);

static_assert(same<heim::type_sequence_unique_t<empty>, empty>);
static_assert(same<heim::type_sequence_unique_t<single>, single>);
static_assert(same<heim::type_sequence_unique_t<triple>, triple>);
static_assert(same<heim::type_sequence_unique_t<repeated>, seq<a, b, c>>);
static_assert(same<heim::type_sequence_unique_t<seq<const a, a, const a, a>>, seq<const a, a>>);
static_assert(same<repeated::unique, seq<a, b, c>>);
