#include <routingblocks/Instance.h>

#include <algorithm>
#include <iostream>

using namespace routingblocks;

Instance::Instance(std::vector<Vertex> vertices, std::vector<std::vector<Arc>> arcs, int fleetSize)
    : _vertices(std::move(vertices)), _arcs(std::move(arcs)), _fleet_size(fleetSize) {
    // vertices should be ordered as [depot, cust_1, ..., cust_n, station_1, ..., station_n]
    auto next_vertex = _vertices.begin();
    size_t next_vertex_id = 0;

    if (!next_vertex->is_depot || next_vertex->id != next_vertex_id) {
        throw std::runtime_error("Depot is not first vertex");
    }

    for (next_vertex = std::next(next_vertex), next_vertex_id = 1;
         !next_vertex->is_station && next_vertex != _vertices.end();
         ++next_vertex, ++next_vertex_id) {
        if (next_vertex->is_depot || next_vertex->is_station || next_vertex->id != next_vertex_id) {
            throw std::runtime_error(
                "Wrong vertex ordering! Expected order: depot, customers, stations with sequential "
                "id's. Problem: a depot or station vertex is at a position where a customer was "
                "expected.");
        }
    }

    _number_of_customers = next_vertex_id - 1;  // Account for depot
    _number_of_stations = _vertices.size() - 1 - _number_of_customers;
    auto station_offset = next_vertex_id;

    for (; next_vertex != _vertices.end(); ++next_vertex, ++next_vertex_id) {
        if (next_vertex->is_depot || !next_vertex->is_station
            || next_vertex_id != next_vertex->id) {
            throw std::runtime_error(
                "Wrong vertex ordering! Expected order: depot, customers, stations with sequential "
                "id's. Problem: A non-station vertex follows customer vertices");
        }
    }

    if (_fleet_size <= 0) {
        throw std::runtime_error(
            "fleet size, vehicle capacity, and vehicle battery capacity must be greater than 0");
    }

    _customers_begin = std::next(_vertices.begin(), 1);
    _customers_end = std::next(_vertices.begin(), 1 + _number_of_customers);
    _stations_begin = std::next(_vertices.begin(), station_offset);
    _stations_end = std::next(_vertices.begin(), station_offset + _number_of_stations);
}
Arc::Arc(Arc::data_t data) : data(std::move(data)) {}
