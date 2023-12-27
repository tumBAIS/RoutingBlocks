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


#ifndef _routingblocks_VERTEX_H
#define _routingblocks_VERTEX_H

#include <memory>
#include <ostream>

namespace routingblocks {
    using VertexID = size_t;

    struct Vertex {
        using data_t = std::shared_ptr<void>;
        // Pointer to type-erased (arbitrary) data
        data_t data;
        VertexID id;
        std::string str_id;
        bool is_station;
        bool is_depot;

        friend std::ostream& operator<<(std::ostream& os, const Vertex& vertex);
        template <class T> T& get_data() { return *static_cast<T*>(data.get()); }
        template <class T> const T& get_data() const { return *static_cast<const T*>(data.get()); }

        [[nodiscard]] bool customer() const { return !is_station && !is_depot; }
        [[nodiscard]] bool station() const { return is_station; }
        [[nodiscard]] bool depot() const { return is_depot; }

        Vertex(VertexID id, std::string str_id, bool is_station, bool is_depot, data_t data)
            : data(std::move(data)),
              id(id),
              str_id(std::move(str_id)),
              is_station(is_station),
              is_depot(is_depot){};
    };

    inline std::ostream& operator<<(std::ostream& os, const routingblocks::Vertex& vertex) {
        os << vertex.str_id;
        return os;
    }
}  // namespace routingblocks

#endif  //_routingblocks_VERTEX_H
