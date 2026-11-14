//===-- Pseudo LRU Cache ------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file plru_cache.hpp
/// \brief Implements a "Tree-PLRU" (Pseudo Least-recently Used) unordered associative container.
///
/// It is intended as a small cache for objects which are relatively cheap to store and relatively
/// expensive to create. The container's keys must be unsigned integral types.

#ifndef MIDI2_ADT_PLRU_CACHE_HPP
#define MIDI2_ADT_PLRU_CACHE_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <climits>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <numeric>
#include <type_traits>
#include <utility>

#if defined(__ARM_NEON) && __ARM_NEON
#include <arm_neon.h>
#endif  // __ARM_NEON

#include "midi2/adt/uinteger.hpp"

namespace midi2::adt {

/// \brief Private implementation details of the library abstract data types
namespace details {

/// \brief Uninitialized storage which is suitably sized and aligned for an object of type \p T.
/// \tparam T  The type of the object to be stored.
template <typename T> struct aligned_storage {
  /// \brief Returns a reference to the stored object.
  /// \returns A reference to the stored object.
  [[nodiscard]] constexpr T& value() noexcept { return *std::bit_cast<T*>(&v[0]); }
  /// \brief Returns a const reference to the stored object.
  /// \returns A const reference to the stored object.
  [[nodiscard]] constexpr T const& value() const noexcept { return *std::bit_cast<T const*>(&v[0]); }
  /// \brief The storage.
  alignas(T) std::byte v[sizeof(T)];
};

/// \brief The binary tree of access bits used to implement the Tree-PLRU replacement policy for a single set.
/// \tparam Ways  The number of members of the set. Must be a power of 2.
template <std::size_t Ways> class tree {
public:
  /// Flip the access bits of the tree to indicate that \p way is the most recently used member.
  /// \param way  The index of the member which was accessed.
  void touch(std::size_t const way) noexcept {
    assert(way < Ways && "Way index is too large");
    auto node = std::size_t{0};
    auto start = std::size_t{0};
    auto end = Ways;
    while (node < Ways - 1U) {
      auto const mid = std::midpoint(start, end);
      auto const is_less = way < mid;
      if (is_less) {
        end = mid;
      } else {
        start = mid;
      }
      bits_[node] = is_less;
      node = 2U * node + 1U + static_cast<unsigned>(!is_less);
    }
  }

  /// Traverses the tree to find the index of the oldest member.
  /// \returns The index of the (approximately) least recently used member.
  [[nodiscard]] constexpr std::size_t oldest() const noexcept {
    auto node = std::size_t{0};
    while (node < Ways - 1U) {
      node = 2U * node + 1U + static_cast<unsigned>(bits_[node]);
    }
    return node - (Ways - 1U);
  }

  /// Resets the access bits to their initial state.
  constexpr void reset() { bits_.reset(); }

private:
  std::bitset<Ways - 1U> bits_{};
};

/// \brief Used to store keys in the cache.
///
/// We don't store the key directly in this struct: some of the bits are determined by the key's set so don't need to
/// be recorded here. We do need to record whether a key entry is occupied, so there's a bit used for that.
///
/// The low SetBits bits of the key are its set number. Bit 0 of the stored value records that the entry is occupied:
/// it overlaps the most significant bit of the set number, which need not be stored. There must therefore be at least
/// one set bit.
///
/// \tparam Key  The key type.
/// \tparam SetBits  The number of low bits of a key which select its set. Must be greater than 0.
template <std::unsigned_integral Key, unsigned SetBits>
  requires(SetBits > 0)
class tagged_key {
public:
  /// \brief The type used to store the tag.
  using value_type = uinteger<sizeof(Key) * CHAR_BIT - SetBits + 1>::type;

