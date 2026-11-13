#include "support.hpp"

using namespace heim_meta_test;

// These verify member aliases themselves, not merely the free *_t metafunctions.
// In particular, mutating type_sequence<>::reverse must fail this TU.
static_assert(same<empty::reverse, empty>);
static_assert(same<empty::unique, empty>);
static_assert(same<empty::join, empty>);
static_assert(same<empty::tuple, std::tuple<>>);
static_assert(same<empty::take<3>, empty>);
static_assert(same<empty::drop<3>, empty>);
static_assert(same<empty::elements<1>, empty>);
static_assert(same<empty::chunk<1>, empty>);
static_assert(same<empty::stride<2>, empty>);
static_assert(same<empty::select<>, empty>);
static_assert(same<empty::erase<0>, empty>);
static_assert(same<empty::slice<0, 2>, empty>);
static_assert(same<empty::concat<seq<a>>, seq<a>>);
static_assert(same<empty::append<a, b>, seq<a, b>>);
static_assert(same<empty::prepend<a, b>, seq<a, b>>);
static_assert(same<empty::insert<0, a>, seq<a>>);
static_assert(same<empty::transform<add_pointer>, empty>);
static_assert(same<empty::filter<always_true>, empty>);
static_assert(same<empty::remove_if<always_true>, empty>);
static_assert(same<empty::remove<a>, empty>);
static_assert(same<empty::fill<a>, empty>);
static_assert(same<empty::split<separator>, seq<empty>>);
static_assert(same<empty::zip<seq<a, b>>, empty>);
static_assert(same<empty::cartesian_product<seq<a>>, empty>);
static_assert(same<empty::sort<less_number>, empty>);
static_assert(same<empty::fold_left<d, make_pair_sequence>, d>);
static_assert(same<empty::fold_right<d, make_pair_sequence>, d>);
static_assert(empty::count<a> == 0);
static_assert(empty::index<a> == 0);
static_assert(!empty::contains<a>);

static_assert(same<single::front, a>);
static_assert(same<single::back, a>);
static_assert(same<single::reverse, single>);
static_assert(same<single::unique, single>);
static_assert(same<single::pop_front, empty>);
static_assert(same<single::pop_back, empty>);
static_assert(same<single::tuple, std::tuple<a>>);
static_assert(same<single::get<0>, a>);
static_assert(same<single::take<10>, single>);
static_assert(same<single::drop<10>, empty>);
static_assert(same<single::chunk<3>, seq<single>>);
static_assert(same<single::stride<4>, single>);
static_assert(same<single::select<0, 0>, seq<a, a>>);
static_assert(same<single::erase<0>, empty>);
static_assert(same<single::swap<0, 0>, single>);
static_assert(same<single::slice<0, 1>, single>);
static_assert(same<single::insert<1, b>, seq<a, b>>);
static_assert(same<single::replace<0, b, c>, seq<b, c>>);
static_assert(same<single::filter<always_false>, empty>);
static_assert(same<single::remove_if<always_true>, empty>);
static_assert(same<single::transform<add_pointer>, seq<a*>>);
static_assert(same<single::fill<b>, seq<b>>);
static_assert(same<single::split<a>, seq<empty, empty>>);
static_assert(same<single::zip<seq<b>>, seq<seq<a, b>>>);
static_assert(same<single::cartesian_product<seq<b, c>>, seq<seq<a, b>, seq<a, c>>>);
static_assert(same<single::fold_left_first<make_pair_sequence>, a>);
static_assert(same<single::fold_right_last<make_pair_sequence>, a>);
static_assert(single::index<a> == 0);
static_assert(single::index<b> == 1);
static_assert(single::count<a> == 1);
static_assert(single::contains<a>);

static_assert(same<triple::front, a>);
static_assert(same<triple::back, c>);
static_assert(same<triple::get<0>, a>);
static_assert(same<triple::get<2>, c>);
static_assert(same<triple::reverse, seq<c, b, a>>);
static_assert(same<triple::unique, triple>);
static_assert(same<triple::pop_front, seq<b, c>>);
static_assert(same<triple::pop_back, seq<a, b>>);
static_assert(same<triple::tuple, std::tuple<a, b, c>>);
static_assert(same<triple::take<2>, seq<a, b>>);
static_assert(same<triple::drop<2>, seq<c>>);
static_assert(same<triple::chunk<2>, seq<seq<a, b>, seq<c>>>);
static_assert(same<triple::stride<2>, seq<a, c>>);
static_assert(same<triple::select<2, 0, 2>, seq<c, a, c>>);
static_assert(same<triple::erase<1>, seq<a, c>>);
static_assert(same<triple::slice<1, 3>, seq<b, c>>);
static_assert(same<triple::swap<0, 2>, seq<c, b, a>>);
static_assert(same<triple::insert<1, d, e>, seq<a, d, e, b, c>>);
static_assert(same<triple::replace<1, d, e>, seq<a, d, e, c>>);
static_assert(same<triple::concat<seq<d>>, seq<a, b, c, d>>);
static_assert(same<triple::append<d>, seq<a, b, c, d>>);
static_assert(same<triple::prepend<d>, seq<d, a, b, c>>);
static_assert(same<triple::remove<b>, seq<a, c>>);
static_assert(same<triple::filter<always_true>, triple>);
static_assert(same<triple::remove_if<always_true>, empty>);
static_assert(same<triple::fill<d>, seq<d, d, d>>);
static_assert(same<triple::transform<add_pointer>, seq<a*, b*, c*>>);
static_assert(same<triple::split<b>, seq<seq<a>, seq<c>>>);
static_assert(same<triple::zip<seq<d, e>>, seq<seq<a, d>, seq<b, e>>>);
static_assert(same<triple::fold_left_first<make_pair_sequence>, seq<seq<a, b>, c>>);
static_assert(same<triple::fold_right_last<make_pair_sequence>, seq<a, seq<b, c>>>);
static_assert(triple::count<b> == 1);
static_assert(triple::contains<c>);
static_assert(triple::index<e> == 3);

using two_numbers = seq<number<3>, number<1>>;
static_assert(same<two_numbers::sort<less_number>, seq<number<1>, number<3>>>);
static_assert(same<two_numbers::sort<greater_number>, two_numbers>);
static_assert(same<repeated::unique, seq<a, b, c>>);
static_assert(!repeated::is_unique);

template<class S>
concept has_member_front = requires { typename S::front; };
template<class S>
concept has_member_back = requires { typename S::back; };
template<class S>
concept has_member_pop_front = requires { typename S::pop_front; };
template<class S>
concept has_member_pop_back = requires { typename S::pop_back; };
template<class S>
concept has_member_fold_left_first = requires { typename S::template fold_left_first<make_pair_sequence>; };
template<class S>
concept has_member_fold_right_last = requires { typename S::template fold_right_last<make_pair_sequence>; };

static_assert(!has_member_front<empty>);
static_assert(!has_member_back<empty>);
static_assert(!has_member_pop_front<empty>);
static_assert(!has_member_pop_back<empty>);
static_assert(!has_member_fold_left_first<empty>);
static_assert(!has_member_fold_right_last<empty>);
static_assert(has_member_front<single>);
static_assert(has_member_back<single>);
static_assert(has_member_pop_front<single>);
static_assert(has_member_fold_left_first<single>);
