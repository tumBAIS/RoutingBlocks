#include "routingblocks_bindings/Instance.hpp"

#include <pybind11/stl.h>
#include <routingblocks/ADPTWEvaluation.h>
#include <routingblocks/Instance.h>
#include <routingblocks/NIFTWEvaluation.h>

#include <routingblocks_bindings/binding_helpers.hpp>

namespace {
    template <class data_t> routingblocks::Vertex vertex_constructor(size_t vid, std::string name,
                                                                     bool is_station, bool is_depot,
                                                                     data_t user_data) {
        return routingblocks::Vertex(vid, name, is_station, is_depot,
                                     std::make_shared<data_t>(std::move(user_data)));
    }

    template <class data_t> routingblocks::Arc arc_constructor(data_t user_data) {
        return routingblocks::Arc(std::make_shared<data_t>(std::move(user_data)));
    }
}  // namespace

namespace routingblocks::bindings {
    template <class VertexData> auto bind_vertex(pybind11::module& m, std::string_view name) {
        return pybind11::class_<routingblocks::Vertex>(m, name.data())
            .def(pybind11::init(&vertex_constructor<VertexData>))
            .def_readonly("id", &routingblocks::Vertex::id)
            .def_readonly("vertex_id", &routingblocks::Vertex::id)
            .def_readonly("str_id", &routingblocks::Vertex::str_id)
            .def_readonly("is_station", &routingblocks::Vertex::is_station)
            .def_readonly("is_depot", &routingblocks::Vertex::is_depot)
            .def_property_readonly("is_customer",
                                   [](const Vertex& v) { return !v.is_station && !v.is_depot; })
            .def("__str__", &::bindings::helpers::ostream_to_string<routingblocks::Vertex>);
    }

    template <class ArcData> auto bind_arc(pybind11::module& m, std::string_view name) {
        return pybind11::class_<routingblocks::Arc>(m, name.data())
            .def(pybind11::init<>(&arc_constructor<ArcData>));
    }

    void bind_routingblocks_instance(pybind11::module& m) {
        bind_vertex<pybind11::object>(m, "Vertex");
        m.def("create_adptw_vertex", &vertex_constructor<ADPTWVertexData>);
        m.def("create_niftw_vertex", &vertex_constructor<NIFTWVertexData>);

        bind_arc<pybind11::object>(m, "Arc");
        m.def("create_adptw_arc", &arc_constructor<ADPTWArcData>);
        m.def("create_niftw_arc", &arc_constructor<NIFTWArcData>);

        pybind11::class_<routingblocks::ADPTWVertexData>(m, "ADPTWVertexData")
            .def(pybind11::init<float, float, resource_t, resource_t, resource_t, resource_t>());
        pybind11::class_<routingblocks::ADPTWArcData>(m, "ADPTWArcData")
            .def(pybind11::init<resource_t, resource_t, resource_t>());

        pybind11::class_<routingblocks::Instance>(m, "Instance")
            .def(pybind11::init<std::vector<routingblocks::Vertex>,
                                std::vector<std::vector<routingblocks::Arc>>, int>())
            .def_property_readonly("fleet_size", &routingblocks::Instance::FleetSize)
            .def_property_readonly("number_of_customers",
                                   &routingblocks::Instance::NumberOfCustomers)
            .def_property_readonly("number_of_stations", &routingblocks::Instance::NumberOfStations)
            .def_property_readonly("number_of_vertices", &routingblocks::Instance::NumberOfVertices)
            .def_property_readonly("depot", &routingblocks::Instance::Depot)
            .def_property_readonly("stations",
                                   [](const routingblocks::Instance& inst) {
                                       auto stations = inst.Stations();
                                       return pybind11::make_iterator(std::begin(stations),
                                                                      std::end(stations));
                                   })
            .def_property_readonly("customers",
                                   [](const routingblocks::Instance& inst) {
                                       auto customers = inst.Customers();
                                       return pybind11::make_iterator(std::begin(customers),
                                                                      std::end(customers));
                                   })
            .def("__len__", &routingblocks::Instance::NumberOfVertices)
            .def("__iter__",
                 [](const routingblocks::Instance& inst) {
                     return pybind11::make_iterator(inst.begin(), inst.end());
                 })
            .def("get_vertex", &routingblocks::Instance::getVertex,
                 pybind11::return_value_policy::reference_internal)
            .def("get_customer", &routingblocks::Instance::getCustomer,
                 pybind11::return_value_policy::reference_internal)
            .def("get_station", &routingblocks::Instance::getStation,
                 pybind11::return_value_policy::reference_internal)
            .def("get_arc", &routingblocks::Instance::getArc, "Gets an arc by vertex id",
                 pybind11::return_value_policy::reference_internal);
    }

}  // namespace routingblocks::bindings
