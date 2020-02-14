// Copyright (c) 2016 Christoph Diegelmann
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#ifndef SORT_DETAIL_MISC_H
#define SORT_DETAIL_MISC_H

#ifdef __INTEL_COMPILER
#pragma warning disable 3373
#endif

#include <algorithm>
#include <array>
#include <climits>
#include <cmath>
#include <cstdint>
#include <tuple>
#include <type_traits>

namespace sort {
namespace detail {
namespace misc {

constexpr const int NOCB = 2;  // Don't use the callback on equal ranges (implies LR)
constexpr const int LR   = 1;  // Direction: left to right
constexpr const int RL   = 0;  // Direction: right to left

constexpr const int INSERTION_MAX =   32;  // When to switch to insertion sort
constexpr const int MEDIAN21      =   64;  // When to switch to pseudo median of 21
constexpr const int MEDIAN65      = 8192;  // When to switch to pseudo median of 65
constexpr const int BLOCK_SIZE    =  128;  // Block Size for block partition ~2 cache lines
constexpr const int COPY_MIN      = 1024;  // Minimum number of elements to use copy
                                           // probably around number of cache lines in L1 cache * 2

template<class T1, class T2>
struct pair {
  constexpr pair() : first(), second() {};

  template<class A, class B>
  constexpr pair(A&& a, B&& b) :
    first(std::forward<A>(a)),
    second(std::forward<B>(b)) {}

  pair& operator^=(const pair& rhs) {
    this->first ^= rhs.first;
    this->second ^= rhs.second;
    return *this;
  }

