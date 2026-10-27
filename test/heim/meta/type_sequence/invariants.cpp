#include "support.hpp"
using namespace heim_meta_test;

template<class S>
consteval bool invariant_reverse_twice()
{
  return same<heim::type_sequence_reverse_t<heim::type_sequence_reverse_t<S>>, S>;
}

template<class S, std::size_t N>
consteval bool invariant_take_drop()
{
  return same<heim::type_sequence_concat_t<heim::type_sequence_take_t<S, N>, heim::type_sequence_drop_t<S, N>>, S>;
}

template<class S>
consteval bool invariant_unique_idempotent()
{
  using deduplicated = heim::type_sequence_unique_t<S>;
  return same<heim::type_sequence_unique_t<deduplicated>, deduplicated>;
}

template<class S>
consteval bool invariant_transform_identity()
{
  return same<heim::type_sequence_transform_t<S, std::type_identity>, S>;
}

template<class S>
consteval bool invariant_filter_partition()
{
  using kept = heim::type_sequence_filter_t<S, is_pointer>;
  using removed = heim::type_sequence_remove_if_t<S, is_pointer>;
  return heim::type_sequence_size_v<kept> + heim::type_sequence_size_v<removed> == heim::type_sequence_size_v<S>;
}

static_assert(invariant_reverse_twice<empty>());
static_assert(invariant_reverse_twice<single>());
static_assert(invariant_reverse_twice<triple>());
static_assert(invariant_reverse_twice<repeated>());
static_assert(invariant_reverse_twice<seq<int&, int&&, const int>>() );
static_assert(invariant_take_drop<empty, 0>());
static_assert(invariant_take_drop<empty, 20>());
static_assert(invariant_take_drop<triple, 0>());
static_assert(invariant_take_drop<triple, 1>());
static_assert(invariant_take_drop<triple, 2>());
static_assert(invariant_take_drop<triple, 3>());
static_assert(invariant_take_drop<triple, 20>());
static_assert(invariant_take_drop<repeated, 4>());
static_assert(invariant_unique_idempotent<empty>());
static_assert(invariant_unique_idempotent<triple>());
static_assert(invariant_unique_idempotent<repeated>());
static_assert(invariant_transform_identity<empty>());
static_assert(invariant_transform_identity<triple>());
static_assert(invariant_transform_identity<repeated>());
static_assert(invariant_filter_partition<empty>());
static_assert(invariant_filter_partition<seq<int*, int, double*, const char, float*>>() );

static_assert(same<heim::type_sequence_swap_t<heim::type_sequence_swap_t<triple, 0, 2>, 0, 2>, triple>);
static_assert(same<heim::type_sequence_sort_t<heim::type_sequence_sort_t<seq<number<3>, number<1>, number<2>>, less_number>, less_number>, seq<number<1>, number<2>, number<3>>>);
static_assert(same<heim::type_sequence_join_t<heim::type_sequence_chunk_t<triple, 2>>, triple>);
static_assert(same<heim::type_sequence_join_t<heim::type_sequence_chunk_t<repeated, 4>>, repeated>);
static_assert(same<heim::type_sequence_filter_t<seq<int, int*, char*, char>, is_pointer>, seq<int*, char*>>);
static_assert(same<heim::type_sequence_remove_if_t<seq<int, int*, char*, char>, is_pointer>, seq<int, char>>);
static_assert(heim::type_sequence_size_v<heim::type_sequence_cartesian_product_t<seq<a, b, c>, seq<d, e>>> == 6);
static_assert(heim::type_sequence_size_v<heim::type_sequence_zip_t<seq<a, b, c>, seq<d, e>>> == 2);
