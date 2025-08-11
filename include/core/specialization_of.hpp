#ifndef HEIM_CORE_SPECIALIZATION_OF_HPP
#define HEIM_CORE_SPECIALIZATION_OF_HPP

#include <type_traits>

namespace heim
{
namespace core
{
template<typename                       T,
         template<typename...> typename Generic>
struct is_specialization_of : std::false_type
{ };

template<template<class ...> class Generic,
         class                     ...Args>
struct is_specialization_of<Generic<Args ...>, Generic> : std::true_type
{ };

template<typename                       T,
         template<typename...> typename Generic>
concept specialization_of = is_specialization_of<T, Generic>::value;

}
}

#endif // HEIM_CORE_SPECIALIZATION_OF_HPP
