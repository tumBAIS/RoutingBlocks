#include "routingblocks_bindings/specializations/NIFTW.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "routingblocks/NIFTWEvaluation.h"
#include "routingblocks_bindings/binding_helpers.hpp"

namespace routingblocks::bindings {

    void bind_niftw(pybind11::module_& m) {
        ::bindings::helpers::bind_concatenation_evaluation_specialization<NIFTWEvaluation>(
            pybind11::class_<routingblocks::NIFTWEvaluation, Evaluation>(m, "NIFTWEvaluation")
                .def(pybind11::init<resource_t, resource_t, resource_t>()))
            .def_readwrite("overload_penalty_factor", &NIFTWEvaluation::overload_penalty_factor)
            .def_readwrite("resource_penalty_factor", &NIFTWEvaluation::overcharge_penalty_factor)
            .def_readwrite("time_shift_penalty_factor",
                           &NIFTWEvaluation::time_shift_penalty_factor);

        pybind11::class_<routingblocks::NIFTWVertexData>(m, "NIFTWVertexData")
            .def(pybind11::init<float, float, resource_t, resource_t, resource_t, resource_t>());
        pybind11::class_<routingblocks::NIFTWArcData>(m, "NIFTWArcData")
            .def(pybind11::init<resource_t, resource_t, resource_t>());
        m.def("create_niftw_vertex", &::bindings::helpers::vertex_constructor<NIFTWVertexData>);
        m.def("create_niftw_arc", &::bindings::helpers::arc_constructor<NIFTWArcData>);

        pybind11::class_<FRVCP<NIFTWDPLabel>>(m, "NIFTWFacilityPlacementOptimizer")
            .def(pybind11::init<>([](const Instance& instance, resource_t resource_capacity,
                                     resource_t replenishment_time) {
                return FRVCP<NIFTWDPLabel>(instance,
                                           std::make_shared<Propagator<NIFTWDPLabel>>(
                                               instance, resource_capacity, replenishment_time));
            }))
            .def("optimize", &FRVCP<NIFTWDPLabel>::optimize,
                 "Solve the detour embedding problem for the specified route.");
    }

}  // namespace routingblocks::bindings