
// The MIT License (MIT)
//
// Copyright (c) 2018 Mateusz Pusz
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <units/kind.h>
#include <units/quantity.h>
#include <concepts>
#include <type_traits>
#include <utility>

namespace units {

namespace detail {

template<Kind K>
inline constexpr auto make_quantity_kind_fn = [](auto q) {
  using Q = decltype(q);
  return quantity_kind<K, typename Q::unit, typename Q::rep>(std::move(q));
};

template<QuantityKind QK>
inline constexpr auto& make_quantity_kind = make_quantity_kind_fn<typename QK::kind_type>;

template<Kind K>
inline constexpr auto downcasted_kind_fn = [](auto q) {
  using Q = decltype(q);
  return make_quantity_kind_fn<downcast_kind<K, typename Q::dimension>>(std::move(q));
};

template<QuantityKind QK>
inline constexpr auto& downcasted_kind = downcasted_kind_fn<typename QK::kind_type::base_kind>;

}  // namespace detail

/**
 * @brief A quantity kind
 *
 * A quantity with more specific usage as determined by its kind.
 * See https://jcgm.bipm.org/vim/en/1.2.html and NOTE 1 at https://jcgm.bipm.org/vim/en/1.1.html.
 *
 * @tparam K the kind of quantity
 * @tparam U the measurement unit of the quantity kind
 * @tparam Rep the type to be used to represent values of the quantity kind
 */
template<Kind K, UnitOf<typename K::dimension> U, QuantityValue Rep = double>
class quantity_kind {
public:
  using kind_type = K;
  using quantity_type = quantity<typename K::dimension, U, Rep>;
  using dimension = typename quantity_type::dimension;
  using unit = typename quantity_type::unit;
  using rep = typename quantity_type::rep;

private:
  quantity_type q_;

public:
  quantity_kind() = default;
  quantity_kind(const quantity_kind&) = default;
  quantity_kind(quantity_kind&&) = default;

  template<safe_convertible_to_<rep> Value>
    requires is_same_v<dimension, dim_one> && std::is_constructible_v<quantity_type, Value>
  constexpr explicit quantity_kind(const Value& v) : q_(v) {}

  template<typename Q>
    requires (Quantity<Q> || QuantityLike<Q>) && std::is_constructible_v<quantity_type, Q>
  constexpr explicit quantity_kind(const Q& q) : q_{q} {}

  template<QuantityKindEquivalentTo<quantity_kind> QK2>
    requires std::is_convertible_v<typename QK2::quantity_type, quantity_type>
  constexpr explicit(false) quantity_kind(const QK2& qk) : q_{qk.common()} {}

  quantity_kind& operator=(const quantity_kind&) = default;
  quantity_kind& operator=(quantity_kind&&) = default;

  [[nodiscard]] constexpr quantity_type common() const noexcept { return q_; }

  [[nodiscard]] static constexpr quantity_kind zero() noexcept
    requires requires { quantity_type::zero(); }
  {
    return quantity_kind(quantity_type::zero());
  }

  [[nodiscard]] static constexpr quantity_kind one() noexcept
    requires requires { quantity_type::one(); }
  {
    return quantity_kind(quantity_type::one());
  }

  [[nodiscard]] static constexpr quantity_kind min() noexcept
    requires requires { quantity_type::min(); }
  {
    return quantity_kind(quantity_type::min());
  }

  [[nodiscard]] static constexpr quantity_kind max() noexcept
    requires requires { quantity_type::max(); }
  {
    return quantity_kind(quantity_type::max());
  }

  [[nodiscard]] constexpr quantity_kind operator+() const
    requires requires(quantity_type q) { +q; }
  {
    return *this;
  }

  [[nodiscard]] constexpr QuantityKind auto operator-() const
    requires requires(quantity_type q) { -q; }
  {
    return detail::make_quantity_kind<quantity_kind>(-q_);
  }

  constexpr quantity_kind& operator++()
    requires requires(quantity_type q) { ++q; }
  {
    ++q_;
    return *this;
  }

  [[nodiscard]] constexpr quantity_kind operator++(int)
    requires requires(quantity_type q) { q++; }
  {
    return quantity_kind(q_++);
  }

  constexpr quantity_kind& operator--()
    requires requires(quantity_type q) { --q; }
  {
    --q_;
    return *this;
  }

  [[nodiscard]] constexpr quantity_kind operator--(int)
    requires requires(quantity_type q) { q--; }
  {
    return quantity_kind(q_--);
  }

  constexpr quantity_kind& operator+=(const quantity_kind& qk)
    requires requires(quantity_type q) { q += qk.common(); }
  {
    q_ += qk.common();
    return *this;
  }

  constexpr quantity_kind& operator-=(const quantity_kind& qk)
    requires requires(quantity_type q) { q -= qk.common(); }
  {
    q_ -= qk.common();
    return *this;
  }

  template<typename Rep2>
  constexpr quantity_kind& operator*=(const Rep2& rhs)
    requires requires(quantity_type q) { q *= rhs; }
  {
    q_ *= rhs;
    return *this;
  }

  template<typename Rep2>
  constexpr quantity_kind& operator/=(const Rep2& rhs)
    requires requires(quantity_type q) { q /= rhs; }
  {
    q_ /= rhs;
    return *this;
  }

  template<typename Rep2>
  constexpr quantity_kind& operator%=(const Rep2& rhs)
    requires (!Quantity<Rep2>) && requires(quantity_type q, const Rep2 r) { q %= r; }
  {
    q_ %= rhs;
    return *this;
  }

