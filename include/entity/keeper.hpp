#ifndef HEIM_ENTITY_KEEPER_HPP
#define HEIM_ENTITY_KEEPER_HPP

#include <concepts>
#include <cstddef>
#include <unordered_map>

#include "signature.hpp"

namespace heim
{
/**
 * @brief Manages and keeps the summoned entities and their signature.
 */
template<typename    Entity,
         std::size_t SignatureChunkSize = 64>
requires  std::unsigned_integral<Entity>
      && (SignatureChunkSize > 0)
class keeper
{
public:
  using entity_type = Entity;

  constexpr
  static std::size_t signature_chunk_size = SignatureChunkSize;

  using signature_type           = signature<signature_chunk_size>;
  using signature_container_type =
      std::unordered_map<entity_type, signature_type>;

  using iterator       = typename signature_container_type::iterator;
  using const_iterator = typename signature_container_type::const_iterator;

public:
  constexpr
  keeper(keeper const &)
  = default;
  constexpr
  keeper(keeper &&)
  noexcept
  = default;
  explicit
  constexpr
  keeper(std::size_t const sign_size)
    : signatures_{},
      signature_size_{sign_size}
  { }

  constexpr
  ~keeper()
  noexcept
  = default;


  constexpr
  keeper &operator=(keeper const &)
  = default;
  constexpr
  keeper &operator=(keeper &&)
  noexcept
  = default;



  /**
   * @return @c true if no entities are kept, @c false otherwise.
   */
  [[nodiscard]]
  constexpr
  bool empty() const
  noexcept
  {
    return signatures_.empty();
  }

  /**
   * @return The number of entities kept.
   */
  [[nodiscard]]
  constexpr
  std::size_t size() const
  noexcept
  {
    return signatures_.size();
  }

  /**
   * @return The maximum number of entities that can be kept.
   */
  [[nodiscard]]
  constexpr
  std::size_t max_size() const
  noexcept
  {
    return signatures_.max_size();
  }



  /**
   * @return An iterator to the first element kept.
   */
  [[nodiscard]]
  constexpr
  iterator       begin()
  noexcept
  {
    return signatures_.begin();
  }
  /**
   * @return A const iterator to the first element kept.
   */
  [[nodiscard]]
  constexpr
  const_iterator begin() const
  noexcept
  {
    return signatures_.begin();
  }

  /**
   * @return An iterator to after the last element kept.
   *
   * @warning This iterator only acts as a sentinel, and is not to be
   *     dereferenced.
   */
  [[nodiscard]]
  constexpr
  iterator       end()
  noexcept
  {
    return signatures_.end();
  }
  /**
   * @return A const iterator to after the last element kept.
   *
   * @warning This iterator only acts as a sentinel, and is not to be
   *     dereferenced.
   */
  [[nodiscard]]
  constexpr
  const_iterator end() const
  noexcept
  {
    return signatures_.end();
  }


  /**
   * @return A const iterator to the first element kept.
   */
  [[nodiscard]]
  constexpr
  const_iterator cbegin() const
  noexcept
  {
    return signatures_.cbegin();
  }

  /**
   * @return A const iterator to after the last element kept.
   *
   * @warning This iterator only acts as a sentinel, and is not to be
   *     dereferenced.
   */
  [[nodiscard]]
  constexpr
  const_iterator cend() const
  noexcept
  {
    return signatures_.cend();
  }



  /**
   * @param e The entity to search the element of.
   * @return @c true if @code e@endcode is kept, @c false otherwise.
   */
  [[nodiscard]]
  constexpr
  bool keeps(entity_type const e) const
  noexcept
  {
    return signatures_.contains(e);
  }

  /**
   * @param e The entity to find the element of.
   * @return An iterator to the element of @code e@endcode, or
   *     @code end()@endcode if not kept.
   */
  [[nodisscard]]
  constexpr
  iterator       find(entity_type const e)
  noexcept
  {
    return signatures_.find(e);
  }
  /**
   * @param e The entity to find the element of.
   * @return A const iterator to the element of @code e@endcode, or
   *     @code end()@endcode if not kept.
   */
  [[nodisscard]]
  constexpr
  const_iterator find(entity_type const e) const
  noexcept
  {
    return signatures_.find(e);
  }


  /**
   * @param e The entity to access the signature of.
   * @return A reference to the signature of @code e@endcode.
   */
  [[nodiscard]]
  constexpr
  signature_type       &operator[](entity_type const e)
  noexcept
  {
    return find(e)->second;
  }
  /**
   * @param e The entity to access the signature of.
   * @return A const reference to the signature of @code e@endcode.
   */
  [[nodiscard]]
  constexpr
  signature_type const &operator[](entity_type const e) const
  noexcept
  {
    return find(e)->second;
  }

  /**
   * @param e The entity to access the signature of.
   * @return A reference to the signature of @code e@endcode.
   * @throw std::out_of_range if @code e@endcode is not kept.
   */
  [[nodiscard]]
  constexpr
  signature_type       &at(entity_type const e)
  noexcept
  {
    return signatures_.at(e);
  }
  /**
   * @param e The entity to access the signature of.
   * @return A const reference to the signature of @code e@endcode.
   * @throw std::out_of_range if @code e@endcode is not kept.
   */
  [[nodiscard]]
  constexpr
  signature_type const &at(entity_type const e) const
  noexcept
  {
    return signatures_.at(e);
  }



  /**
   * @brief Adds @code e@endcode to be kept along with a new empty signature,
   *     if it is not already.
   *
   * @param e The entity to keep.
   * @return @c true if @code e@endcode is not already kept, @c false
   *     otherwise.
   */
  constexpr
  bool keep(entity_type const e)
  {
    return signatures_.try_emplace(e, signature_size_).second;
  }


  /**
   * @brief Removes @code e@endcode and its signature from the keeper.
   *
   * @param e The entity to remove.
   * @return @c true if @code e@endcode was kept, @c false otherwise.
   */
  constexpr
  bool remove(entity_type const e)
  noexcept
  {
    return signatures_.erase(e) != 0;
  }



  /**
   * @brief Swaps the contents of @p *this and @code other@endcode.
   *
   * @param other The other keeper to swap the contents of.
   */
  constexpr
  void swap(keeper &other)
  noexcept
  {
    std::swap(signatures_    , other.signatures_);
    std::swap(signature_size_, other.signature_size_);
  }

private:
  signature_container_type signatures_;
  std::size_t              signature_size_;

};


/**
 * @brief Swaps the contents of @code lhs@endcode and @code rhs@endcode.
 *
 * @tparam Entity             The type of the entities of the keepers.
 * @tparam SignatureChunkSize The size of each "chunk" of bits of the entities'
 *     signatures.
 * @param lhs The first  keeper to swap the contents of.
 * @param rhs The second keeper to swap the contents of.
 */
template<typename    Entity,
         std::size_t SignatureChunkSize = 64>
requires  std::unsigned_integral<Entity>
      && (SignatureChunkSize > 0)
constexpr
void swap(
    keeper<Entity, SignatureChunkSize> &lhs,
    keeper<Entity, SignatureChunkSize> &rhs)
noexcept
{
  lhs.swap(rhs);
}

}

#endif // HEIM_ENTITY_KEEPER_HPP
