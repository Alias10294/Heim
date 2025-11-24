#ifndef HEIM_COMPONENT_MANAGER_HPP
#define HEIM_COMPONENT_MANAGER_HPP

#include <memory>
#include "lib/type_sequence.hpp"
#include "configuration.hpp"
#include "scheme.hpp"

namespace heim
{
template<
    typename Index,
    typename Scheme = type_sequence<>>
class component_manager
{
public:
  using index_type  = Index;
  using scheme_type = Scheme;


  using size_type = std::size_t;

private:
  using scheme_traits_t
  = scheme_traits<scheme_type>;


  using container_tuple_t
  = typename scheme_traits_t::template container_sequence<Index>::tuple;

  template<typename C>
  using container_of_t
  = typename scheme_traits_t::template container_of<Index, C>;

  template<typename C>
  static constexpr size_type
  container_index_of = scheme_traits_t::template component_index_of<C>;


  using sync_length_array_t
  = std::array<size_type, scheme_type::size>;

  template<typename C>
  static constexpr size_type
  sync_index_of = scheme_traits_t::template sync_index_of<C>;

public:
  template<
      typename    C,
      std::size_t PageSize = page_size_for_v<C>,
      typename    CAlloc   = allocator_for_t<C>>
  using declare_component
  = component_manager<
      Index,
      typename scheme_traits_t
          ::template declare_component<C, PageSize, CAlloc>
          ::type>;

  template<typename ...Cs>
  using declare_sync
  = component_manager<
      Index,
      typename scheme_traits_t
          ::template declare_sync<Cs ...>
          ::type>;

private:
  container_tuple_t   m_containers;
  sync_length_array_t m_sync_lengths;

public:
  constexpr void
  swap(component_manager &other)
  noexcept
  {
    m_containers.swap(other.m_containers);
  }

  friend constexpr void
  swap(component_manager &lhs, component_manager &rhs)
  noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }



  template<typename C>
  [[nodiscard]]
  constexpr container_of_t<C> &
  container()
  noexcept
  {
    return std::get<container_index_of<C>>(m_containers);
  }

  template<typename C>
  [[nodiscard]]
  constexpr container_of_t<C> const &
  container() const
  noexcept
  {
    return std::get<container_index_of<C>>(m_containers);
  }



  template<typename C>
  constexpr void
  swap(container_of_t<C> &other)
  noexcept
  {
    return container<C>().swap(other);
  }


  template<typename C>
  [[nodiscard]]
  constexpr auto
  get_allocator() const
  noexcept
  {
    return container<C>().get_allocator();
  }


  template<typename C>
  [[nodiscard]]
  constexpr size_type
  size() const
  noexcept
  {
    return container<C>().size();
  }


  template<typename C>
  [[nodiscard]]
  constexpr size_type
  max_size() const
  noexcept
  {
    return container<C>().max_size();
  }


  template<typename C>
  [[nodiscard]]
  constexpr bool
  empty() const
  noexcept
  {
    return container<C>().empty();
  }


  template<typename C>
  [[nodiscard]]
  constexpr size_type
  capacity() const
  noexcept
  {
    return container<C>().capacity();
  }


  template<typename C>
  constexpr void
  reserve(size_type const new_cap)
  {
    container<C>().reserve(new_cap);
  }


  template<typename C>
  constexpr void
  shrink_to_fit()
  {
    container<C>().shrink_to_fit();
  }


  template<typename C>
  [[nodiscard]]
  constexpr bool
  contains(index_type const i) const
  noexcept
  {
    return container<C>().contains(i);
  }


  template<typename C>
  [[nodiscard]]
  constexpr auto
  find(index_type const i)
  noexcept
  {
    return container<C>().find(i);
  }

  template<typename C>
  [[nodiscard]]
  constexpr auto
  find(index_type const i) const
  noexcept
  {
    return container<C>().find(i);
  }




};


}


#endif // HEIM_COMPONENT_MANAGER_HPP
