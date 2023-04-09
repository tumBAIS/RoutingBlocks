
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
