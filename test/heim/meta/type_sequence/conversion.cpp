#include "support.hpp"

using namespace heim_meta_test;

static_assert(heim::is_specialization_of_type_sequence_v<empty>);
static_assert(heim::is_specialization_of_type_sequence_v<triple>);
static_assert(!heim::is_specialization_of_type_sequence_v<int>);
static_assert(!heim::is_specialization_of_type_sequence_v<std::tuple<a, b>>);
static_assert(!heim::is_specialization_of_type_sequence_v<const triple>);
static_assert(heim::specialization_of_type_sequence<empty>);
static_assert(heim::specialization_of_type_sequence<triple>);
static_assert(!heim::specialization_of_type_sequence<std::pair<a, b>>);

static_assert(same<heim::type_sequence_tuple_t<empty>, std::tuple<>>);
static_assert(same<heim::type_sequence_tuple_t<triple>, std::tuple<a, b, c>>);
static_assert(same<heim::type_sequence_tuple_t<seq<int&, const int, int*>>, std::tuple<int&, const int,
    int*>>);
static_assert(same<triple::tuple, std::tuple<a, b, c>>);
static_assert(same<empty::tuple, std::tuple<>>);
static_assert(same<heim::to_type_sequence_t<std::tuple<>>, empty>);
static_assert(same<heim::to_type_sequence_t<std::tuple<a, b, c>>, triple>);
static_assert(same<heim::to_type_sequence_t<std::tuple<a&, const b>>, seq<a&, const b>>);
static_assert(same<heim::to_type_sequence_t<std::pair<a, b>>, seq<a, b>>);
static_assert(same<heim::to_type_sequence_t<std::pair<const a, b&>>, seq<const a, b&>>);
static_assert(same<heim::to_type_sequence_t<std::array<int, 0>>, empty>);
static_assert(same<heim::to_type_sequence_t<std::array<int, 1>>, seq<int>>);
static_assert(same<heim::to_type_sequence_t<std::array<int, 3>>, seq<int, int, int>>);
static_assert(same<heim::to_type_sequence_t<std::array<int, 8>>, heim::type_sequence_array_t<int, 8>>);
static_assert(same<heim::to_type_sequence_t<heim::type_sequence_tuple_t<repeated>>, repeated>);
static_assert(same<heim::type_sequence_tuple_t<heim::to_type_sequence_t<std::tuple<a, b>>>, std::tuple<a,
    b>>);
