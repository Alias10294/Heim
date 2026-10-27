#include "support.hpp"
using namespace heim_meta_test;

using unsorted = seq<number<3>, number<1>, number<4>, number<0>, number<2>>;
using ascending = seq<number<0>, number<1>, number<2>, number<3>, number<4>>;
using descending = seq<number<4>, number<3>, number<2>, number<1>, number<0>>;

static_assert(same<heim::type_sequence_sort_t<empty, less_number>, empty>);
static_assert(same<heim::type_sequence_sort_t<seq<number<7>>, less_number>, seq<number<7>>>);
static_assert(same<heim::type_sequence_sort_t<seq<number<2>, number<1>>, less_number>, seq<number<1>, number<2>>>);
static_assert(same<heim::type_sequence_sort_t<unsorted, less_number>, ascending>);
static_assert(same<heim::type_sequence_sort_t<unsorted, greater_number>, descending>);
static_assert(same<heim::type_sequence_sort_t<ascending, less_number>, ascending>);
static_assert(same<heim::type_sequence_sort_t<descending, greater_number>, descending>);
static_assert(same<heim::type_sequence_sort_t<seq<number<1>, number<1>, number<0>>, less_number>, seq<number<0>, number<1>, number<1>>>);
static_assert(same<unsorted::sort<less_number>, ascending>);

// Comparator-equivalent but nonidentical types must retain their input order.
struct first_one : number<1> {};
struct second_one : number<1> {};
struct third_one : number<1> {};
static_assert(same<heim::type_sequence_sort_t<seq<first_one, number<0>, second_one, number<2>, third_one>, less_number>, seq<number<0>, first_one, second_one, third_one, number<2>>>);
static_assert(same<heim::type_sequence_sort_t<seq<third_one, second_one, first_one>, less_number>, seq<third_one, second_one, first_one>>);
static_assert(same<heim::type_sequence_sort_t<seq<number<-3>, number<8>, number<0>, number<-1>>, less_number>, seq<number<-3>, number<-1>, number<0>, number<8>>>);
