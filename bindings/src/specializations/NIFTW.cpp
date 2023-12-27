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