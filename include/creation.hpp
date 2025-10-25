#ifndef HEIM_CREATION_HPP
#define HEIM_CREATION_HPP

#include <cstddef>
#include <memory>
#include "creation_info.hpp"
#include "lib/index_map.hpp"

namespace heim
{
template<
    typename Entity,
    typename TSeq   = type_sequence<>>
class creation
{
private:
  static_assert(
      is_creation_info_v<TSeq>,
      "heim::creation<Entity, TSeq>: is_creation_info_v<TSeq>;");

public:
  using entity_type   = Entity;
  using creation_info = TSeq;

public:
  template<
      typename    T,
      std::size_t PageSize = 4096,
      typename    Alloc    = std::allocator<T>>
  using component
  = creation<
      Entity,
      typename creation_info_traits<TSeq>
          ::template add_t<make_component_info<T, PageSize, Alloc>>
          ::type>;


  template<typename First, typename Second, typename ...Rest>
  using sync
  = creation<
      Entity,
      typename creation_info_traits<TSeq>
          ::template sync_t<First, Second, Rest ...>
          ::type>;

private:
  template<typename T>
  using component_info_t
  = sync_info_traits<typename TSeq::flat_t>
      ::template component_info_t<T>;


  template<typename USeq>
  struct to_index_map
  {
    using type
    = index_map<
        Entity,
        typename component_info_traits<USeq>::component_t,
        component_info_traits         <USeq>::page_size_v,
        typename component_info_traits<USeq>::allocator_t>;

  };

  using map_tuple_t
  = TSeq
      ::flat_t
      ::template map_t<to_index_map>;

private:
  map_tuple_t m_maps;

public:

};


} // namespace heim


#endif // HEIM_CREATION_HPP