  /// \brief Constructs an unoccupied (invalid) key.
  constexpr tagged_key() noexcept = default;
  /// \brief Constructs a valid tag from a key.
  /// \param key  The key.
  constexpr explicit tagged_key(Key key) noexcept : v_{static_cast<value_type>(1U | (key >> (SetBits - 1U)))} {}
  /// \brief Compares two tags for equality.
  /// \returns True if the two tags are equal.
  friend constexpr bool operator==(tagged_key const&, tagged_key const&) noexcept = default;
  /// \brief Returns the key from which this value was created, with the set number bits 0.
  /// \returns The key from which this value was created, with the set number bits 0.
  [[nodiscard]] constexpr Key key() const noexcept {
    return static_cast<Key>(static_cast<Key>(v_ & ~value_type{1}) << (SetBits - 1U));
  }
  /// \brief Returns true if the tag represents an occupied entry.
  /// \returns True if the tag represents an occupied entry.
  [[nodiscard]] constexpr bool valid() const noexcept { return static_cast<bool>(v_ & 1); }
#if defined(__cpp_explicit_this_parameter) && __cpp_explicit_this_parameter >= 202110L
  /// \brief Returns a reference to the stored tag value.
  /// \param self  The object whose tag value is to be returned.
  /// \returns A reference to the stored tag value.
  [[nodiscard]] constexpr decltype(auto) get(this auto& self) noexcept { return self.v_; }
#else
  /// \brief Returns a reference to the stored tag value.
  /// \returns A reference to the stored tag value.
  [[nodiscard]] constexpr value_type& get() noexcept { return v_; }
  /// \brief Returns a const reference to the stored tag value.
  /// \returns A const reference to the stored tag value.
  [[nodiscard]] constexpr value_type const& get() const noexcept { return v_; }
#endif

private:
  value_type v_ = 0;
};

/// \brief Searches an array of tagged keys for a match.
///
/// The generic implementation is a simple linear search. There are specializations which use SIMD instructions where
/// available.
///
/// \tparam Key  The key type.
/// \tparam SetBits  The number of low bits of a key which select its set.
/// \tparam Ways  The number of members of a set.
template <std::unsigned_integral Key, unsigned SetBits, unsigned Ways> struct match_finder {
  /// \brief The type of the tagged keys being searched.
  using tagged_key_type = tagged_key<Key, SetBits>;

  /// \brief Searches \p values for \p tk.
  /// \param tk  The tagged key to be found.
  /// \param values  The tagged keys to be searched.
  /// \return The lane index [0..Ways) if a match is found, or Ways if no match
  static constexpr std::size_t find(tagged_key_type const tk,
                                    std::array<tagged_key_type, Ways> const& values) noexcept {
    auto index = std::size_t{0};
    for (; index < Ways; ++index) {
      if (values[index] == tk) {
        break;
      }
    }
    return index;
  }
};

#if defined(__ARM_NEON) && __ARM_NEON
template <std::unsigned_integral Key, unsigned SetBits>
  requires(std::is_same_v<typename tagged_key<Key, SetBits>::value_type, std::uint16_t>)
struct match_finder<Key, SetBits, 4> {
  using tagged_key_type = tagged_key<Key, SetBits>;

  static constexpr std::size_t find(tagged_key_type const new_tag,
                                    std::array<tagged_key_type, 4> const& values) noexcept {
    uint16x4_t const a = vdup_n_u16(new_tag.get());
    uint16x4_t const b = vld1_u16(std::bit_cast<std::uint16_t const*>(values.data()));

    uint16x4_t const cmp = vceq_u16(a, b);  // Compare lanes: 0xFFFF if equal
    // Treat 4 16-bit lanes as a single 64-bit scalar
    std::uint64_t const packed = vget_lane_u64(vreinterpret_u64_u16(cmp), 0);

    // Extract the matching lane by counting trailing groups of 16 bits. If packed is zero, no lane matched.
    // For nonzero packed: ctz = 16*i (i = matching lane) -> (ctz+1)/16 = i.
    // For zero packed:    ctz = 63 -> (ctz+1)/16 = 4 (no match).
    std::uint64_t const nz_or = packed | (static_cast<std::uint64_t>(packed == 0) << 63);
    return (static_cast<std::size_t>(static_cast<unsigned>(std::countr_zero(nz_or)) + 1U)) / 16U;
  }
};

template <std::unsigned_integral Key, unsigned SetBits>
  requires(std::is_same_v<typename tagged_key<Key, SetBits>::value_type, std::uint16_t>)
struct match_finder<Key, SetBits, 8> {
  using tagged_key_type = tagged_key<Key, SetBits>;

