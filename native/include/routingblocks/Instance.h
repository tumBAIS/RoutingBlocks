
#ifndef routingblocks_INSTANCE_H
#define routingblocks_INSTANCE_H

#include <routingblocks/arc.h>
#include <routingblocks/utility/iterator_pair.h>
#include <routingblocks/vertex.h>

#include <cassert>
#include <memory>
#include <ostream>
#include <vector>

namespace routingblocks {
    class Instance {
        // contains [depot, customer_1, ..., customer_n, station_1, ..., station_n]
        std::vector<Vertex> _vertices;
        std::vector<std::vector<Arc>> _arcs;

        VertexID _number_of_customers;
        VertexID _number_of_stations;

        std::vector<Vertex>::const_iterator _customers_begin;
        std::vector<Vertex>::const_iterator _customers_end;
        std::vector<Vertex>::const_iterator _stations_begin;
        std::vector<Vertex>::const_iterator _stations_end;

        int _fleet_size;

      public:
        Instance(std::vector<Vertex> vertices, std::vector<std::vector<Arc>> arcs, int fleetSize);

        [[nodiscard]] const Vertex& getVertex(size_t id) const {
            assert(id < _vertices.size());
            return _vertices[id];
        }

        [[nodiscard]] const Vertex& getCustomer(size_t id) const {
            assert(id < _number_of_customers);
            return *std::next(_customers_begin, id);
        }

        [[nodiscard]] const Vertex& getStation(size_t id) const {
            assert(id < _number_of_stations);
            return *std::next(_stations_begin, id);
        }

        [[nodiscard]] const Arc& getArc(size_t i, size_t j) const { return _arcs[i][j]; }

        [[nodiscard]] size_t NumberOfVertices() const { return _vertices.size(); }

        [[nodiscard]] size_t NumberOfCustomers() const { return _number_of_customers; }

        [[nodiscard]] size_t NumberOfStations() const { return _number_of_stations; }

        [[nodiscard]] int FleetSize() const { return _fleet_size; }

        [[nodiscard]] const Vertex& Depot() const { return _vertices[0]; }

        [[nodiscard]] auto Customers() const {
            return utility::make_iterator_pair(_customers_begin, _customers_end);
        }

        [[nodiscard]] auto Stations() const {
            return utility::make_iterator_pair(_stations_begin, _stations_end);
        }

        [[nodiscard]] auto begin() { return _vertices.begin(); }
        [[nodiscard]] auto begin() const { return _vertices.begin(); }
        [[nodiscard]] auto end() { return _vertices.end(); }
        [[nodiscard]] auto end() const { return _vertices.end(); }
    };
}  // namespace routingblocks
#endif  // routingblocks_INSTANCE_H
