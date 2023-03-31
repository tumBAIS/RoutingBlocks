
#ifndef vrpis_INSTANCE_H
#define vrpis_INSTANCE_H

#include <cassert>
#include <memory>
#include <ostream>
#include <ranges>
#include <vector>

#include "vrpis/interfaces/arc.h"
#include "vrpis/interfaces/vertex.h"

namespace vrpis {
    class Instance {
        // contains [depot, customer_1, ..., customer_n, station_1, ..., station_n]
        std::vector<Vertex> _vertices;
        std::vector<std::vector<Arc>> _arcs;

        VertexID _number_of_customers;
        VertexID _station_offset;
        VertexID _number_of_stations;

        int _fleet_size;

      public:
        Instance(std::vector<Vertex> vertices, std::vector<std::vector<Arc>> arcs, int fleetSize);

        [[nodiscard]] const Vertex& getVertex(size_t id) const {
            assert(id < _vertices.size());
            return _vertices[id];
        }

        [[nodiscard]] const Vertex& getCustomer(size_t id) const {
            assert(id < _number_of_customers);
            return _vertices[id + 1];
        }

        [[nodiscard]] const Vertex& getStation(size_t id) const {
            assert(id < _number_of_stations);
            return _vertices[id + _station_offset];
        }

        [[nodiscard]] const Arc& getArc(size_t i, size_t j) const { return _arcs[i][j]; }

        [[nodiscard]] size_t NumberOfVertices() const { return _vertices.size(); }

        [[nodiscard]] size_t NumberOfCustomers() const { return _number_of_customers; }

        [[nodiscard]] size_t NumberOfStations() const { return _number_of_stations; }

        [[nodiscard]] int FleetSize() const { return _fleet_size; }

        [[nodiscard]] const Vertex& Depot() const { return _vertices[0]; }

        [[nodiscard]] auto Customers() const {
            return std::views::counted(_vertices.begin() + 1, _number_of_customers);
        }

        [[nodiscard]] auto Stations() const {
            return std::views::counted(_vertices.begin() + _station_offset, _number_of_stations);
        }

        [[nodiscard]] auto begin() { return _vertices.begin(); }
        [[nodiscard]] auto begin() const { return _vertices.begin(); }
        [[nodiscard]] auto end() { return _vertices.end(); }
        [[nodiscard]] auto end() const { return _vertices.end(); }
    };
}  // namespace vrpis
#endif  // vrpis_INSTANCE_H
