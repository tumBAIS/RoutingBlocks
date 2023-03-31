#include "vrpis_bindings/Instance.hpp"

#include <pybind11/stl.h>
#include <vrpis/ADPTWEvaluation.h>
#include <vrpis/Instance.h>
#include <vrpis/NIFTWEvaluation.h>

#include "vrpis_bindings/binding_helpers.hpp"

namespace {
    template <class data_t> vrpis::Vertex vertex_constructor(size_t vid, std::string name,
                                                             bool is_station, bool is_depot,
                                                             data_t user_data) {
        return vrpis::Vertex(vid, name, is_station, is_depot,
                             std::make_shared<data_t>(std::move(user_data)));
    }

    template <class data_t> vrpis::Arc arc_constructor(data_t user_data) {
        return vrpis::Arc(std::make_shared<data_t>(std::move(user_data)));
    }
}  // namespace

namespace vrpis::bindings {
    template <class VertexData> auto bind_vertex(pybind11::module& m, std::string_view name) {
        return pybind11::class_<vrpis::Vertex>(m, name.data())
            .def(pybind11::init(&vertex_constructor<VertexData>))
            .def_readonly("id", &vrpis::Vertex::id)
            .def_readonly("vertex_id", &vrpis::Vertex::id)
            .def_readonly("str_id", &vrpis::Vertex::str_id)
            .def_readonly("is_station", &vrpis::Vertex::is_station)
            .def_readonly("is_depot", &vrpis::Vertex::is_depot)
            .def_property_readonly("is_customer",
                                   [](const Vertex& v) { return !v.is_station && !v.is_depot; })
            .def("__str__", &::bindings::helpers::ostream_to_string<vrpis::Vertex>);
    }

    template <class ArcData> auto bind_arc(pybind11::module& m, std::string_view name) {
        return pybind11::class_<vrpis::Arc>(m, name.data())
            .def(pybind11::init<>(&arc_constructor<ArcData>));
    }

    void bind_vrpis_instance(pybind11::module& m) {
        bind_vertex<pybind11::object>(m, "Vertex");
        m.def("create_adptw_vertex", &vertex_constructor<ADPTWVertexData>);
        m.def("create_niftw_vertex", &vertex_constructor<NIFTWVertexData>);

        bind_arc<pybind11::object>(m, "Arc");
        m.def("create_adptw_arc", &arc_constructor<ADPTWArcData>);
        m.def("create_niftw_arc", &arc_constructor<NIFTWArcData>);

        pybind11::class_<vrpis::ADPTWVertexData>(m, "ADPTWVertexData")
            .def(pybind11::init<float, float, resource_t, resource_t, resource_t, resource_t>());
        pybind11::class_<vrpis::ADPTWArcData>(m, "ADPTWArcData")
            .def(pybind11::init<resource_t, resource_t, resource_t>());

        pybind11::class_<vrpis::Instance>(m, "Instance")
            .def(pybind11::init<std::vector<vrpis::Vertex>, std::vector<std::vector<vrpis::Arc>>,
                                int>())
            .def_property_readonly("fleet_size", &vrpis::Instance::FleetSize)
            .def_property_readonly("number_of_customers", &vrpis::Instance::NumberOfCustomers)
            .def_property_readonly("number_of_stations", &vrpis::Instance::NumberOfStations)
            .def_property_readonly("number_of_vertices", &vrpis::Instance::NumberOfVertices)
            .def_property_readonly("depot", &vrpis::Instance::Depot)
            .def_property_readonly("stations",
                                   [](const vrpis::Instance& inst) {
                                       auto stations = inst.Stations();
                                       return pybind11::make_iterator(stations.begin(),
                                                                      stations.end());
                                   })
            .def_property_readonly("customers",
                                   [](const vrpis::Instance& inst) {
                                       auto customers = inst.Customers();
                                       return pybind11::make_iterator(customers.begin(),
                                                                      customers.end());
                                   })
            .def("__len__", &vrpis::Instance::NumberOfVertices)
            .def("__iter__",
                 [](const vrpis::Instance& inst) {
                     return pybind11::make_iterator(inst.begin(), inst.end());
                 })
            .def("get_vertex", &vrpis::Instance::getVertex,
                 pybind11::return_value_policy::reference_internal)
            .def("get_customer", &vrpis::Instance::getCustomer,
                 pybind11::return_value_policy::reference_internal)
            .def("get_station", &vrpis::Instance::getStation,
                 pybind11::return_value_policy::reference_internal)
            .def("get_arc", &vrpis::Instance::getArc, "Gets an arc by vertex id",
                 pybind11::return_value_policy::reference_internal);
    }

}  // namespace vrpis::bindings
