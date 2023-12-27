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


#ifndef _routingblocks_ITERATOR_PAIR_H
#define _routingblocks_ITERATOR_PAIR_H

namespace routingblocks::utility {
    /***
     * Provides a simple iterator helper class that allows to use range-based for loops with
     * std::pair<iterator, iterator> objects.
     */
    template <typename Iterator> class iterator_pair {
        Iterator _begin;
        Iterator _end;

      public:
        iterator_pair(Iterator begin, Iterator end) : _begin(begin), _end(end) {}
        Iterator begin() const { return _begin; }
        Iterator end() const { return _end; }
    };

    template <typename Iterator>
    iterator_pair<Iterator> make_iterator_pair(Iterator begin, Iterator end) {
        return iterator_pair<Iterator>(begin, end);
    }
}  // namespace routingblocks::utility

#endif  //_routingblocks_ITERATOR_PAIR_H
