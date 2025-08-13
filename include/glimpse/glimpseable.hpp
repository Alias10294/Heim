#ifndef HEIM_GLIMPSE_GLIMPSEABLE_HPP
#define HEIM_GLIMPSE_GLIMPSEABLE_HPP

#include <concepts>
#include <cstddef>
#include "iterator.hpp"

namespace heim
{
template<typename T>
concept glimpseable = requires(T t, typename T::entity_type const e)
{
  requires iterator<typename T::iterator      >;
  requires iterator<typename T::const_iterator>;
  { t.begin()  } noexcept;
  { t.end()    } noexcept;
  { t.cbegin() } noexcept;
  { t.cend()   } noexcept;

  { t.size() } noexcept -> std::same_as<std::size_t>;

  { t.contains(e) } noexcept -> std::same_as<bool>;

};

}

#endif // HEIM_GLIMPSE_GLIMPSEABLE_HPP
