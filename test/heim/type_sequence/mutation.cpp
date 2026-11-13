#include "support.hpp"

using namespace heim_meta_test;

static_assert(same<heim::type_sequence_insert_t<empty, 0, a>, seq<a>>);
static_assert(same<heim::type_sequence_insert_t<triple, 0, d>, seq<d, a, b, c>>);
static_assert(same<heim::type_sequence_insert_t<triple, 1, d, e>, seq<a, d, e, b, c>>);
static_assert(same<heim::type_sequence_insert_t<triple, 3, d>, seq<a, b, c, d>>);
static_assert(same<heim::type_sequence_insert_t<triple, 2>, triple>);
static_assert(same<triple::insert<2, d>, seq<a, b, d, c>>);

static_assert(same<heim::type_sequence_replace_t<triple, 0, d>, seq<d, b, c>>);
static_assert(same<heim::type_sequence_replace_t<triple, 1, d, e>, seq<a, d, e, c>>);
static_assert(same<heim::type_sequence_replace_t<triple, 2>, seq<a, b>>);
static_assert(same<heim::type_sequence_replace_t<triple, 2, d>, seq<a, b, d>>);
static_assert(same<triple::replace<1, e>, seq<a, e, c>>);

static_assert(same<heim::type_sequence_erase_t<empty>, empty>);
static_assert(same<heim::type_sequence_erase_t<triple>, triple>);
static_assert(same<heim::type_sequence_erase_t<triple, 0>, seq<b, c>>);
static_assert(same<heim::type_sequence_erase_t<triple, 1>, seq<a, c>>);
static_assert(same<heim::type_sequence_erase_t<triple, 2>, seq<a, b>>);
static_assert(same<heim::type_sequence_erase_t<triple, 0, 2>, seq<b>>);
static_assert(same<heim::type_sequence_erase_t<triple, 1, 1>, seq<a, c>>);
static_assert(same<heim::type_sequence_erase_t<triple, 9>, triple>);
static_assert(same<triple::erase<0, 1, 2>, empty>);

static_assert(same<heim::type_sequence_swap_t<triple, 0, 2>, seq<c, b, a>>);
static_assert(same<heim::type_sequence_swap_t<triple, 1, 1>, triple>);
static_assert(same<triple::swap<0, 1>, seq<b, a, c>>);

static_assert(same<heim::type_sequence_pop_front_t<triple>, seq<b, c>>);
static_assert(same<heim::type_sequence_pop_back_t<triple>, seq<a, b>>);
static_assert(same<heim::type_sequence_pop_front_t<single>, empty>);
static_assert(same<heim::type_sequence_pop_back_t<single>, empty>);
static_assert(same<triple::pop_front, seq<b, c>>);
static_assert(same<triple::pop_back, seq<a, b>>);
