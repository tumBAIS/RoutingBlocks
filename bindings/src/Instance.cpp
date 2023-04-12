#include "routingblocks_bindings/Instance.hpp"

#include <pybind11/stl.h>
#include <routingblocks/ADPTWEvaluation.h>
#include <routingblocks/Instance.h>
#include <routingblocks/NIFTWEvaluation.h>

#include <routingblocks_bindings/binding_helpers.hpp>

namespace routingblocks::bindings {
    template <class VertexData> auto bind_vertex(pybind11::module& m, std::string_view name) {
        return pybind11::class_<routingblocks::Vertex>(m, name.data())
            .def(pybind11::init(&::bindings::helpers::vertex_constructor<VertexData>))
            .def_readonly("vertex_id", &routingblocks::Vertex::id, "The id of the vertex.")
            .def_readonly("str_id", &routingblocks::Vertex::str_id, "The name of the vertex.")
            .def_readonly("is_station", &routingblocks::Vertex::is_station)
            .def_readonly("is_depot", &routingblocks::Vertex::is_depot)
            .def_property_readonly("is_customer",
                                   [](const Vertex& v) { return !v.is_station && !v.is_depot; })
            .def_property_readonly(
                "data",
                [](const Vertex& v) -> pybind11::object { return v.get_data<pybind11::object>(); },
                "Get the vertex data. Note that this function is well-defined only for VertexData "
                "classes defined in Python.")
            .def("__str__", &::bindings::helpers::ostream_to_string<routingblocks::Vertex>);
    }

    template <class ArcData> auto bind_arc(pybind11::module& m, std::string_view name) {
        return pybind11::class_<routingblocks::Arc>(m, name.data())
            .def(pybind11::init<>(&::bindings::helpers::arc_constructor<ArcData>))
            .def_property_readonly(
                "data",
                [](const Arc& a) -> pybind11::object { return a.get_data<pybind11::object>(); },
                "Get the arc data. Note that this function is well-defined only for ArcData "
                "classes defined in Python.");
    }

    void bind_routingblocks_instance(pybind11::module& m) {
        bind_vertex<pybind11::object>(m, "Vertex");
        bind_arc<pybind11::object>(m, "Arc");

        auto adptw = m.def_submodule("adptw");
        pybind11::class_<routingblocks::ADPTWVertexData>(adptw, "ADPTWVertexData")
            .def(pybind11::init<float, float, resource_t, resource_t, resource_t, resource_t>());
        pybind11::class_<routingblocks::ADPTWArcData>(adptw, "ADPTWArcData")
            .def(pybind11::init<resource_t, resource_t, resource_t>());
        adptw.def("create_adptw_vertex", &::bindings::helpers::vertex_constructor<ADPTWVertexData>);
        adptw.def("create_adptw_arc", &::bindings::helpers::arc_constructor<ADPTWArcData>);

        auto niftw = m.def_submodule("niftw");
        pybind11::class_<routingblocks::NIFTWVertexData>(niftw, "NIFTWVertexData")
            .def(pybind11::init<float, float, resource_t, resource_t, resource_t, resource_t>());
        pybind11::class_<routingblocks::NIFTWArcData>(niftw, "NIFTWArcData")
            .def(pybind11::init<resource_t, resource_t, resource_t>());
        niftw.def("create_niftw_vertex", &::bindings::helpers::vertex_constructor<NIFTWVertexData>);
        niftw.def("create_niftw_arc", &::bindings::helpers::arc_constructor<NIFTWArcData>);

        pybind11::class_<routingblocks::Instance>(m, "Instance")
            .def(pybind11::init<std::vector<routingblocks::Vertex>,
                                std::vector<std::vector<routingblocks::Arc>>>())
            .def(pybind11::init<std::vector<routingblocks::Vertex>,
                                std::vector<std::vector<routingblocks::Arc>>, int>())
            .def(pybind11::init<routingblocks::Vertex, std::vector<routingblocks::Vertex>,
                                std::vector<routingblocks::Vertex>,
                                std::vector<std::vector<routingblocks::Arc>>, int>())
            .def_property_readonly("fleet_size", &routingblocks::Instance::FleetSize, "The number of "
                                                                                     "vehicles available.")
            .def_property_readonly("number_of_customers",
                                   &routingblocks::Instance::NumberOfCustomers)
            .def_property_readonly("number_of_stations", &routingblocks::Instance::NumberOfStations)
            .def_property_readonly("number_of_vertices", &routingblocks::Instance::NumberOfVertices)
            .def_property_readonly("depot", &routingblocks::Instance::Depot, "The depot vertex.")
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