  constexpr quantity_kind& operator%=(const quantity_kind& qk)
    requires requires(quantity_type q) { q %= qk.common(); }
  {
    q_ %= qk.common();
    return *this;
  }

  // Hidden Friends
  // Below friend functions are to be found via argument-dependent lookup only
  template<QuantityValue Value>
  [[nodiscard]] friend constexpr QuantityKind auto operator*(const quantity_kind& qk, const Value& v)
    requires requires { { qk.common() * v } -> Quantity; }
  {
    return detail::make_quantity_kind<quantity_kind>(qk.common() * v);
  }

  template<QuantityValue Value>
  [[nodiscard]] friend constexpr QuantityKind auto operator*(const Value& v, const quantity_kind& qk)
    requires requires { { v * qk.common() } -> Quantity; }
  {
    return detail::make_quantity_kind<quantity_kind>(v * qk.common());
  }

  template<QuantityValue Value>
  [[nodiscard]] friend constexpr QuantityKind auto operator/(const quantity_kind& qk, const Value& v)
    requires requires { { qk.common() / v } -> Quantity; }
  {
    return detail::make_quantity_kind<quantity_kind>(qk.common() / v);
  }

  template<QuantityValue Value>
  [[nodiscard]] friend constexpr QuantityKind auto operator/(const Value& v, const quantity_kind& qk)
    requires requires { { v / qk.common() } -> Quantity; }
  {
    return detail::downcasted_kind<quantity_kind>(v / qk.common());
  }

  template<QuantityValue Value>
  [[nodiscard]] friend constexpr QuantityKind auto operator%(const quantity_kind& qk, const Value& v)
    requires requires { qk.common() % v; }
  {
    return detail::make_quantity_kind<quantity_kind>(qk.common() % v);
  }

  [[nodiscard]] friend constexpr auto operator<=>(const quantity_kind& lhs, const quantity_kind& rhs)
    requires std::three_way_comparable<quantity_type>
  {
    return lhs.common() <=> rhs.common();
  }

  [[nodiscard]] friend constexpr bool operator==(const quantity_kind&, const quantity_kind&) = default;
};

template<Kind K, QuantityValue V>
explicit(false) quantity_kind(K, V) -> quantity_kind<K, one, V>;

template<QuantityKind QK1, QuantityKindEquivalentTo<QK1> QK2>
[[nodiscard]] constexpr QuantityKind auto operator+(const QK1& lhs, const QK2& rhs)
  requires requires { lhs.common() + rhs.common(); }
{
  return detail::make_quantity_kind<QK1>(lhs.common() + rhs.common());
}

template<QuantityKind QK1, QuantityKindEquivalentTo<QK1> QK2>
[[nodiscard]] constexpr QuantityKind auto operator-(const QK1& lhs, const QK2& rhs)
    requires requires { lhs.common() - rhs.common(); }
{
  return detail::make_quantity_kind<QK1>(lhs.common() - rhs.common());
}

template<QuantityKind QK, Quantity Q>
[[nodiscard]] constexpr QuantityKind auto operator*(const QK& lhs, const Q& rhs)
  requires requires { lhs.common() * rhs; }
{
  return detail::downcasted_kind<QK>(lhs.common() * rhs);
}

template<Quantity Q, QuantityKind QK>
[[nodiscard]] constexpr QuantityKind auto operator*(const Q& lhs, const QK& rhs)
  requires requires { lhs * rhs.common(); }
{
  return detail::downcasted_kind<QK>(lhs * rhs.common());
}

template<QuantityKind QK, Quantity Q>
[[nodiscard]] constexpr QuantityKind auto operator/(const QK& lhs, const Q& rhs)
  requires requires { lhs.common() / rhs; }
{
  return detail::downcasted_kind<QK>(lhs.common() / rhs);
}

template<Quantity Q, QuantityKind QK>
[[nodiscard]] constexpr QuantityKind auto operator/(const Q& lhs, const QK& rhs)
  requires requires { lhs / rhs.common(); }
{
  return detail::downcasted_kind<QK>(lhs / rhs.common());
}

template<QuantityKind QK, Dimensionless D>
[[nodiscard]] constexpr QuantityKind auto operator%(const QK& lhs, const D& rhs)
  requires requires { lhs.common() % rhs; }
{
  return detail::make_quantity_kind<QK>(lhs.common() % rhs);
}

template<QuantityKind QK1, QuantityKindEquivalentTo<QK1> QK2>
[[nodiscard]] constexpr QuantityKind auto operator%(const QK1& lhs, const QK2& rhs)
  requires requires { lhs.common() % rhs.common(); }
{
  return detail::make_quantity_kind<QK1>(lhs.common() % rhs.common());
}

template<QuantityKind QK1, QuantityKindEquivalentTo<QK1> QK2>
  requires std::three_way_comparable_with<typename QK1::quantity_type, typename QK2::quantity_type>
[[nodiscard]] constexpr auto operator<=>(const QK1& lhs, const QK2& rhs)
{
  return lhs.common() <=> rhs.common();
}

template<QuantityKind QK1, QuantityKindEquivalentTo<QK1> QK2>
  requires std::equality_comparable_with<typename QK1::quantity_type, typename QK2::quantity_type>
[[nodiscard]] constexpr bool operator==(const QK1& lhs, const QK2& rhs)
{
  return lhs.common() == rhs.common();
}

// type traits
namespace detail {

template<typename K, typename U, typename Rep>
inline constexpr bool is_quantity_kind<quantity_kind<K, U, Rep>> = true;

}  // namespace detail

}  // namespace units