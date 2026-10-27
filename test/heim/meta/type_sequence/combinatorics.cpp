#include "support.hpp"
using namespace heim_meta_test;

static_assert(same<heim::type_sequence_zip_t<>, empty>);
static_assert(same<heim::type_sequence_zip_t<empty>, empty>);
static_assert(same<heim::type_sequence_zip_t<triple>, seq<seq<a>, seq<b>, seq<c>>>);
static_assert(same<heim::type_sequence_zip_t<seq<a, b>, seq<c, d>>, seq<seq<a, c>, seq<b, d>>>);
static_assert(same<heim::type_sequence_zip_t<seq<a, b, c>, seq<d, e>>, seq<seq<a, d>, seq<b, e>>>);
static_assert(same<heim::type_sequence_zip_t<triple, empty>, empty>);
static_assert(same<heim::type_sequence_zip_t<seq<a, b>, seq<c, d>, seq<e, separator>>, seq<seq<a, c, e>, seq<b, d, separator>>>);
static_assert(same<triple::zip<seq<d, e>>, seq<seq<a, d>, seq<b, e>>>);

static_assert(same<heim::type_sequence_cartesian_product_t<>, seq<empty>>);
static_assert(same<heim::type_sequence_cartesian_product_t<empty>, empty>);
static_assert(same<heim::type_sequence_cartesian_product_t<seq<a, b>>, seq<seq<a>, seq<b>>>);
static_assert(same<heim::type_sequence_cartesian_product_t<seq<a, b>, seq<c, d>>, seq<seq<a, c>, seq<a, d>, seq<b, c>, seq<b, d>>>);
static_assert(same<heim::type_sequence_cartesian_product_t<empty, triple>, empty>);
static_assert(same<heim::type_sequence_cartesian_product_t<triple, empty>, empty>);
static_assert(same<heim::type_sequence_cartesian_product_t<seq<a>, seq<b>, seq<c>>, seq<seq<a, b, c>>>);
static_assert(same<heim::type_sequence_cartesian_product_t<seq<a, b>, seq<c>, seq<d, e>>, seq<seq<a, c, d>, seq<a, c, e>, seq<b, c, d>, seq<b, c, e>>>);
static_assert(same<triple::cartesian_product<seq<d>>, seq<seq<a, d>, seq<b, d>, seq<c, d>>>);

static_assert(same<heim::type_sequence_array_t<a, 0>, empty>);
static_assert(same<heim::type_sequence_array_t<a, 1>, seq<a>>);
static_assert(same<heim::type_sequence_array_t<a, 2>, seq<a, a>>);
static_assert(same<heim::type_sequence_array_t<a, 5>, seq<a, a, a, a, a>>);
static_assert(same<heim::type_sequence_array_t<const int&, 2>, seq<const int&, const int&>>);
static_assert(heim::type_sequence_size_v<heim::type_sequence_array_t<a, 32>> == 32);
static_assert(heim::type_sequence_is_unique_v<heim::type_sequence_array_t<a, 0>>);
static_assert(!heim::type_sequence_is_unique_v<heim::type_sequence_array_t<a, 4>>);