  static constexpr std::size_t find(tagged_key_type const new_tag,
                                    std::array<tagged_key_type, 8> const& values) noexcept {
    uint16x8_t const a = vdupq_n_u16(new_tag.get());
    uint16x8_t const b = vld1q_u16(std::bit_cast<std::uint16_t const*>(values.data()));

    uint16x8_t const cmp = vceqq_u16(a, b);   // Compare lanes: 0xFFFF if equal
    uint8x8_t const narrow = vmovn_u16(cmp);  // Narrow to 8-bit: 0xFF if equal, 0x00 if not
    std::uint64_t const packed =
        vget_lane_u64(vreinterpret_u64_u8(narrow), 0);  // Treat 8 lanes as a single 64-bit scalar

    // Extract the matching lane by counting trailing groups of 8 bits. If packed is zero, no lane matched.
    // For nonzero packed: ctz = 8*i (i = matching lane).
    // For zero packed:    ctz = 63  -> (ctz+1)>>3 = 8  (no match).
    std::uint64_t const nz_or = packed | (static_cast<std::uint64_t>(packed == 0) << 63);
    return (static_cast<std::size_t>(static_cast<unsigned>(std::countr_zero(nz_or)) + 1U)) >> 3;
  }
};

template <std::unsigned_integral Key, unsigned SetBits>
  requires(std::is_same_v<typename tagged_key<Key, SetBits>::value_type, std::uint32_t>)
struct match_finder<Key, SetBits, 4> {
  using tagged_key_type = tagged_key<Key, SetBits>;

