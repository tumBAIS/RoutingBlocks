#include <pybind11/pybind11.h>

#include <iostream>

#include "vrpis/arc.h"
#include "vrpis/interfaces/evaluation.h"
#include "vrpis/vertex.h"

#define STRINGIFY(x) #x
#define PREPROCESSOR_TO_STRING(x) STRINGIFY(x)

#ifndef VRPIS_VERSION
#    define VRPIS_VERSION "dev"
#endif

using label_holder_t = vrpis::detail::label_holder;

struct CVRP_forward_label {
    resource_t distance;
    resource_t load;

    CVRP_forward_label(resource_t distance, resource_t load) : distance(distance), load(load) {}
};
struct CVRP_backward_label {
    resource_t distance;
    resource_t load;

    CVRP_backward_label(resource_t distance, resource_t load) : distance(distance), load(load) {}
};
struct CVRP_vertex_data {
    resource_t demand;

    CVRP_vertex_data(resource_t demand) : demand(demand) {}
};
struct CVRP_arc_data {
    resource_t distance;

    CVRP_arc_data(resource_t distance) : distance(distance) {}
};

class CVRPEvaluation
    : public vrpis::ConcatenationBasedEvaluationImpl<CVRPEvaluation, CVRP_forward_label,
                                                     CVRP_backward_label, CVRP_vertex_data,
                                                     CVRP_arc_data> {
  public:
    enum CostComponent {
        DIST_INDEX = 0,
        OVERLOAD_INDEX = 0,
    };

  private:
    const resource_t _storage_capacity;

    double _overload_penalty_factor = 1.;

  public:
    CVRPEvaluation(resource_t storage_capacity) : _storage_capacity(storage_capacity){};

  private:
    cost_t _compute_cost(resource_t distance, resource_t overload) const {
        return static_cast<cost_t>(distance)
               + static_cast<cost_t>(overload * _overload_penalty_factor);
    };

  public:
    auto get_penalty_factors() const {
        auto vector = std::array<double, 2>();
        vector[DIST_INDEX] = 1.;
        vector[OVERLOAD_INDEX] = _overload_penalty_factor;
        return vector;
    };

    void set_penalty_factors(const std::array<double, 2>& factors) {
        _overload_penalty_factor = factors[OVERLOAD_INDEX];
    };

    cost_t concatenate(const CVRP_forward_label& fwd, const CVRP_backward_label& bwd,
                       const vrpis::Vertex& vertex, const CVRP_vertex_data& vertex_data) {
        return _compute_cost(fwd.distance + bwd.distance,
                             std::max(resource_t(0), (fwd.load + bwd.load) - _storage_capacity));
    }

    [[nodiscard]] std::vector<resource_t> get_cost_components(const CVRP_forward_label& fwd) const {
        return {fwd.distance, std::max(resource_t(0), fwd.load - _storage_capacity)};
    };

    [[nodiscard]] cost_t compute_cost(const CVRP_forward_label& label) const {
        return _compute_cost(label.distance, label.load);
    };

    [[nodiscard]] bool is_feasible(const CVRP_forward_label& fwd) const {
        return fwd.load <= _storage_capacity;
    };

    [[nodiscard]] CVRP_forward_label propagate_forward(const CVRP_forward_label& pred_label,
                                                       const vrpis::Vertex& pred_vertex,
                                                       const CVRP_vertex_data& pred_vertex_data,
                                                       const vrpis::Vertex& vertex,
                                                       const CVRP_vertex_data& vertex_data,
                                                       const vrpis::Arc& arc,
                                                       const CVRP_arc_data& arc_data) const {
        return {pred_label.distance + arc_data.distance, pred_label.load + vertex_data.demand};
    };

    [[nodiscard]] CVRP_backward_label propagate_backward(const CVRP_backward_label& succ_label,
                                                         const vrpis::Vertex& succ_vertex,
                                                         const CVRP_vertex_data& succ_vertex_data,
                                                         const vrpis::Vertex& vertex,
                                                         const CVRP_vertex_data& vertex_data,
                                                         const vrpis::Arc& arc,
                                                         const CVRP_arc_data& arc_data) const {
        return {succ_label.distance + arc_data.distance, succ_label.load + succ_vertex_data.demand};
    };

    CVRP_forward_label create_forward_label(const vrpis::Vertex& vertex,
                                            const CVRP_vertex_data& vertex_data) {
        return {0, vertex_data.demand};
    };

    CVRP_backward_label create_backward_label(const vrpis::Vertex& vertex,
                                              const CVRP_vertex_data& vertex_data) {
        return {0, 0};
    };
};

PYBIND11_MODULE(VRPIS_EXT_MODULE_NAME, m) {
    m.attr("__version__") = PREPROCESSOR_TO_STRING(VRPIS_EXT_MODULE_VERSION);

    pybind11::class_<CVRPEvaluation, vrpis::ConcatenationBasedEvaluation,
                     std::shared_ptr<CVRPEvaluation>>(m, "CVRPEvaluation")
        .def(pybind11::init<resource_t>())
        .def("concatenate", &CVRPEvaluation::concatenate)
        .def("compute_cost", &CVRPEvaluation::compute_cost)
        .def("is_feasible", &CVRPEvaluation::is_feasible)
        .def("get_cost_components", &CVRPEvaluation::get_cost_components)
        .def("propagate_forward", &CVRPEvaluation::propagate_forward)
        .def("propagate_backward", &CVRPEvaluation::propagate_backward)
        .def("create_forward_label", &CVRPEvaluation::create_forward_label)
        .def("create_backward_label", &CVRPEvaluation::create_backward_label);

    pybind11::class_<CVRP_forward_label>(m, "CVRPForwardLabel")
        .def_property_readonly("distance",
                               [](const CVRP_forward_label& label) { return label.distance; })
        .def_property_readonly("load", [](const CVRP_forward_label& label) { return label.load; });

    pybind11::class_<CVRP_backward_label>(m, "CVRPBackwardLabel")
        .def_property_readonly("distance",
                               [](const CVRP_backward_label& label) { return label.distance; })
        .def_property_readonly("load", [](const CVRP_backward_label& label) { return label.load; });

    pybind11::class_<CVRP_vertex_data>(m, "CVRPVertexData")
        .def_property_readonly("demand", [](const CVRP_vertex_data& data) { return data.demand; });

    pybind11::class_<CVRP_arc_data>(m, "CVRPArcData")
        .def_property_readonly("distance", [](const CVRP_arc_data& data) { return data.distance; });
}