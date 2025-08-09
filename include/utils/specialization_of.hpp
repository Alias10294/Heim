#ifndef HEIM_UTILS_SPECIALIZATION_OF_HPP
#define HEIM_UTILS_SPECIALIZATION_OF_HPP

#include <type_traits>

namespace heim
{
template<typename                        T,
         template<typename ...> typename Generic>
struct is_specialization_of : std::false_type
{ };

template<template<typename ...> typename Generic,
         typename                     ...Args>
struct is_specialization_of<Generic<Args ...>, Generic> : std::true_type
{ };

template<typename                        T,
         template<typename ...> typename Generic>
constexpr
inline bool is_specialization_of_v = is_specialization_of<T, Generic>::value;


template<typename                        T,
         template<typename ...> typename Generic>
concept specialization_of = is_specialization_of_v<T, Generic>;

}

#endif // HEIM_UTILS_SPECIALIZATION_OF_HPP
