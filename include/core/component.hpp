#ifndef HEIM_CORE_COMPONENT_HPP
#define HEIM_CORE_COMPONENT_HPP

#include "composable.hpp"

namespace heim
{
namespace core
{

template<typename T>
concept component = requires(T t)
{
  requires composable<T>;

};

}
}

#endif // HEIM_CORE_COMPONENT_HPP