  static constexpr std::size_t find(tagged_key_type const new_tag,
                                    std::array<tagged_key_type, 4> const& values) noexcept {
    uint32x4_t const a = vdupq_n_u32(new_tag.get());
    uint32x4_t const b = vld1q_u32(std::bit_cast<std::uint32_t const*>(values.data()));

    uint32x4_t const cmp = vceqq_u32(a, b);    // Compare lanes: 0xFFFFFFFF if equal
    uint16x4_t const narrow = vmovn_u32(cmp);  // Narrow to 16-bit: 0xFFFF if equal, 0x0000 if not
    std::uint64_t const packed =
        vget_lane_u64(vreinterpret_u64_u16(narrow), 0);  // Treat 4 16-bit lanes as a single 64-bit scalar

    // Extract the matching lane by counting trailing groups of 16 bits. If packed is zero, no lane matched.
    // For nonzero packed: ctz = 16*i (i = matching lane) -> (ctz+1)/16 = i.
    // For zero packed:    ctz = 63 -> (ctz+1)/16 = 4 (no match).
    std::uint64_t const nz_or = packed | (static_cast<std::uint64_t>(packed == 0) << 63);
    return (static_cast<unsigned>(std::countr_zero(nz_or)) + 1U) / 16U;
  }
};

#endif  // __ARM_NEON

/// \brief A single set of the cache: up to \p Ways key/value pairs together with their PLRU access tree.
///
/// \tparam Key  The key type.
/// \tparam MappedType  The value type.
/// \tparam SetBits  The number of low bits of a key which select its set.
/// \tparam Ways  The number of members of the set.
template <std::unsigned_integral Key, typename MappedType, unsigned SetBits, std::size_t Ways> class cache_set {
  using tagged_key_type = tagged_key<Key, SetBits>;

public:
  /// \brief Searches the set for \p key, creating the associated value if it is not present or is no longer valid.
  ///
  /// \tparam MissFn  The type of the function called to instantiate a value.
  /// \tparam ValidFn  The type of the function called to check the validity of a cached value.
  /// \param key  The key of the element to search for.
  /// \param miss  Called as miss(key, index) to create the value associated with \p key.
  /// \param valid  Called to determine whether a cached value remains valid.
  /// \param index  The cache-wide index of the first member of this set. The index of the member that is being
  ///   populated is added to this value before it is passed to \p miss.
  /// \returns A reference to the cached value.
  template <typename MissFn, typename ValidFn>
    requires(std::is_invocable_r_v<MappedType, MissFn, Key, std::size_t> &&
             std::is_invocable_r_v<bool, ValidFn, MappedType const&>)
  MappedType& access(Key key, MissFn miss, ValidFn valid, std::size_t index) {
    auto const tag = tagged_key_type{key};
    if (auto const vi = find_matching(tag); vi < Ways) {
      assert(keys_[vi].valid());
      auto& found_result = values_[vi].value();
      if (!valid(found_result)) {
        found_result = miss(key, index + vi);
      }
      plru_.touch(vi);
      return found_result;
    }

    // Find the array member that is to be re-used by traversing the tree.
    std::size_t const victim = plru_.oldest();
    // The key was not found: call miss() to populate it.
    auto& result = values_[victim].value();
    if (keys_[victim].valid()) {
      result = miss(key, index + victim);
    } else {
      std::construct_at(&result, miss(key, index + victim));
    }
    keys_[victim] = std::move(tag);
    plru_.touch(victim);
    return result;
  }

  /// \brief Checks whether the set contains \p key.
  /// \param key  The key to search for.
  /// \returns True if the set contains \p key.
  [[nodiscard]] constexpr bool contains(Key key) const noexcept { return find_matching(tagged_key_type{key}) < Ways; }
  /// \brief Destroys all of the values in the set.
  constexpr void clear() noexcept {
    for (auto index = std::size_t{0}; index < Ways; ++index) {
      if (keys_[index].valid()) {
        std::destroy_at(&values_[index].value());
      }
      keys_[index] = tagged_key_type{};
    }
    plru_.reset();
  }
  /// \brief Returns the number of occupied members of the set.
  /// \returns The number of occupied members of the set.
  [[nodiscard]] constexpr std::size_t size() const noexcept {
    return static_cast<std::size_t>(
        std::ranges::count_if(keys_, [](tagged_key_type const& v) constexpr { return v.valid(); }));
  }

  /// \brief Checks whether a member of the set is occupied.
  /// \param index  The index of the member [0, Ways).
  /// \returns True if the member is occupied.
  [[nodiscard]] constexpr bool valid(std::size_t index) const noexcept {
    assert(index < Ways);
    return keys_[index].valid();
  }
  /// \brief Returns the key of a member of the set, excluding the bits which select the set.
  /// \param index  The index of the member [0, Ways).
  /// \returns The key with its set bits cleared. The caller must OR in the set number to obtain the full key.
  [[nodiscard]] constexpr Key key(std::size_t const index) const noexcept {
    assert(index < keys_.size());
    return keys_[index].key();  // this not the entire key: the set number bits are 0.
  }
  /// \brief Returns the value of a member of the set.
  /// \param index  The index of the member [0, Ways). The member must be occupied.
  /// \returns A const reference to the value.
  [[nodiscard]] constexpr MappedType const& value(std::size_t const index) const noexcept {
    assert(index < values_.size());
    return values_[index].value();
  }
  /// \brief Returns the value of a member of the set.
  /// \param index  The index of the member [0, Ways). The member must be occupied.
  /// \returns A reference to the value.
  [[nodiscard]] constexpr MappedType& value(std::size_t const index) noexcept {
    assert(index < values_.size());
    return values_[index].value();
  }

private:
  /// Search the keys_ array for the key specified by \p tk.
  ///
  /// \param tk The key for which the function will search
  /// \returns  The index of the located key if present otherwise \p Ways.
  [[nodiscard]] constexpr std::size_t find_matching(tagged_key_type tk) const noexcept {
    return match_finder<Key, SetBits, Ways>::find(tk, keys_);
  }

  std::array<tagged_key_type, Ways> keys_{};
  std::array<aligned_storage<MappedType>, Ways> values_{};
  tree<Ways> plru_{};
};

}  // end namespace details

/// \brief A "Tree-PLRU" (Pseudo Least-recently Used) unordered associative container.
///
/// It is intended as a small cache for objects which are relatively cheap to store and
/// relatively expensive to create. The container's keys must be unsigned integral types.
///
/// The total number of cache entries is given by Sets * Ways.
///
/// \tparam Key  The key type.
/// \tparam Mapped  The value type.
/// \tparam Sets  The number of sets. A key's set is given by its low bits (key & (Sets - 1)); all of the keys which
///   map to the same set compete for that set's entries. Must be a power of 2 greater than 1.
/// \tparam Ways  The number of entries within each set. The number of ways in a set determines how many entries
///   with the same set index can coexist. Must be a power of 2.
template <std::unsigned_integral Key, typename Mapped, std::size_t Sets, std::size_t Ways>
  requires(std::popcount(Sets) == 1 && std::popcount(Ways) == 1 && Sets > 1)
class plru_cache {
public:
  using key_type = Key;                             ///< The key type.
  using mapped_type = Mapped;                       ///< The type of the values held by the cache.
  using value_type = std::pair<Key const, Mapped>;  ///< The type of a key/value pair.

