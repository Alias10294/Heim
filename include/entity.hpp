#ifndef HEIM_ENTITY_HPP
#define HEIM_ENTITY_HPP

#include <concepts>

namespace heim
{
namespace core
{
template<typename T>
concept entity = requires
{
  requires std::unsigned_integral<T>;

};

}
}

#endif // HEIM_ENTITY_HPP
