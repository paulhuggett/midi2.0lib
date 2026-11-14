//===-- UMP As Array ----------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file ump_as_array.hpp
/// \brief UMP array type helpers
///
/// Some of the UMP types can be treated as containing arrays: specifically the four data64::sysex7 messages, the four
/// data128::sysex8 messages, and the flex_data::text_common message. The file contains helper types which those
/// message classes use to implement this behaviour.

#ifndef MIDI2_UMP_AS_ARRAY_HPP
#define MIDI2_UMP_AS_ARRAY_HPP

#include <algorithm>
#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <tuple>
#include <type_traits>

#include "midi2/adt/uinteger.hpp"

namespace midi2::ump::details {

/// \brief Satisfied by types whose elements can be read (and, unless the type is const, written) by index.
///
/// The type must provide a value_type member type and a data(index) member function which returns the element at
/// index. A non-const type must also provide data(index, value) which sets that element.
template <typename T>
concept accessible_as_array = requires(T const ct, std::size_t i) {
  typename T::value_type;
  { ct.data(i) } -> std::convertible_to<typename T::value_type>;
} && (std::is_const_v<T> || requires(T t, std::size_t i, typename T::value_type v) {
                                { t.data(i, v) };
                              });

/// \brief A proxy for an element of a message that is accessible as an array.
///
/// Reading the proxy (by conversion to value_type) calls the owner's data(index) member function; assigning to it calls
/// data(index, value). Like std::vector<bool>::reference, assignment writes through to the referenced element: it never
/// changes the element to which the proxy refers, even when the right-hand side is another proxy.
///
/// \tparam T  The message type, possibly const-qualified.
template <accessible_as_array T> class array_subscript_proxy {
public:
  /// \brief The type of an element.
  using value_type = typename T::value_type;

  /// \brief Constructs a proxy which does not refer to any element.
  constexpr array_subscript_proxy() noexcept = default;
  /// \brief Constructs a proxy for an element of a message.
  /// \param owner  The message containing the element.
  /// \param index  The index of the element.
  constexpr array_subscript_proxy(T& owner, std::size_t const index) noexcept : owner_{&owner}, index_{index} {}
  /// \brief Copy constructor. The new proxy refers to the same element as \p other.
  /// \param other  The proxy to be copied.
  constexpr array_subscript_proxy(array_subscript_proxy const& other) noexcept = default;
  /// \brief Destructor.
  constexpr ~array_subscript_proxy() noexcept = default;
  /// \brief Compares two proxies for equality.
  /// \tparam U  The message type of \p rhs.
  /// \param lhs  The first proxy to be compared.
  /// \param rhs  The second proxy to be compared.
  /// \returns True if the two proxies refer to the same element of the same message.
  template <typename U>
    requires(std::is_same_v<U, std::remove_const_t<T>> || std::is_same_v<U, T const>)
  friend constexpr bool operator==(array_subscript_proxy const& lhs, array_subscript_proxy<U> const& rhs) {
    return lhs.owner() == rhs.owner() && lhs.index() == rhs.index();
  }
  /// \brief Three-way comparison of two proxies.
  /// \tparam U  The message type of \p rhs.
  /// \param lhs  The first proxy to be compared.
  /// \param rhs  The second proxy to be compared.
  /// \returns The ordering of the (owner, index) pairs of the two proxies.
  template <typename U>
    requires(std::is_same_v<U, std::remove_const_t<T>> || std::is_same_v<U, T const>)
  friend constexpr std::partial_ordering operator<=>(array_subscript_proxy const& lhs,
                                                     array_subscript_proxy<U> const& rhs) noexcept {
    return std::tuple{lhs.owner(), lhs.index()} <=> std::tuple{rhs.owner(), rhs.index()};
  }
  /// \brief Sets the value of the referenced element.
  /// \param v  The new value of the element.
  /// \returns *this
  constexpr array_subscript_proxy const& operator=(value_type const v) const noexcept
    requires(!std::is_const_v<T>)
  {
    owner_->data(index_, v);
    return *this;
  }
  /// \brief Sets the value of the referenced element to the value of the element referenced by \p other.
  /// \param other  A proxy for the element whose value is to be copied.
  /// \returns *this
  constexpr array_subscript_proxy& operator=(array_subscript_proxy const& other) noexcept
    requires(!std::is_const_v<T>)
  {
    if (this == &other) {
      return *this;
    }
    owner_->data(index_, static_cast<value_type>(other));
    return *this;
  }
  /// \brief Returns the value of the referenced element.
  /// \returns The value of the referenced element.
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
  constexpr operator value_type() const noexcept { return owner_->data(index_); }

  /// \brief Moves the proxy to refer to the next element.
  /// \returns *this
  constexpr array_subscript_proxy& operator++() noexcept {
    ++index_;
    return *this;
  }
  /// \brief Moves the proxy to refer to the next element.
  /// \returns A copy of the proxy before it was moved.
  constexpr array_subscript_proxy operator++(int) noexcept {
    auto const result = *this;
    ++*this;
    return result;
  }
  /// \brief Moves the proxy to refer to the previous element.
  /// \returns *this
  constexpr array_subscript_proxy& operator--() noexcept {
    assert(index_ > 0 && "Index out of range");
    --index_;
    return *this;
  }
  /// \brief Moves the proxy to refer to the previous element.
  /// \returns A copy of the proxy before it was moved.
  constexpr array_subscript_proxy operator--(int) noexcept {
    auto const result = *this;
    --*this;
    return result;
  }

  /// \brief Moves the proxy forwards by \p n elements.
  /// \tparam U  An integral type.
  /// \param n  The number of elements by which the proxy is moved.
  /// \returns *this
  template <std::integral U> constexpr auto& operator+=(U const n) noexcept {
    index_ += n;
    return *this;
  }
  /// \brief Moves the proxy backwards by \p n elements.
  /// \tparam U  An integral type.
  /// \param n  The number of elements by which the proxy is moved. Must not exceed the current index.
  /// \returns *this
  template <std::integral U> constexpr auto& operator-=(U const n) noexcept {
    assert(n <= index_ && "Index out of range");
    index_ -= n;
    return *this;
  }

  /// \brief Returns the message which contains the referenced element.
  /// \returns A pointer to the message which contains the referenced element.
  [[nodiscard]] constexpr T const* owner() const noexcept { return owner_; }
  /// \brief Returns the message which contains the referenced element.
  /// \returns A pointer to the message which contains the referenced element.
  [[nodiscard]] constexpr T* owner() noexcept
    requires(!std::is_const_v<T>)
  {
    return owner_;
  }
  /// \brief Returns the index of the referenced element.
  /// \returns The index of the referenced element.
  [[nodiscard]] constexpr std::size_t index() const noexcept { return index_; }

private:
  T* owner_ = nullptr;
  std::size_t index_ = 0;
};

/// \brief A random-access iterator over the elements of a message that is accessible as an array.
///
/// Dereferencing the iterator yields an array_subscript_proxy<> for the element. Assigning to that proxy
/// writes to the element.
///
/// \tparam T  The message type, possibly const-qualified.
template <typename T> class iterator_base {
public:
  /// Defines this class as fulfilling the requirements of a random-access iterator.
  using iterator_category = std::random_access_iterator_tag;
  /// The type accessible through this iterator.
  using value_type = array_subscript_proxy<T>;
  /// A type that can be used to identify distance between iterators.
  using difference_type = std::ptrdiff_t;
  /// The iterator does not provide operator->.
  using pointer = void;
  /// The type yielded by dereferencing the iterator: a proxy for the element, returned by value.
  using reference = value_type;

  /// \brief Constructs an iterator which does not refer to any message.
  constexpr iterator_base() noexcept = default;
  /// \brief Constructs an iterator which refers to an element of a message.
  /// \param owner  The message.
  /// \param index  The index of the element.
  constexpr iterator_base(T& owner, std::size_t const index) noexcept : owner_{&owner}, index_{index} {}

  /// \brief Assigns from an iterator with a different const qualification.
  /// \tparam U  The message type of \p other. A non-const iterator may be assigned to a const iterator but not the
  ///   other way round.
  /// \param other  The iterator to be copied.
  /// \returns *this
  template <typename U>
    requires(std::is_same_v<std::remove_const_t<T>, std::remove_const_t<U>> && !std::is_same_v<T, U> &&
             std::is_const_v<T>)
  constexpr iterator_base& operator=(iterator_base<U> const& other) noexcept {
    owner_ = other.owner_;
    index_ = other.index_;
    return *this;
  }

  /// \brief Compares two iterators for equality.
  /// \tparam U  The message type of \p rhs.
  /// \param lhs  The first iterator to be compared.
  /// \param rhs  The second iterator to be compared.
  /// \returns True if the two iterators refer to the same element of the same message.
  template <typename U>
    requires(std::is_same_v<std::remove_const_t<T>, std::remove_const_t<U>>)
  friend constexpr bool operator==(iterator_base const& lhs, iterator_base<U> const& rhs) noexcept {
    return lhs.owner_ == rhs.owner_ && lhs.index_ == rhs.index_;
  }

  /// \brief Three-way comparison of two iterators.
  /// \tparam U  The message type of \p rhs.
  /// \param lhs  The first iterator to be compared.
  /// \param rhs  The second iterator to be compared.
  /// \returns The ordering of the (owner, index) pairs of the two iterators.
  template <typename U>
    requires(std::is_same_v<std::remove_const_t<T>, std::remove_const_t<U>>)
  friend constexpr std::strong_ordering operator<=>(iterator_base const& lhs, iterator_base<U> const& rhs) noexcept {
    if (auto const c = std::compare_three_way{}(lhs.owner_, rhs.owner_); std::is_neq(c)) {
      return c;
    }
    return lhs.index_ <=> rhs.index_;
  }

  /// \brief Returns a proxy for the referenced element.
  /// \returns A proxy for the referenced element.
  constexpr reference operator*() const noexcept {
    assert(owner_ != nullptr && "Cannot dereference an iterator which does not refer to a message");
    return reference{*owner_, index_};
  }
  /// \brief Returns a proxy for the element \p n positions after the referenced element.
  /// \param n  The offset of the element.
  /// \returns A proxy for the element.
  constexpr reference operator[](difference_type const n) const noexcept { return *(*this + n); }

  /// \brief Moves the iterator to the next element.
  /// \returns *this
  constexpr iterator_base& operator++() noexcept {
    ++index_;
    return *this;
  }
  /// \brief Moves the iterator to the next element.
  /// \returns A copy of the iterator before it was moved.
  constexpr iterator_base operator++(int) noexcept {
    auto const prev = *this;
    ++*this;
    return prev;
  }
  /// \brief Moves the iterator to the previous element.
  /// \returns *this
  constexpr iterator_base& operator--() noexcept {
    assert(index_ > 0 && "Index out of range");
    --index_;
    return *this;
  }
  /// \brief Moves the iterator to the previous element.
  /// \returns A copy of the iterator before it was moved.
  constexpr iterator_base operator--(int) noexcept {
    auto const prev = *this;
    --*this;
    return prev;
  }

  /// \brief Moves the iterator forwards by \p n elements.
  /// \tparam U  An integral type.
  /// \param n  The number of elements by which the iterator is moved. May be negative.
  /// \returns *this
  template <std::integral U> constexpr iterator_base& operator+=(U const n) noexcept {
    auto const index = static_cast<difference_type>(index_) + static_cast<difference_type>(n);
    assert(index >= 0 && "Index out of range");
    index_ = static_cast<std::size_t>(index);
    return *this;
  }
  /// \brief Moves the iterator backwards by \p n elements.
  /// \tparam U  An integral type.
  /// \param n  The number of elements by which the iterator is moved. May be negative.
  /// \returns *this
  template <std::integral U> constexpr iterator_base& operator-=(U const n) noexcept {
    return *this += -static_cast<difference_type>(n);
  }

  /// Returns the distance between two iterators \p b - \p a.
  ///
  /// \param b  The first iterator.
  /// \param a  The second iterator.
  /// \returns  distance between two iterators \p b - \p a.
  friend constexpr difference_type operator-(iterator_base const& b, iterator_base const& a) noexcept {
    assert(a.owner_ == b.owner_ && "Cannot get the distance between iterators with different owners");
    return static_cast<difference_type>(b.index_) - static_cast<difference_type>(a.index_);
  }

  /// Move an iterator \p it forwards by distance \p n. \p n can be both positive
  /// or negative.
  /// \tparam U  An integral type.
  /// \param it  The iterator to be moved.
  /// \param n  The distance by which iterator \p it should be moved.
  /// \returns  The new iterator.
  template <std::integral U> friend constexpr iterator_base operator+(iterator_base const it, U const n) noexcept {
    auto t = it;
    return t += n;
  }
  /// Move an iterator \p it forwards by distance \p n. \p n can be both positive
  /// or negative.
  /// \tparam U  An integral type.
  /// \param n  The distance by which iterator \p it should be moved.
  /// \param it  The iterator to be moved.
  /// \returns  The new iterator.
  template <std::integral U> friend constexpr iterator_base operator+(U const n, iterator_base const it) noexcept {
    return it + n;
  }

  /// Move an iterator \p it backwards by distance \p n. \p n can be both positive
  /// or negative.
  /// \tparam U  An integral type.
  /// \param it  The iterator to be moved.
  /// \param n  The distance by which iterator \p it should be moved.
  /// \returns  The new iterator.
  template <std::integral U> friend constexpr iterator_base operator-(iterator_base const it, U const n) noexcept {
    auto t = it;
    return t -= n;
  }

private:
  template <typename U> friend class iterator_base;

  T* owner_ = nullptr;
  std::size_t index_ = 0;
};

/// \brief Implements the data() member functions which set the elements of a message that is accessible as an array.
///
/// \tparam Message  The message type.
/// \tparam Bits  The number of bits in each element.
template <typename Message, unsigned Bits> struct data_helper {
  /// \brief The type of an element.
  using value_type = typename Message::value_type;

  /// \brief Sets the elements of a message from an initializer list.
  /// \tparam T  An integral type.
  /// \param message  The message to be modified.
  /// \param vs  The values to be stored. Values beyond the capacity of the message are ignored.
  /// \returns The number of elements stored.
  template <std::integral T> static constexpr std::size_t data(Message& message, std::initializer_list<T> vs) {
    assert(vs.size() <= message.max_size() && "initializer list has too many members");
    auto index = std::size_t{0};
    for (auto const& v : vs) {
      if (index >= message.max_size()) {
        break;
      }
      assert(v >= 0 && v < (1 << Bits) && "data value is out of range");
      message[index] = static_cast<::midi2::adt::uinteger_t<Bits>>(v);
      ++index;
    }
    return index;
  }
  /// \brief Sets the elements of a message from a range.
  /// \tparam Range  An input range type with integral elements.
  /// \tparam Proj  The type of the projection.
  /// \param message  The message to be modified.
  /// \param range  The values to be stored. Values beyond the capacity of the message are ignored.
  /// \param proj  The projection to apply to each element of \p range.
  /// \returns The number of elements stored.
  template <std::ranges::input_range Range, typename Proj = std::identity>
    requires std::is_integral_v<std::remove_cvref_t<std::ranges::range_value_t<Range>>>
  static constexpr std::size_t data(Message& message, Range&& range, Proj proj = {}) {
    auto index = std::size_t{0};
    std::ranges::for_each(
        std::forward<Range>(range),
        [&message, &index](auto const& v) constexpr {
          if (index < message.max_size()) {
            assert(v >= 0 && v < (1 << Bits) && "data value is out of range");
            message[index] = static_cast<::midi2::adt::uinteger_t<Bits>>(v);
            ++index;
          }
        },
        proj);
    return index;
  }
  /// \brief Sets the elements of a message from an iterator range.
  /// \tparam I  An input iterator type with integral elements.
  /// \tparam S  A sentinel type for \p I.
  /// \param message  The message to be modified.
  /// \param first  The beginning of the range of values to be stored.
  /// \param last  The end of the range of values to be stored. Values beyond the capacity of the message are ignored.
  /// \returns The number of elements stored.
  template <std::input_iterator I, std::sentinel_for<I> S>
    requires std::is_integral_v<std::remove_cvref_t<typename std::iterator_traits<I>::value_type>>
  static constexpr std::size_t data(Message& message, I first, S last) {
    auto index = std::size_t{0};
    std::for_each(first, last, [&message, &index](auto const& v) constexpr {
      if (index < message.max_size()) {
        assert(v >= 0 && v < (1 << Bits) && "data value is out of range");
        message[index] = static_cast<::midi2::adt::uinteger_t<Bits>>(v);
        ++index;
      }
    });
    return index;
  }
};

}  // namespace midi2::ump::details

#endif  // MIDI2_UMP_AS_ARRAY_HPP
