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

#ifndef SORT_INPLACE_H
#define SORT_INPLACE_H

#ifdef __INTEL_COMPILER
#pragma warning disable 3373
#endif

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdint>
#include <tuple>
#include <type_traits>

#include "detail/misc.h"
#include "detail/inplace.h"

namespace sort {
namespace inplace {

// Fast general purpose multi pivot introsort
// with index function and optimal callback on each equal ranges.
// One of these implementation should already be faster than std::sort
// and especially for highly repetitive data

// If branch misses slow down your sort use block()
// In any other case (slow index function) use quick()
// as it reduces the number of index() calls

// Average runtime is O(n * log(m))
// Worst case is O(n * log(n))
// where m is the number of destinct values
template <int LR = detail::misc::LR, class T, class I, class C>
inline void quick(T first, T last, I index, C cb) {
  int budget =  3 * detail::misc::ilogb(last - first + 1) >> 1;
  detail::inplace::quick<LR, 0>(first, last, index, cb, budget);
}

template <int LR = detail::misc::LR, class T, class I>
inline void quick(T first, T last, I index) {
  int budget = detail::misc::ilogb(last - first + 1);
  detail::inplace::quick<LR, 0>(first, last, index, [](auto a, auto b) {
    (void) a; (void) b;
  }, budget);
}

template <int LR = detail::misc::LR, class T, class I, class C>
inline void block(T first, T last, I index, C cb) {
  int budget = 2 * detail::misc::ilogb(last - first + 1);
  detail::inplace::quick<LR, 1>(first, last, index, cb, budget);
}

template <int LR = detail::misc::LR, class T, class I>
inline void block(T first, T last, I index) {
  int budget = 2 * detail::misc::ilogb(last - first + 1);
  detail::inplace::quick<LR, 1>(first, last, index, [](auto a, auto b) {
    (void) a; (void) b;
  }, budget);
}

}  // inplace
}  // sort

#endif  // SORT_INPLACE_H