  static constexpr std::size_t sets = Sets;  ///< The number of sets.
  static constexpr std::size_t ways = Ways;  ///< The number of entries in each set.

  /// A proxy type that pretends to be type pair<const Key, T>&
  /// \tparam T  The mapped type, possibly const-qualified.
  template <typename T>
    requires(std::is_same_v<T, mapped_type> || std::is_same_v<T, mapped_type const>)
  struct proxy {
    /// \brief The key.
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    Key const first;
    /// \brief A reference to the value.
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    T& second;
    /// \brief Converts to a std::pair<> holding a copy of the key and the value.
    /// \returns A std::pair<> holding a copy of the key and the value.
    // NOLINTNEXTLINE(hicpp-explicit-conversions,google-explicit-constructor)
    constexpr operator std::pair<Key const, std::remove_const_t<T>>() const noexcept { return {first, second}; }
    /// \brief Compares two proxies for equality.
    /// \returns True if the two proxies are equal.
    constexpr friend bool operator==(proxy const&, proxy const&) noexcept = default;
    /// \brief Compares a proxy with a key/value pair.
    /// \param lhs  The proxy to be compared.
    /// \param rhs  The key/value pair to be compared.
    /// \returns True if the key and value of \p lhs are equal to those of \p rhs.
    constexpr friend bool operator==(proxy const& lhs,
                                     std::pair<Key const, std::remove_const_t<T>> const& rhs) noexcept {
      return lhs.first == rhs.first && lhs.second == rhs.second;
    }
    /// \brief Compares a key/value pair with a proxy.
    /// \param lhs  The key/value pair to be compared.
    /// \param rhs  The proxy to be compared.
    /// \returns True if the key and value of \p lhs are equal to those of \p rhs.
    constexpr friend bool operator==(std::pair<Key const, std::remove_const_t<T>> const& lhs,
                                     proxy const& rhs) noexcept {
      return lhs.first == rhs.first && lhs.second == rhs.second;
    }
  };

  /// \brief A forward iterator over the occupied entries of the cache.
  /// \tparam T  The mapped type: const-qualified for a const_iterator.
  template <typename T>
    requires(std::is_same_v<T, mapped_type> || std::is_same_v<T, mapped_type const>)
  class iterator_type {
  public:
    /// \brief The type of the cache being iterated.
    using owner_type = std::conditional_t<std::is_const_v<T>, plru_cache const, plru_cache>;

    using value_type = std::pair<Key const, T>;  ///< The type of a key/value pair.
    using reference = proxy<T>;                  ///< The type returned by dereferencing the iterator.
    using pointer = proxy<T>*;                   ///< A pointer to the reference type.
    using difference_type = std::ptrdiff_t;      ///< The type of the difference between two iterators.

    using iterator_category = std::forward_iterator_tag;  ///< The iterator category.

    /// \brief Constructs an "end" iterator.
    /// \param owner  The cache being iterated.
    constexpr explicit iterator_type(owner_type* const owner) noexcept : owner_{owner} {}
    /// \brief Constructs an iterator which refers to a specific entry.
    /// \param owner  The cache being iterated.
    /// \param indexes  The set and way indexes of the entry.
    constexpr iterator_type(owner_type* const owner, std::pair<std::size_t, std::size_t> const& indexes) noexcept
        : owner_{owner}, set_index_{indexes.first}, way_index_{indexes.second} {}
    /// \brief Compares two iterators for equality.
    /// \returns True if the two iterators are equal.
    constexpr friend bool operator==(iterator_type const&, iterator_type const&) noexcept = default;