  T1 first;
  T2 second;
};

template<class T1, class T2>
inline pair<T1, T2> operator^(pair<T1, T2> lhs, const pair<T1, T2>& rhs) {
  lhs ^= rhs;
  return lhs;
}

template<class T1, class T2>
constexpr auto make_pair(T1&& a, T2&& b)->detail::misc::pair<
  std::remove_reference_t<T1>,
  std::remove_reference_t<T2>> {
  return detail::misc::pair<
    std::remove_reference_t<T1>,
    std::remove_reference_t<T2>
  >(std::forward<T1>(a), std::forward<T2>(b));
}

template <typename, typename = void>
struct has_xor_operator
  : std::false_type {};

template <typename T>
struct has_xor_operator<T, decltype(void(std::declval<T>() ^ std::declval<T>()))>
  : std::true_type {};

template <class T> int ilogb(T v) {
#if defined(__GNUC__)
  return (31 - __builtin_clz(v));
#else
  int r = 0;
  while (v >>= 1)
    ++r;
  return r;
#endif
}

template <class V> inline void cswap(V &a, V &b) {
#if defined(__GNUC__) || defined(_MSC_VER) && !defined(__clang__) && !defined(__INTEL_COMPILER)
  if (has_xor_operator<V>::value) {
    std::remove_reference_t<V> da = a, db = b, tmp;
    tmp = a = da < db ? da : db;
    b ^= da ^ tmp;
  } else {
    if (b < a) std::swap(a, b);
  }
#else
  // clang+icc nicely optimize this into cmoves
  if (b < a) std::swap(a, b);
#endif
}

template <class V> inline void cmovl(V &a, V b) {
  if (b < a) a = b;
}

template <class V> inline void cmovg(V a, V &b) {
  if (b < a) b = a;
}

template <class V> inline V median3(V a, V b, V c) {
  // Get median by sorting 3 elements and return the middle element
  // using a sorting network

  cswap(a, c); cmovg(a, b);
  cmovl(b, c);

  return b;
}

template <class V> inline V median5(V a, V b, V c, V d, V e) {
  // Get median by sorting 5 elements and return the middle element
  // using a sorting network

  cswap(a, b); cswap(d, e);
  cswap(c, e); cswap(c, d);
  cswap(a, d); cmovg(a, c);
  cmovl(b, e); cmovl(b, d);
  cmovg(b, c);

  return c;
}

template <class V, class T, class I>
inline std::tuple<V, V, V> median7(T first, I index) {
  // Get 3 pivots by sorting 7 elements and returning element 1, 3 and 5
  // using a sorting network

  V a = index(first[0]);
  V c = index(first[2]);
  V e = index(first[4]);
  V g = index(first[6]);

  cswap(c, g); cswap(a, e);
  V b = index(first[1]);
  V f = index(first[5]);
  cswap(b, f); cswap(e, g);
  cswap(a, c);
  V d = index(first[3]);
  cswap(b, d); cswap(c, e);
  cswap(d, f); cmovg(a, b);
  cswap(e, f); cswap(c, d);
  cswap(b, e); cswap(d, g);
  cmovl(b, c); cmovl(d, e);
  cmovl(f, g);

  return std::make_tuple(b, d, f);
}

template <class V, class T, class I>
inline std::tuple<V, int> median7_copy(T first, I index) {
  // Get 3 pivots by sorting 7 elements and returning element 1, 3 and 5
  // using a sorting network and return the number of equal elements

  V a = index(first[0]);
  V c = index(first[2]);
  V e = index(first[4]);
  V g = index(first[6]);

  cswap(c, g); cswap(a, e);
  V b = index(first[1]);
  V f = index(first[5]);
  cswap(b, f); cswap(e, g);
  cswap(a, c);
  V d = index(first[3]);
  cswap(b, d); cswap(c, e);
  cswap(d, f); cmovg(a, b);
  cswap(e, f); cswap(c, d);
  cswap(b, e); cswap(d, g);
  cswap(b, c); cswap(d, e);
  cswap(f, g);

  return std::make_tuple(d, (a == b) + (b == c) + (c == d) + (d == e) + (e == f) + (f == g));
}

template <class V, class T, class I>
static std::tuple<V, V, V> median15(T first, I index) {
  // Get 3 pivots by sorting 15 elements and returning element 4, 8 and 12

  V a = index(first[0]);
  V b = index(first[1]);
  V c = index(first[2]);
  V d = index(first[3]);
  cswap(a, b); cswap(c, d);
  V e = index(first[4]);
  V f = index(first[5]);
  V g = index(first[6]);
  V h = index(first[7]);
  cswap(e, f); cswap(g, h);
  V i = index(first[8]);
  V j = index(first[9]);
  V k = index(first[10]);
  V l = index(first[11]);
  cswap(i, j); cswap(k, l);
  V m = index(first[12]);
  V n = index(first[13]);
  cswap(m, n); cswap(a, c);
  cswap(e, g); cswap(i, k);
  V o = index(first[14]);
  cswap(m, o); cswap(b, d);
  cswap(f, h); cswap(j, l);
  cswap(a, e); cswap(i, m);
  cswap(b, f); cswap(j, n);
  cswap(c, g); cswap(k, o);
  cswap(d, h); cmovg(a, i);
  cswap(b, j); cswap(c, k);
  cswap(d, l); cswap(e, m);
  cswap(f, n); cswap(g, o);
  cswap(f, k); cswap(g, j);
  cswap(d, m); cswap(n, o);
  cswap(h, l); cswap(b, c);
  cswap(e, i); cmovg(b, e);
  cswap(h, n); cswap(c, i);
  cmovl(l, o); cmovg(c, e);
  cswap(f, g); cswap(j, k);
  cmovl(l, n); cswap(d, i);
  cswap(h, m); cswap(g, i);
  cmovg(k, m); cswap(d, f);
  cmovl(h, j); cmovl(d, e);
  cmovg(f, g); cmovl(h, i);
  cmovl(l, m); cmovg(g, h);

  return std::make_tuple(d, h, l);
}

// I refactored this out to always give them the same name
template <class I> inline auto compare(I index) {
  return [index](auto a, auto b) { return index(a) < index(b); };
}

template <int LR, class T, class I, class C>
inline void call_range(T first, T last, I index, C cb) {
  // Callbacks in LR or RL
  if (LR != detail::misc::NOCB) {
    if (LR) for (auto itf = first; itf < last;) {
      auto itl = itf + 1;
      while (itl < last && index(*itf) == index(*itl)) ++itl;
      cb(itf, itl);
      itf = itl;
    } else for (auto itl = last; itl > first;) {
      auto itf = itl - 1;
      while (itf > first && index(itf[-1]) == index(itl[-1])) --itf;
      cb(itf, itl);
      itl = itf;
    }
  }
}

// I refactored this out to always give it the same name
// This may result in better code sharing
template <class U, class D>
inline auto index(U ISA, D depth) {
  // We don't have to check if a + depth > n because
  // all groups leading up to n are always already unique
  return [ISAd = ISA + depth](auto a) { return ISAd[a]; };
}

template <class A>
auto castTo() {
  return [](auto val) { return static_cast<std::remove_reference_t<A>>(val); };
}

}  // misc
}  // detail
}  // sort

#endif  // SORT_DETAIL_MISC_H
