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

// Three Pivot Quicksort
//  a three pivot quicksort implementation switching to three way
//  partitioning if usefull

#ifndef SORT_DETAIL_INPLACE_H
#define SORT_DETAIL_INPLACE_H

#ifdef __INTEL_COMPILER
#pragma warning disable 3373
#endif

#include <array>
#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdint>
#include <tuple>
#include <type_traits>

#include "misc.h"

namespace sort {
namespace detail {
namespace inplace {

template <class T, class I, class V>
static std::pair<T, T> partition(T first, T last, I index, V pa) {
  using W = std::remove_reference_t<decltype(*first)>;
  // Doesn't assume that pa is in [first, last)
  // see "Engineering a Sort Function" - BENTLEY, McILROY

  auto a = first, c = std::prev(last);
  auto b = first, d = last;

  W bv, cv;
  for (V v; b <= c && (v = index(bv = *b)) <= pa; ++b)
    if (v == pa) *b = *a, *a++ = bv;

  for (V v; b < c && pa <= (v = index(cv = *c)); --c)
    if (v == pa) *c = *--d, *d = cv;

  // we now have a final guard on both ends
  if (b < c) do {
    *c-- = bv, *b++ = cv;

    for (V v; (v = index(bv = *b)) <= pa; ++b)
      if (v == pa) *b = *a, *a++ = bv;

    for (V v; pa <= (v = index(cv = *c)); --c)
      if (v == pa) *c = *--d, *d = cv;
  } while (b <= c);

  auto s = std::min(first + (b - a), a);
  std::swap_ranges(first, s, b - (s - first));

  auto t = std::min(last + (b - d), d);
  std::swap_ranges(b, t, last - (t - b));

  return std::make_pair(first + (b - a), last + (b - d));
}

template <class T, class I, class V>
static std::pair<T, T> exchange1(T a, T f, I index, V pa) {
  using W = std::remove_reference_t<decltype(*a)>;
  // Assumes pa to be at least the median of 3 elements in [a, f)
  // see "Engineering a Sort Function" - BENTLEY, McILROY

  auto b = a;
  while (b != f && index(*b) == pa) ++b;
  if (b == f) return std::make_pair(a, f);
  b -= a != b;  // we need at least one pivot element in the range

  auto c = b, d = f, e = f;
  while (true) {
    W cv, dv; V v1, v2;
    while ((v1 = index(cv = *c++)) < pa);
    while (pa < (v2 = index(dv = *--d)));

    if (--c >= d) break;

    if (v1 == pa) *d = *--e, *e = cv; else *d = cv;
    if (v2 == pa) *c = *b, *b++ = dv; else *c = dv;
    ++c;
  }
  d = c + (c == d);  // c == d means center element is equal to pa, keep it there

  auto s = std::min(a + (c - b), b);
  std::swap_ranges(a, s, c - (s - a));

  auto t = std::min(d + (f - e), e);
  std::swap_ranges(d, t, f - (t - d));

  return std::make_pair(a + (c - b), d + (f - e));
}

template <class T, class I, class V>
static std::tuple<T, T, T> exchange3(T first, T last, I index, V pa, V pb, V pc) {
  using W = std::remove_reference_t<decltype(*first)>;
  // Assumes pa < pb < pc and all exists within [first, last)
  // see "How Good is Multi-Pivot Quicksort?" - Aumueller, Dietzfelbinger, Klaue

  auto a = first, c = last - 1;
  auto b = first, d = last;

  while (true) {
    W bv, cv; V v1, v2;
    for (; !(pb < (v1 = index(bv = *b))); ++b)
      if (v1 < pa) *b = *a, *a++ = bv;

    for (; (pb < (v2 = index(cv = *c))); --c)
      if (v2 > pc) *c = *--d, *d = cv;

    if (b > c) break;

    if (v2 < pa) *b = *a, *a++ = cv; else *b = cv;
    if (v1 > pc) *c = *--d, *d = bv; else *c = bv;
    ++b; c--;
  }

  return std::make_tuple(a, b, d);
}

template <class T, class I, class V>
static T exchange_block(T first, T last, I index, V p) {
  //using W = std::remove_reference_t<decltype(*first)>;
  // Assumes p is at least median of three and exists within [first, last)
  // see "BlockQuicksort: How Branch Mispredictions don't affect Quicksort" - Edelkamp, Weiss

  auto a = first, b = last;

  constexpr const int BLOCK_SIZE = detail::misc::BLOCK_SIZE;
  std::array<uint8_t, BLOCK_SIZE> offsets_a;
  std::array<uint8_t, BLOCK_SIZE> offsets_b;
  intptr_t ac = 0, bc = 0;  // counts
  intptr_t au = 0, bu = 0;  // number of used
  while (2 * BLOCK_SIZE <= std::distance(a, b)) {
    auto t = ac;
    if (ac == 0) {
      au = 0;
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
      for (intptr_t i = 0; i < BLOCK_SIZE;) {
        offsets_a[ac] = i + 0; ac += p <= index(a[i + 0]);
        offsets_a[ac] = i + 1; ac += p <= index(a[i + 1]);
        offsets_a[ac] = i + 2; ac += p <= index(a[i + 2]);
        offsets_a[ac] = i + 3; ac += p <= index(a[i + 3]);
        i += 4;
      }
#else
      union { intptr_t a; uint8_t b; } t{0lu};
      for (intptr_t i = 0; i < BLOCK_SIZE;) {
        offsets_a[ac] = i; t.b = p <= index(a[i]); ac += t.a; ++i;
        offsets_a[ac] = i; t.b = p <= index(a[i]); ac += t.a; ++i;
        offsets_a[ac] = i; t.b = p <= index(a[i]); ac += t.a; ++i;
        offsets_a[ac] = i; t.b = p <= index(a[i]); ac += t.a; ++i;
      }
#endif
    }
    if (t != 0 || bc == 0) {
      bu = 0;
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
      for (intptr_t i = 0; i < BLOCK_SIZE;) {
        offsets_b[bc] = i + 0; bc += index(b[i + 0 - BLOCK_SIZE]) < p;
        offsets_b[bc] = i + 1; bc += index(b[i + 1 - BLOCK_SIZE]) < p;
        offsets_b[bc] = i + 2; bc += index(b[i + 2 - BLOCK_SIZE]) < p;
        offsets_b[bc] = i + 3; bc += index(b[i + 3 - BLOCK_SIZE]) < p;
        i += 4;
      }
#else
      union { intptr_t a; uint8_t b; } t{0lu};
      for (intptr_t i = 0; i < BLOCK_SIZE;) {
        offsets_b[bc] = i; t.b = index(b[i - BLOCK_SIZE]) < p; bc += t.a; ++i;
        offsets_b[bc] = i; t.b = index(b[i - BLOCK_SIZE]) < p; bc += t.a; ++i;
        offsets_b[bc] = i; t.b = index(b[i - BLOCK_SIZE]) < p; bc += t.a; ++i;
        offsets_b[bc] = i; t.b = index(b[i - BLOCK_SIZE]) < p; bc += t.a; ++i;
      }
#endif
    }

    auto c = std::min(ac, bc);
    for (decltype(c) i = 0; i < c; ++i)
      std::iter_swap(a + offsets_a[au + i], b + offsets_b[bu + i] - BLOCK_SIZE);
    ac -= c; bc -= c;
    au += c; bu += c;
    if (ac == 0) a += BLOCK_SIZE;
    if (bc == 0) b -= BLOCK_SIZE;
  }
  auto r = std::distance(a, b);
  decltype(r) bsa, bsb;  //block size a/b

  if ((ac | bc) == 0) {
    au = bu = 0;
    bsa = r >> 1; bsb = r - bsa;

    for (decltype(r) i = 0; i < bsa; ++i) {
      offsets_a[ac] = i; ac += p <= index(a[i]);
      offsets_b[bc] = i; bc += index(b[i - bsb]) < p;
    }
    // in case of odd r
    if (bsb > bsa) {
      offsets_b[bc] = static_cast<uint8_t>(bsa);
      bc += index(b[-1]) < p;
    }
  } else if (ac == 0) {
    au = 0;
    bsb = BLOCK_SIZE;
    for (decltype(r) i = 0; i < (bsa = r - BLOCK_SIZE); ++i) {
      offsets_a[ac] = i; ac += p <= index(a[i]);
    }
  } else {
    bu = 0;
    bsa = BLOCK_SIZE;
    for (decltype(r) i = 0; i < (bsb = r - BLOCK_SIZE); ++i) {
      offsets_b[bc] = i; bc += index(b[i - bsb]) < p;
    }
  }

  auto c = std::min(ac, bc);
  for (decltype(c) i = 0; i < c; ++i)
    std::iter_swap(a + offsets_a[au + i], b + offsets_b[bu + i] - bsb);
  ac -= c; bc -= c;
  au += c; bu += c;
  if (ac == 0) a += bsa;
  if (bc == 0) b -= bsb;

  if (ac) {
    while (ac--) std::iter_swap(a + offsets_a[au + ac], --b);
    return b;
  } else {
    for (decltype(c) i = 0; i < bc; ++i)
      std::iter_swap(a++, b + offsets_b[bu + i] - bsb);
    return a;
  }
}

template<class V, class T, class I>
static std::tuple<V, V, V> pivot(T first, T last, I index) {
  V a, b, c;
  if (std::distance(first, last) < detail::misc::MEDIAN21) {
    // Get 3 pivots using median of 7
    return detail::misc::median7<V>(first, index);
  } else if (std::distance(first, last) < detail::misc::MEDIAN65) {
    // Get 3 pivots using pseudo median of 21
    auto middle = first + std::distance(first, last) / 2;
    V a1, b1, c1; V a2, b2, c2; V a3, b3, c3;
    std::tie(a1, b1, c1) = detail::misc::median7<V>(first - 0, index);
    std::tie(a2, b2, c2) = detail::misc::median7<V>(middle - 4, index);
    std::tie(a3, b3, c3) = detail::misc::median7<V>(last - 8, index);
    a = detail::misc::median3(a1, a2, a3);
    b = detail::misc::median3(b1, b2, b3);
    c = detail::misc::median3(c1, c2, c3);
  } else {
    // Get 3 pivots using pseudo median of 65
    auto lower = first + std::distance(first, last) * 1 / 4;
    auto middle = first + std::distance(first, last) * 2 / 4;
    auto upper = first + std::distance(first, last) * 3 / 4;
    V a1, b1, c1; V a2, b2, c2; V a3, b3, c3; V a4, b4, c4; V a5, b5, c5;
    std::tie(a1, b1, c1) = detail::misc::median15<V>(first - 0, index);
    std::tie(a2, b2, c2) = detail::misc::median15<V>(lower - 8, index);
    std::tie(a3, b3, c3) = detail::misc::median15<V>(middle - 8, index);
    std::tie(a4, b4, c4) = detail::misc::median15<V>(upper - 8, index);
    std::tie(a5, b5, c5) = detail::misc::median15<V>(last - 16, index);
    a = detail::misc::median5(a1, a2, a3, a4, a5);
    b = detail::misc::median5(b1, b2, b3, b4, b5);
    c = detail::misc::median5(c1, c2, c3, c4, c5);
  }
  return std::make_tuple(a, b, c);
}

template <int LR, class T, class I, class C>
inline void insertion(T first, T last, I index, C cb) {
  // Insertion sort
  if (first != last) for (auto i = first + 1, j = i; i < last; ++i) {
    auto tmp = *i;
    auto val = index(tmp);
    for (j = i; j > first && val < index(j[-1]); --j)
      *j = j[-1];
    *j = tmp;
  }

  // Callbacks
  return detail::misc::call_range<LR>(first, last, index, cb);
}

template <int LR, int P, class T, class I, class C>
static void quick(T first, T last, I index, C &&cb, int budget) {
  using V = std::remove_reference_t<decltype(index(*first))>;

  while (1) {
    // Simple insertion sort on small groups
    if (std::distance(first, last) <= detail::misc::INSERTION_MAX)
      return detail::inplace::insertion<LR>(first, last, index, cb);

    // Switch to heap sort when quicksort degenerates
    if (budget-- == 0) {
      auto cmp = detail::misc::compare(index);
      std::make_heap(first, last, cmp);
      std::sort_heap(first, last, cmp);
      return detail::misc::call_range<LR>(first, last, index, cb);
    }

    V a, b, c;
    std::tie(a, b, c) = detail::inplace::pivot<V>(first, last, index);

    if (a == b || b == c) {
      // At least 3 out of 7 were equal to the pivot so switch
      // to three way quicksort
      T d, e;
      std::tie(d, e) = detail::inplace::exchange1(first, last, index, b);

      if (LR) {
        quick<LR, P>(first, d, index, cb, budget);
        if (LR != detail::misc::NOCB)
          cb(d, e);  // equal range callback - must exist
        first = e;  // tail recursion
      } else {
        quick<LR, P>(e, last, index, cb, budget);
        if (LR != detail::misc::NOCB)
          cb(d, e);  // equal range callback - must exist
        last = d;  // tail recursion
      }
    } else if (P == 0) {
      // Three pivot quicksort
      T d, e, f;
      std::tie(d, e, f) = detail::inplace::exchange3(first, last, index, a, b, c);

      if (LR) {
        quick<LR, P>(first, d, index, cb, budget);
        quick<LR, P>(d, e, index, cb, budget);
        quick<LR, P>(e, f, index, cb, budget);
        first = f;  // tail recursion
      } else {
        quick<LR, P>(f, last, index, cb, budget);
        quick<LR, P>(e, f, index, cb, budget);
        quick<LR, P>(d, e, index, cb, budget);
        last = d;  // tail recursion
      }
    } else {
      // block quicksort
      T d = detail::inplace::exchange_block(first, last, index, b);

      if (LR) {
        quick<LR, P>(first, d, index, cb, budget);
        first = d;  // tail recursion
      } else {
        quick<LR, P>(d, last, index, cb, budget);
        last = d;  // tail recursion
      }
    }
  }
}

}  // inplace
}  // detail
}  // sort

#endif  // SORT_DETAIL_INPLACE_H
