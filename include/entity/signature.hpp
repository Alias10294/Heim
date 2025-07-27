#ifndef HEIM_ENTITY_SIGNATURE_HPP
#define HEIM_ENTITY_SIGNATURE_HPP

#include <bitset>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace heim
{
/**
 * @brief An implementation of a dynamic bitset.
 *
 * Provides much cheaper bit-to-bit operations than its STL counterpart
 *     std::vector<bool> while still offering rudimentary dynamic memory
 *     management.
 * Its implementation somewhat uses the concept of chunking, using regular
 * std::bitset as chunks of the signature.
 *
 * @tparam ChunkSize The size of the individual "chunk" bitsets in the
 *     signature.
 */
template<std::size_t ChunkSize = 32>
requires (ChunkSize > 0)
class signature
{
public:
  static constexpr std::size_t chunk_size = ChunkSize;

private:
  using bitset_type = std::bitset<chunk_size>;
  using reference   = typename bitset_type::reference;

public:
  constexpr
  signature()
  = default;
  constexpr
  signature(signature const &)
  = default;
  constexpr
  signature(signature &&)
  noexcept
  = default;
  explicit
  constexpr
  signature(std::size_t const size)
    : bitsets_{(size + chunk_size - 1) / chunk_size},
      size_   {size}
  { }

  constexpr
  ~signature()
  noexcept
  = default;


  [[nodiscard]]
  constexpr
  signature &operator=(signature const &)
  = default;
  [[nodiscard]]
  constexpr
  signature &operator=(signature &&)
  noexcept
  = default;



  /**
   * @return @c true if @code size() == 0@endcode, @c false otherwise.
   */
  [[nodiscard]]
  constexpr
  bool empty() const
  noexcept
  {
    return bitsets_.empty();
  }

  /**
   * @return The number of bits in the signature.
   */
  [[nodiscard]]
  constexpr
  std::size_t size() const
  noexcept
  {
    return size_;
  }

  /**
   * @brief Resizes the signature to contain @code size@endcode bits.
   *
   * @param size The new size of the signature.
   *
   * @note All new bits are set to zero.
   */
  constexpr
  void resize(std::size_t const size)
  {
    bitsets_.resize((size + chunk_size - 1) / chunk_size);
    bitsets_.shrink_to_fit();
    size_ = size;
  }



  [[nodiscard]]
  constexpr
  bool operator==(signature const &) const
  noexcept
  = default;


  /**
   * @param pos The position of the bit to access.
   * @return The value of the bit at position @code pos@endcode.
   */
  [[nodiscard]]
  constexpr
  bool      operator[](std::size_t const pos) const
  noexcept
  {
    return bitsets_[pos / chunk_size][pos % chunk_size];
  }
  /**
   * @param pos The position of the bit to access.
   * @return An object of type @p std::bitset::reference that allows
   *     modification of the value.
   */
  [[nodiscard]]
  constexpr
  reference operator[](std::size_t const pos)
  noexcept
  {
    return bitsets_[pos / chunk_size][pos % chunk_size];
  }

  /**
   * @param pos The position of the bit to access.
   * @return The value of the bit at position @code pos@endcode.
   * @throw std::out_of_range if @code pos >= size()@endcode.
   */
  [[nodiscard]]
  constexpr
  bool      at(std::size_t const pos) const
  {
    if (pos >= size_)
      throw std::out_of_range("heim::signature::at");
    return operator[](pos);
  }
  /**
   * @param pos The position of the bit to access.
   * @return An object of type @p std::bitset::reference that allows
   *     modification of the value.
   * @throw std::out_of_range if @code pos >= size()@endcode.
   */
  [[nodiscard]]
  constexpr
  reference at(std::size_t const pos)
  {
    if (pos >= size_)
      throw std::out_of_range("heim::signature::at");
    return operator[](pos);
  }


  /**
   * @return @c true if all bits are set to @c true, @c false otherwise.
   */
  [[nodiscard]]
  constexpr
  bool all() const
  noexcept
  {
    for (auto const &bitset : bitsets_)
    {
      if (!bitset.all())
        return false;
    }
    return true;
  }

  /**
   * @return @c true if any bits are set to @c true, @c false otherwise.
   */
  [[nodiscard]]
  constexpr
  bool any() const
  noexcept
  {
    for (auto const &bitset : bitsets_)
    {
      if (bitset.any())
        return true;
    }
    return false;
  }

  /**
   * @return @c true if none of the bits are set to @c true, @c false
   *     otherwise.
   */
  [[nodiscard]]
  constexpr
  bool none() const
  noexcept
  {
    return !any();
  }

  /**
   * @return The number of bits that are set to @c true .
   */
  [[nodiscard]]
  constexpr
  std::size_t count() const
  noexcept
  {
    std::size_t count = 0;
    for (auto const &bitset : bitsets_)
      count += bitset.count();
    return count;
  }



  [[nodiscard]]
  constexpr
  signature operator&(signature const &other) const
  {
    if (size() != other.size())
      throw std::invalid_argument("heim::signature::operator&: size mismatch");

    signature result{size_};
    for (std::size_t i = 0; i < result.bitsets_.size(); ++i)
      result.bitsets_[i] = bitsets_[i] & other.bitsets_[i];

    result.reset_tail();
    return result;
  }

  [[nodiscard]]
  constexpr
  signature operator|(signature const &other) const
  {
    if (size() != other.size())
      throw std::invalid_argument("heim::signature::operator|: size mismatch");

    signature result{size_};
    for (std::size_t i = 0; i < result.bitsets_.size(); ++i)
      result.bitsets_[i] = bitsets_[i] | other.bitsets_[i];

    result.reset_tail();
    return result;
  }

  [[nodiscard]]
  constexpr
  signature operator^(signature const &other) const
  {
    if (size() != other.size())
      throw std::invalid_argument("heim::signature::operator^: size mismatch");

    signature result{size_};
    for (std::size_t i = 0; i < result.bitsets_.size(); ++i)
      result.bitsets_[i] = bitsets_[i] ^ other.bitsets_[i];

    result.reset_tail();
    return result;
  }

  [[nodiscard]]
  constexpr
  signature operator~() const
  noexcept
  {
    signature result{size_};
    for (std::size_t i = 0; i < result.bitsets_.size(); ++i)
      result.bitsets_[i] = ~bitsets_[i];

    result.reset_tail();
    return result;
  }


  [[nodiscard]]
  constexpr
  signature &operator&=(signature const &other)
  {
    return *this = *this & other;
  }

  [[nodiscard]]
  constexpr
  signature &operator|=(signature const &other)
  {
    return *this = *this | other;
  }

  [[nodiscard]]
  constexpr
  signature &operator^=(signature const &other)
  {
    return *this = *this ^ other;
  }


  /**
   * @brief Sets all bits to @c true .
   *
   * @return @p *this .
   */
  [[nodiscard]]
  constexpr
  signature &set()
  noexcept
  {
    for (auto const &bitset : bitsets_)
      bitset.set();

    reset_tail();
    return *this;
  }
  /**
   * @brief Sets the bit at position @code pos@endcode to @code value@endcode.
   *
   * @param pos The position of the bit to set.
   * @param value The value to set the bit of.
   * @return @p *this .
   */
  [[nodiscard]]
  constexpr
  signature &set(std::size_t pos, bool value = true)
  {
    at(pos) = value;
    return *this;
  }

  /**
   * @brief Resets all bits to @c false .
   *
   * @return @p *this .
   */
  [[nodiscard]]
  constexpr
  signature &reset()
  noexcept
  {
    for (auto const &bitset : bitsets_)
      bitset.reset();

    reset_tail();
    return *this;
  }
  /**
   * @brief Resets the bit at position @code pos@endcode.
   *
   * @param pos The position of the bit to reset.
   * @return @p *this .
   */
  [[nodiscard]]
  constexpr
  signature &reset(std::size_t pos)
  {
    at(pos) = false;
    return *this;
  }

  /**
   * @brief Flips all bits.
   *
   * @return @p *this .
   */
  [[nodiscard]]
  constexpr
  signature &flip()
  {
    for (auto const &bitset : bitsets_)
      bitset.flip();

    reset_tail();
    return *this;
  }
  /**
   * @brief Flips the bit at the position @code pos@endcode.
   *
   * @param pos The position of the bit to flip.
   * @return @p *this .
   */
  [[nodiscard]]
  constexpr
  signature &flip(std::size_t pos)
  {
    at(pos) = !at(pos);
    return *this;
  }



  /**
   * @brief Swaps the contents of @p *this and @code other@endcode.
   *
   * @param other The other signature to swap the contents of.
   */
  constexpr
  void swap(signature &other)
  noexcept
  {
    std::swap(bitsets_, other.bitsets_);
    std::swap(size_   , other.size_);
  }

private:

  /// @cond INTERNAL

  /**
   * @brief Resets the bits exceeding the signature's size to @c false.
   */
  constexpr
  void reset_tail()
  noexcept
  {
    std::size_t const tail_size = size_ % chunk_size;
    if (tail_size == 0)
      return;

    bitset_type tail_mask;
    for (std::size_t i = 0; i < tail_size; ++i)
      tail_mask.set(i);
    bitsets_.back() &= tail_mask;
  }

  /// @endcond

private:
  std::vector<bitset_type> bitsets_;
  std::size_t              size_;

};


/**
 * @brief Swaps the contents of @code lhs@endcode and @code rhs@endcode.
 *
 * @param lhs The first  signature to swap the contents of.
 * @param rhs The second signature to swap the contents of.
 */
template<std::size_t ChunkSize = 32>
requires (ChunkSize > 0)
constexpr
void swap(signature<ChunkSize> &lhs, signature<ChunkSize> &rhs)
noexcept
{
  lhs.swap(rhs);
}

}

#endif // HEIM_ENTITY_SIGNATURE_HPP
