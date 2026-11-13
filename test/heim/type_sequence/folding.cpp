#include "support.hpp"

using namespace heim_meta_test;

static_assert(same<heim::type_sequence_fold_left_t<empty, a, make_pair_sequence>, a>);
static_assert(same<heim::type_sequence_fold_right_t<empty, a, make_pair_sequence>, a>);
static_assert(same<heim::type_sequence_fold_left_t<single, b, make_pair_sequence>, seq<b, a>>);
static_assert(same<heim::type_sequence_fold_right_t<single, b, make_pair_sequence>, seq<a, b>>);
static_assert(same<heim::type_sequence_fold_left_t<triple, d, make_pair_sequence>, seq<seq<seq<d, a>, b>,
    c>>);
static_assert(same<heim::type_sequence_fold_right_t<triple, d, make_pair_sequence>, seq<a, seq<b, seq<c,
    d>>>>);
static_assert(same<heim::type_sequence_fold_left_first_t<triple, make_pair_sequence>, seq<seq<a, b>, c>>);
static_assert(same<heim::type_sequence_fold_right_last_t<triple, make_pair_sequence>, seq<a, seq<b, c>>>);
static_assert(same<heim::type_sequence_fold_left_first_t<single, make_pair_sequence>, a>);
static_assert(same<heim::type_sequence_fold_right_last_t<single, make_pair_sequence>, a>);
static_assert(same<triple::fold_left<d, make_pair_sequence>, seq<seq<seq<d, a>, b>, c>>);
static_assert(same<triple::fold_right<d, make_pair_sequence>, seq<a, seq<b, seq<c, d>>>>);
static_assert(same<triple::fold_left_first<make_pair_sequence>, seq<seq<a, b>, c>>);
static_assert(same<triple::fold_right_last<make_pair_sequence>, seq<a, seq<b, c>>>);

// The concatenation operation is associative; both folds give the same list.
using nested = seq<seq<a>, seq<b, c>, seq<d>>;
static_assert(same<heim::type_sequence_fold_left_t<nested, empty, sequence_cat>, seq<a, b, c, d>>);
static_assert(same<heim::type_sequence_fold_right_t<nested, empty, sequence_cat>, seq<a, b, c, d>>);
static_assert(same<heim::type_sequence_fold_left_first_t<nested, sequence_cat>, seq<a, b, c, d>>);
static_assert(same<heim::type_sequence_fold_right_last_t<nested, sequence_cat>, seq<a, b, c, d>>);
