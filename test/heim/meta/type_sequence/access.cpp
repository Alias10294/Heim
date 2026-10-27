#include "support.hpp"
using namespace heim_meta_test;

static_assert(same<heim::type_sequence_front_t<single>, a>);
static_assert(same<heim::type_sequence_front_t<triple>, a>);
static_assert(same<heim::type_sequence_back_t<single>, a>);
static_assert(same<heim::type_sequence_back_t<triple>, c>);
static_assert(!has_front<empty>);
static_assert(!has_back<empty>);
static_assert(has_get<triple, 0>);
static_assert(has_get<triple, 2>);
static_assert(!has_get<triple, 3>);
static_assert(!has_get<empty, 0>);
static_assert(same<heim::type_sequence_get_t<triple, 0>, a>);
static_assert(same<heim::type_sequence_get_t<triple, 1>, b>);
static_assert(same<heim::type_sequence_get_t<triple, 2>, c>);
static_assert(same<heim::type_sequence_get_t<seq<a, const a, a&, a&&>, 1>, const a>);
static_assert(same<heim::type_sequence_get_t<seq<a, const a, a&, a&&>, 2>, a&>);
static_assert(same<triple::front, a>);
static_assert(same<triple::back, c>);
static_assert(same<triple::get<1>, b>);

static_assert(same<heim::type_sequence_take_t<empty, 0>, empty>);
static_assert(same<heim::type_sequence_take_t<empty, 5>, empty>);
static_assert(same<heim::type_sequence_take_t<triple, 0>, empty>);
static_assert(same<heim::type_sequence_take_t<triple, 1>, seq<a>>);
static_assert(same<heim::type_sequence_take_t<triple, 2>, seq<a, b>>);
static_assert(same<heim::type_sequence_take_t<triple, 3>, triple>);
static_assert(same<heim::type_sequence_take_t<triple, 7>, triple>);
static_assert(same<heim::type_sequence_drop_t<empty, 0>, empty>);
static_assert(same<heim::type_sequence_drop_t<empty, 5>, empty>);
static_assert(same<heim::type_sequence_drop_t<triple, 0>, triple>);
static_assert(same<heim::type_sequence_drop_t<triple, 1>, seq<b, c>>);
static_assert(same<heim::type_sequence_drop_t<triple, 2>, seq<c>>);
static_assert(same<heim::type_sequence_drop_t<triple, 3>, empty>);
static_assert(same<heim::type_sequence_drop_t<triple, 9>, empty>);
static_assert(same<triple::take<2>, seq<a, b>>);
static_assert(same<triple::drop<2>, seq<c>>);

static_assert(same<heim::type_sequence_select_t<triple>, empty>);
static_assert(same<heim::type_sequence_select_t<triple, 0>, seq<a>>);
static_assert(same<heim::type_sequence_select_t<triple, 2, 0>, seq<c, a>>);
static_assert(same<heim::type_sequence_select_t<triple, 1, 1, 1>, seq<b, b, b>>);
static_assert(same<triple::select<2, 1, 0>, seq<c, b, a>>);
static_assert(same<empty::select<>, empty>);

// slice<M, N> selects the half-open range [M, N), not [M, M + N).
static_assert(same<heim::type_sequence_slice_t<triple, 0, 3>, triple>);
static_assert(same<heim::type_sequence_slice_t<triple, 1, 3>, seq<b, c>>);
static_assert(same<heim::type_sequence_slice_t<triple, 1, 2>, seq<b>>);
static_assert(same<heim::type_sequence_slice_t<triple, 2, 2>, empty>);
static_assert(same<heim::type_sequence_slice_t<empty, 0, 0>, empty>);
static_assert(same<triple::slice<1, 3>, seq<b, c>>);
