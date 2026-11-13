#include "support.hpp"

using namespace heim_meta_test;

using partition_source = seq<a, separator, b, c, separator, d>;
static_assert(same<heim::type_sequence_split_t<empty, separator>, seq<empty>>);
static_assert(same<heim::type_sequence_split_t<triple, separator>, seq<triple>>);
static_assert(same<heim::type_sequence_split_t<partition_source, separator>, seq<seq<a>, seq<b, c>,
    seq<d>>>);
static_assert(same<heim::type_sequence_split_t<seq<separator, a>, separator>, seq<empty, seq<a>>>);
static_assert(same<heim::type_sequence_split_t<seq<a, separator>, separator>, seq<seq<a>, empty>>);
static_assert(same<heim::type_sequence_split_t<seq<a, separator, separator, b>, separator>, seq<seq<a>,
    empty, seq<b>>>);
static_assert(same<triple::split<a>, seq<empty, seq<b, c>>>);

static_assert(same<heim::type_sequence_chunk_t<empty, 1>, empty>);
static_assert(same<heim::type_sequence_chunk_t<triple, 1>, seq<seq<a>, seq<b>, seq<c>>>);
static_assert(same<heim::type_sequence_chunk_t<triple, 2>, seq<seq<a, b>, seq<c>>>);
static_assert(same<heim::type_sequence_chunk_t<triple, 3>, seq<triple>>);
static_assert(same<heim::type_sequence_chunk_t<triple, 8>, seq<triple>>);
static_assert(same<heim::type_sequence_chunk_t<seq<a, b, c, d, e>, 2>, seq<seq<a, b>, seq<c, d>, seq<e>>>);
static_assert(!has_chunk<triple, 0>);
static_assert(same<triple::chunk<2>, seq<seq<a, b>, seq<c>>>);

static_assert(same<heim::type_sequence_stride_t<empty, 1>, empty>);
static_assert(same<heim::type_sequence_stride_t<triple, 1>, triple>);
static_assert(same<heim::type_sequence_stride_t<triple, 2>, seq<a, c>>);
static_assert(same<heim::type_sequence_stride_t<triple, 3>, seq<a>>);
static_assert(same<heim::type_sequence_stride_t<triple, 4>, seq<a>>);
static_assert(same<heim::type_sequence_stride_t<seq<a, b, c, d, e>, 2>, seq<a, c, e>>);
static_assert(same<triple::stride<2>, seq<a, c>>);

static_assert(same<heim::type_sequence_elements_t<empty, 0>, empty>);
static_assert(same<heim::type_sequence_elements_t<seq<seq<a, b>, seq<c, d>>, 0>, seq<a, c>>);
static_assert(same<heim::type_sequence_elements_t<seq<seq<a, b>, seq<c, d>>, 1>, seq<b, d>>);
static_assert(same<heim::type_sequence_elements_t<seq<seq<a>, seq<b, c>>, 1>, seq<c>>);
static_assert(same<seq<seq<a>, seq<b, c>>::elements<0>, seq<a, b>>);
