
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
