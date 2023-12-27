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


#ifndef _routingblocks_ARC_SET_H
#define _routingblocks_ARC_SET_H

#include <dynamic_bitset/dynamic_bitset.hpp>

namespace routingblocks::utility {
    class arc_set {
        using bitset_t = sul::dynamic_bitset<>;
        bitset_t _bitset;
        size_t _number_of_vertices;

      public:
        explicit arc_set(size_t number_of_vertices)
            : _bitset(number_of_vertices * number_of_vertices),
              _number_of_vertices(number_of_vertices) {
            _bitset.set();
        }

        void forbid_arc(VertexID from, VertexID to) {
            _bitset[from * _number_of_vertices + to] = false;
        }

        void include_arc(VertexID from, VertexID to) {
            _bitset[from * _number_of_vertices + to] = true;
        }

        [[nodiscard]] bool includes_arc(VertexID from, VertexID to) const {
            return _bitset.test(from * _number_of_vertices + to);
        }
    };
}  // namespace routingblocks::utility

#endif  //_routingblocks_ARC_SET_H