    /// \brief Returns a proxy for the entry referenced by the iterator.
    /// \returns A proxy for the entry referenced by the iterator.
    constexpr reference operator*() const
      requires(std::is_const_v<T>)
    {
      auto const& s = owner_->sets_[set_index_];
      return proxy<T>{static_cast<Key>(s.key(way_index_) | set_index_), s.value(way_index_)};
    }
    /// \brief Returns a proxy for the entry referenced by the iterator.
    /// \returns A proxy for the entry referenced by the iterator.
    constexpr reference operator*()
      requires(!std::is_const_v<T>)
    {
      auto& s = owner_->sets_[set_index_];
      return proxy<Mapped>{static_cast<Key>(s.key(way_index_) | set_index_), s.value(way_index_)};
    }
    /// \brief Advances the iterator to the next occupied entry.
    /// \returns *this
    constexpr iterator_type& operator++() {
      do {
        ++way_index_;
        if (way_index_ >= plru_cache::ways) {
          ++set_index_;
          if (set_index_ >= plru_cache::sets) {
            break;
          }
          way_index_ = 0;
        }
      } while (!this->valid());
      assert(((way_index_ == plru_cache::ways && set_index_ == plru_cache::sets) || this->valid()) &&
             R"(post-condition says that we reference a valid object or "end")");
      return *this;
    }

  private:
    owner_type* owner_ = nullptr;
    std::size_t set_index_ = sets;
    std::size_t way_index_ = ways;

    [[nodiscard]] constexpr bool valid() const { return owner_->sets_[set_index_].valid(way_index_); }
  };

  /// \brief Deduces a mutable iterator type from a pointer to a cache.
  /// \tparam T  The cache type.
  template <typename T> iterator_type(T*) -> iterator_type<typename T::mapped_type>;
  /// \brief Deduces a const iterator type from a pointer to a const cache.
  /// \tparam T  The cache type.
  template <typename T> iterator_type(T const*) -> iterator_type<typename T::mapped_type const>;

  using iterator = iterator_type<Mapped>;              ///< A forward iterator over the entries of the cache.
  using const_iterator = iterator_type<Mapped const>;  ///< A const forward iterator over the entries of the cache.

  /// \brief Constructs an empty cache.
  constexpr plru_cache() noexcept = default;
  // (note that the following member functions aren't provided simply because I don't currently
  // have a need for them.)
  plru_cache(plru_cache const&) = delete;
  plru_cache(plru_cache&&) noexcept = delete;
  /// \brief Destroys the cache and all of its values.
  ~plru_cache() noexcept { clear(); }
  plru_cache& operator=(plru_cache const&) noexcept = delete;
  plru_cache& operator=(plru_cache&&) noexcept = delete;

  constexpr friend bool operator==(plru_cache const&, plru_cache const&) = delete;

  /// Searches the cache for \p key and returns a reference to its value if present and valid. If the key is present
  /// but \p valid returns false for its value, \p miss is called to replace the value. If the key is not present, the
  /// likely least recently used member of the key's set is evicted then \p miss is called to instantiate the value.
  ///
  /// \tparam MissFn  The type of the function called to instantiate a value in the cache.
  /// \tparam ValidFn  The type of the function called to check the validity of a value in the cache.
  /// \param key  Key value of the element to search for.
  /// \param miss  The function that will be called to instantiate the associated value if \p key
  ///   is not present in the cache. The function must be compatible with the signature
  ///   mapped_type(key_type key, std::size_t index) where index is the index of the cache entry [0, Sets * Ways)
  ///   which is being populated.
  /// \param valid  The function that will be called to determine whether the cached value remains
  ///   valid. It must be compatible with the signature bool(mapped_type const&).
  /// \returns The cached value.
  template <typename MissFn, typename ValidFn>
    requires(std::is_invocable_r_v<mapped_type, MissFn, Key, std::size_t> &&
             std::is_invocable_r_v<bool, ValidFn, mapped_type const&>)
  mapped_type& access(key_type key, MissFn miss, ValidFn valid) {
    assert(plru_cache::set(key) < Sets);
    return sets_[plru_cache::set(key)].access(key, std::move(miss), std::move(valid),
                                              plru_cache::set(key) * plru_cache::ways);
  }

