// Copyright (c) 2023 Patrick S. Klein (@libklein)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <routingblocks/Solution.h>

#include <numeric>

namespace routingblocks {

    bool NodeLocation::operator==(const NodeLocation& rhs) const {
        return route == rhs.route && position == rhs.position;
    }

    bool NodeLocation::operator!=(const NodeLocation& rhs) const { return !(rhs == *this); }

    void Solution::_update_vertex_lookup(unsigned int route_index) {
        const auto& route = _routes[route_index];
        auto node_iter = route.begin();
        for (auto node_index = 0u; node_index < route.size(); ++node_index, ++node_iter) {
            const auto& node = *node_iter;
            // Remove old entries
            auto& vertex_lookup = _vertex_lookup[node.vertex_id()];
            /*vertex_lookup.erase(std::remove_if(
                vertex_lookup.begin(), vertex_lookup.end(),
                [route_index](const NodeLocation& loc) { return loc.route == route_index; }));*/
            // Update
            vertex_lookup.emplace_back(route_index, node_index);
        }
    }
    void Solution::_update_vertex_lookup() {
        for (auto& lookup : _vertex_lookup) lookup.clear();

        for (unsigned int route_index = 0u; route_index < _routes.size(); ++route_index) {
            _update_vertex_lookup(route_index);
        }
    }
    void Solution::exchange_segment(Solution::iterator from_route,
                                    typename route_t::iterator from_route_segment_begin,
                                    typename route_t::iterator from_route_segment_end,
                                    Solution::iterator to_route,
                                    typename route_t::iterator to_route_segment_begin,
                                    typename route_t::iterator to_route_segment_end) {
        if (from_route != to_route) {
            from_route->exchange_segments(from_route_segment_begin, from_route_segment_end,
                                          to_route_segment_begin, to_route_segment_end, *to_route);
        } else {
            from_route->exchange_segments(from_route_segment_begin, from_route_segment_end,
                                          to_route_segment_begin, to_route_segment_end);
        }
        // Update
        _update_vertex_lookup();
    }

    Solution::route_t::iterator Solution::insert_vertex_after(Solution::iterator route,
                                                              typename route_t::iterator pos,
                                                              VertexID vertex_id) {
        const auto& inserted_vertex = _instance->getVertex(vertex_id);
        std::array<route_t::node_t, 1> temporary_segment
            = {route_t::node_t(inserted_vertex, _evaluation->create_forward_label(inserted_vertex),
                               _evaluation->create_backward_label(inserted_vertex))};
        auto new_pos
            = route->insert_segment_after(pos, std::make_move_iterator(temporary_segment.begin()),
                                          std::make_move_iterator(temporary_segment.end()));
        _update_vertex_lookup();
        return new_pos;
    }

    Solution::route_t::iterator Solution::remove_route_segment(Solution::iterator route,
                                                               typename route_t::iterator begin,
                                                               typename route_t::iterator end) {
        auto new_pos = route->remove_segment(begin, end);
        _update_vertex_lookup();
        return new_pos;
    }
    auto Solution::remove_vertex(Solution::iterator route, typename route_t::iterator position) ->
        typename route_t::iterator {
        return this->remove_route_segment(route, position, std::next(position));
    }
}  // namespace routingblocks
