#ifndef HEIM_CORE_ENTITY_HPP
#define HEIM_CORE_ENTITY_HPP

#include <concepts>

namespace heim
{
namespace core
{
template<typename T>
concept entity = requires(T t)
{
  requires std::unsigned_integral<T>;

};

}
}

#endif // HEIM_CORE_ENTITY_HPP