  /// Searches the cache for the \p key and returns a reference to it if present. If not present
  /// and the cache is fully occupied, the likely least recently used value is evicted from the
  /// cache then the \p miss function is called to instantiate the mapped type associated with
  /// \p key. This value is then stored in the cache.
  ///
  /// \tparam MissFn  The type of the function called to instantiate a value in the cache.
  /// \param key  Key value of the element to search for.
  /// \param miss  The function that will be called to instantiate the associated value if \p key
  ///   is not present in the cache. The function must be compatible with the signature
  ///   mapped_type(key_type key, std::size_t index) where index is the index of the cache entry [0, Sets * Ways)
  ///   which is being populated.
  /// \returns The cached value.
  template <typename MissFn>
    requires(std::is_invocable_r_v<mapped_type, MissFn, Key, std::size_t>)
  mapped_type& access(key_type const key, MissFn const miss) {
    return this->access(key, miss, [](mapped_type const&) constexpr noexcept { return true; });
  }

  /// Checks if there is an element with a key that compares equivalent to \p key
  ///
  /// \param key  The key to search for.
  /// \return true if there is such an element, otherwise false.
  [[nodiscard]] constexpr bool contains(key_type key) const noexcept {
    assert(plru_cache::set(key) < Sets);
    return sets_[plru_cache::set(key)].contains(key);
  }

  /// \brief Clears the contents of the cache.
  constexpr void clear() noexcept {
    for (ways_type& w : sets_) {
      w.clear();
    }
  }

  /// Returns an iterator to the beginning
  /// \returns An iterator to the first occupied entry, or end() if the cache is empty.
  [[nodiscard]] constexpr iterator begin() noexcept { return {this, this->first_valid()}; }
  /// Returns a const iterator to the beginning
  /// \returns An iterator to the first occupied entry, or end() if the cache is empty.
  [[nodiscard]] constexpr const_iterator begin() const noexcept { return {this, this->first_valid()}; }
  /// Returns a const iterator to the beginning
  /// \returns An iterator to the first occupied entry, or cend() if the cache is empty.
  [[nodiscard]] constexpr const_iterator cbegin() noexcept { return {this, this->first_valid()}; }

  /// Returns an iterator to the end
  /// \returns An iterator past the last entry.
  [[nodiscard]] constexpr iterator end() noexcept { return iterator_type{this}; }
  /// Returns a const iterator to the end
  /// \returns An iterator past the last entry.
  [[nodiscard]] constexpr const_iterator end() const noexcept { return iterator_type{this}; }
  /// Returns a const iterator to the end
  /// \returns An iterator past the last entry.
  [[nodiscard]] constexpr const_iterator cend() noexcept { return iterator_type{this}; }

  /// \returns The maximum possible number of elements that can be held by the cache.
  [[nodiscard]] constexpr std::size_t max_size() const noexcept { return Sets * Ways; }
  /// \returns The number of elements held by the cache.
  [[nodiscard]] constexpr std::size_t size() const noexcept {
    return std::ranges::fold_left(sets_, std::size_t{0},
                                  [](std::size_t acc, auto const& set) { return acc + std::size(set); });
  }

  /// \brief Returns the set number of a key.
  /// \param key  The key
  /// \returns The set number of a supplied key
  [[nodiscard]] static constexpr std::size_t set(key_type key) noexcept { return key & (Sets - 1U); }
  /// \brief Returns the way number of a key.
  /// \param key  The key
  /// \returns The way number of a supplied key
  [[nodiscard]] static constexpr std::size_t way(key_type key) noexcept { return (key >> set_bits_) & (Ways - 1U); }

private:
  static constexpr unsigned set_bits_ = std::bit_width(Sets - 1U);
  using ways_type = details::cache_set<key_type, mapped_type, set_bits_, ways>;
  std::array<ways_type, Sets> sets_{};

  [[nodiscard]] constexpr std::pair<std::size_t, std::size_t> first_valid() const {
    // Search for the first in-use slot.
    auto set = std::size_t{0};
    auto way = std::size_t{0};
    while (!sets_[set].valid(way)) {
      ++way;
      if (way >= plru_cache::ways) {
        ++set;
        if (set >= plru_cache::sets) {
          break;
        }
        way = 0;
      }
    }
    return {set, way};
  }
};

}  // end namespace midi2::adt

#endif  // MIDI2_ADT_PLRU_CACHE_HPP
