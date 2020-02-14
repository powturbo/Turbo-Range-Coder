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

// Linear Time Suffix Sorting By Depth Awareness

#ifndef SORT_DETAIL_SUFFIX_H
#define SORT_DETAIL_SUFFIX_H

#ifdef __INTEL_COMPILER
#pragma warning disable 3373
#endif

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <vector>

#include "misc.h"
#include "../inplace.h"
#include "../copy.h"

// define if additional space may be used
#define USE_COPY

namespace sort {
namespace detail {
namespace suffix {

template <class T, class U, class D, class G>
inline T induce(T SA, U ISA, T a, T b, T e, T f, D depth, G group) {
  if (b == e) {
    if (a != b) b[-1] = ~b[-1];
    return b;
  }

  // [a, b) = lower range          = group
  // [b, e) = range to be induced  = group
  // [e, f) = high range           > group
  T c = b, d = e;
  auto index = detail::misc::index(ISA, depth);
  auto castToIndex = detail::misc::castTo<decltype(*ISA)>();

  // Induce upper part
  while (e != f) {
    for (auto it = f; it != e; --it) {
      auto v = it[-1] < 0 ? ~it[-1] : it[-1];
      // If the prev element is in the group
      if (depth <= v && ISA[v = (v - depth)] == group)
        *--d = v; // put it into the bucket
    }
    // update the beginning of the group
    if (d != e) ISA[e[-1]] = castToIndex(d - SA);
    // Iterate into the left part
    f = e; e = d;
  }

  // Induce lower part
  auto ndepth = depth;
  while (b != d) {
    ndepth += depth;
    auto cgroup = castToIndex(b - SA);
    for (auto it = a; it != b; ++it) {
      auto v = *it;
      // If the prev element is in the group
      if (depth <= v && ISA[v = (v - depth)] == group) {
        ISA[*c++ = v] = cgroup;
        ISA[v + 1]    = -ndepth;
      }
    }
    // Flag end of lower part to mark it type F
    b[-1] = ~b[-1];
    // Interate into the right part
    a = b; b = c;
  }
  // Flag the center
  if (a != b) b[-1] = ~b[-1];

  return b;
}

// Partition into type L, T and S
template <class T, class U, class D>
static T partition(T SA, U ISA, T first, T last, D depth) {
  // Index function: return the next element according to the current depth
  auto index = detail::misc::index(ISA, depth);

  // Name of the group equals the index of the first element
  // Only sort items leading to groups bigger than the current
  T a, b;
  static_assert(!std::is_reference<T>::value, "T is a reference");
  std::tie(a, b) = detail::inplace::partition(first, last, index, first - SA);

  // update the beginning of the group
  auto castToIndex = detail::misc::castTo<decltype(*ISA)>();
  if (b != last) ISA[last[-1]] = castToIndex(b - SA);

  // Induce sort the tandem repeats part (= equal partition) into type L and S
  return detail::suffix::induce(SA, ISA, first, a, b, last, depth, first - SA);
}

template <class T, class U, class D>
inline auto name(T SA, U ISA, D depth) {
  return [SA, ISA, depth = depth + 1](auto a, auto b) {
    auto castToIndex = detail::misc::castTo<decltype(*ISA)>();
    if (std::distance(a, b) < 2) {
      ISA[*a] = castToIndex(a - SA);
      return (void) (*a = ~*a); // Group is unique - flag it as type F
    }

    // Get the element following the current
    auto n = ISA[*a + depth];
    // If it's negative it was already sorted and contains the current sorting depth
    auto ndepth = depth + ((n >> (sizeof(decltype(n)) * CHAR_BIT - 1)) & ~n);
    // auto ndepth = depth + (ISA[*a + depth] < 0 ? -ISA[*a + depth] - 1 : 0);

    // Naming Stage: rename and update depth
    std::for_each(a, b, [ISA, cgroup = castToIndex(a - SA), ndepth](auto c) {
      // Storing the depth in the next item is possible due to
      // the fact that we will never sort this pair again.
      // That this works strongly indicates O(n) time for radix sort implementation
      // because it shows that every pair is sorted at max once and there are
      // only O(n) pairs.The only problem arises with tandem repeats which need to
      // be sorted using induction
      ISA[c + 0] = +cgroup;
      ISA[c + 1] = -ndepth;
    });

    // Partition and return
    return (void) sort::detail::suffix::partition(SA, ISA, a, b, ndepth);
  };
}

}  // suffix
}  // detail
}  // sort

#endif  // SORT_DETAIL_SUFFIX_H
