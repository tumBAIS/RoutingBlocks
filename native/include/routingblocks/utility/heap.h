/*
 * Copyright (c) 2023 Patrick S. Klein (@libklein)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef UTIL_HEAP_H
#define UTIL_HEAP_H

#include <queue>

namespace util {
    template <class T, class Comp = std::greater<T>> class Heap {
      private:
        using heap_impl_t = std::priority_queue<T, std::vector<T>, Comp>;

      public:
        using size_t = typename heap_impl_t::size_type;

      private:
        Comp _comp;
        heap_impl_t _heap_impl;

      public:
        Heap() = default;
        explicit Heap(const Comp& comp) : _comp(comp), _heap_impl(_comp){};

        T pop() {
            assert(!empty());
            T moved_from_top = std::move(const_cast<T&>(_heap_impl.top()));
            _heap_impl.pop();
            return moved_from_top;
        };
        const T& top() const { return _heap_impl.top(); };

        void push(T value) { _heap_impl.push(std::move(value)); }

        [[nodiscard]] bool empty() const { return _heap_impl.empty(); }

        [[nodiscard]] size_t size() const { return _heap_impl.size(); }

        void clear() {
            decltype(_heap_impl) empty_heap(_comp);
            _heap_impl.swap(empty_heap);
        }
    };
}  // namespace util

#endif  // UTIL_HEAP_H
