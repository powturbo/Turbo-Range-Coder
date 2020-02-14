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

#ifndef SORT_COPY_H
#define SORT_COPY_H

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
#include "inplace.h"

namespace sort {

// Make pair accessible from outside
template<class T1, class T2>
using pair = detail::misc::pair<T1, T2>;

namespace copy {

// Oportunistic version of the quicksort
// Uses free space given to it to copy together key and value
// then sorting it
template <int LR = detail::misc::LR, class T, class U, class I, class C>
inline void quick(T first, T last, U Sf, U Sl, I index, C cb) {
  using typeA = std::remove_reference_t<decltype(*Sf)>;
  using typeB = std::remove_reference_t<decltype(*first)>;
  using typeC = std::remove_reference_t<decltype(index(*first))>;
  static_assert(std::is_same<typeA, detail::misc::pair<typeC, typeB>>::value, "Type mismatch");

  if (detail::misc::COPY_MIN <= std::distance(first, last)
      && std::distance(first, last) < std::distance(Sf, Sl)) {
    Sl = Sf + std::distance(first, last);

    // get a pivot
    typeC pivot; int equals;
    std::tie(pivot, equals) = detail::misc::median7_copy<typeC>(first, index);
    if (std::distance(first, last) * (6 - equals) < detail::misc::COPY_MIN * 7)
      return sort::inplace::quick<LR>(first, last, index, cb);

    // copy together + initial partitioning
    auto a = Sf, b = Sl;
    for (auto it = first; it != last; ++it) {
      auto v = detail::misc::make_pair(index(*it), *it);
      *(v.first < pivot ? a++ : --b) = v;
    }

    auto idx = [](auto a) { return a.first; };
    auto icb = [first, Sf, cb](auto a, auto b) {
      // copy back
      for (auto it = a; it != b; ++it)
        first[it - Sf] = it->second;

      // call the cb
      cb(first + (a - Sf), first + (b - Sf));
    };

    if (LR) {
      sort::inplace::block<LR>(Sf, a, idx, icb);
      sort::inplace::block<LR>(a, Sl, idx, icb);
    } else {
      sort::inplace::block<LR>(a, Sl, idx, icb);
      sort::inplace::block<LR>(Sf, a, idx, icb);
    }
  } else  // not enough space
    sort::inplace::quick<LR>(first, last, index, cb);
}

template <int LR = detail::misc::LR, class T, class U, class I>
inline void quick(T first, T last, U Sf, U Sl, I index) {
  using typeA = std::remove_reference_t<decltype(*Sf)>;
  using typeB = std::remove_reference_t<decltype(*first)>;
  using typeC = std::remove_reference_t<decltype(index(*first))>;
  static_assert(std::is_same<typeA, detail::misc::pair<typeC, typeB>>::value, "Type mismatch");

  if (detail::misc::COPY_MIN <= std::distance(first, last)
      && std::distance(first, last) < std::distance(Sf, Sl)) {
    Sl = Sf + std::distance(first, last);

    // get a pivot
    typeC pivot = index(*first);

    // copy together + initial partition
    auto a = Sf, b = Sl;
    for (auto it = first; it != last; ++it) {
      auto v = detail::misc::make_pair(index(*it), *it);
      *(v.first < pivot ? a++ : --b) = v;
    }

    auto idx = [](auto a) { return a.first; };

    if (LR) {
      sort::inplace::block<LR>(Sf, a, idx);
      for (auto it = Sf; it != a; ++it)
        first[std::distance(Sf, it)] = it->second;
      sort::inplace::block<LR>(a, Sl, idx);
      for (auto it = a; it != Sl; ++it)
        first[std::distance(Sf, it)] = it->second;
    } else {
      sort::inplace::block<LR>(a, Sl, idx);
      for (auto it = a; it != Sl; ++it)
        first[std::distance(Sf, it)] = it->second;
      sort::inplace::block<LR>(Sf, a, idx);
      for (auto it = Sf; it != a; ++it)
        first[std::distance(Sf, it)] = it->second;
    }

  } else  // not enough space
    sort::inplace::quick<LR>(first, last, index);
}

}  // copy
}  // sort

#endif  // SORT_COPY_H
