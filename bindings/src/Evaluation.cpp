#include <pybind11/stl.h>
#include <routingblocks/ADPTWEvaluation.h>
#include <routingblocks/NIFTWEvaluation.h>
#include <routingblocks/evaluation.h>
#include <routingblocks_bindings/Evaluation.h>

#include <routingblocks_bindings/binding_helpers.hpp>

namespace routingblocks::bindings {

    class PyEvaluation : public routingblocks::Evaluation {
      protected:
        using py_type = pybind11::object;
        using py_segment_node_type = std::tuple<const Vertex*, py_type, py_type>;
        using py_segment_type = std::vector<py_segment_node_type>;

      public:
        using routingblocks::Evaluation::Evaluation;

        virtual cost_t py_evaluate(const Instance& instance, std::vector<py_segment_type> segments)
            = 0;

        virtual cost_t py_compute_cost(const py_type& label) const = 0;
        virtual bool py_is_feasible(const py_type& label) const = 0;

        virtual py_type py_propagate_forward(const py_type& pred_label, const Vertex& pred_vertex,
                                             const Vertex& vertex, const Arc& arc) const
            = 0;

        virtual py_type py_propagate_backward(const py_type& succ_label, const Vertex& succ_vertex,
                                              const Vertex& vertex, const Arc& arc) const
            = 0;

        virtual py_type py_create_forward_label(const Vertex& vertex) = 0;

        virtual py_type py_create_backward_label(const Vertex& vertex) = 0;
        virtual std::vector<resource_t> py_get_cost_components(const py_type& label) const = 0;

      private:
      public:
        cost_t evaluate(const Instance& instance,
                        std::span<const route_segment> segments) override {
            // TODO Figure out a better way to do this
            std::vector<py_segment_type> py_segments(segments.size());
            std::transform(segments.begin(), segments.end(), py_segments.begin(),
                           [](const route_segment segment) {
                               py_segment_type segment_nodes(segment.size());
                               std::transform(segment.begin(), segment.end(), segment_nodes.begin(),
                                              [](const Node& node) {
                                                  return std::make_tuple(
                                                      &node.vertex(),
                                                      node.forward_label().get<py_type>(),
                                                      node.backward_label().get<py_type>());
                                              });
                               return segment_nodes;
                           });
            return py_evaluate(instance, std::move(py_segments));
        }

      private:
        cost_t compute_cost(const Evaluation::label_holder_t& label) const final {
            return py_compute_cost(label.get<py_type>());
        }

        bool is_feasible(const Evaluation::label_holder_t& label) const final {
            return py_is_feasible(label.get<py_type>());
        }

        Evaluation::label_holder_t propagate_forward(const Evaluation::label_holder_t& pred_label,
                                                     const Vertex& pred_vertex,
                                                     const Vertex& vertex,
                                                     const Arc& arc) const final {
            return Evaluation::label_holder_t(std::make_shared<py_type>(
                py_propagate_forward(pred_label.get<py_type>(), pred_vertex, vertex, arc)));
        }

        Evaluation::label_holder_t propagate_backward(const Evaluation::label_holder_t& succ_label,
                                                      const Vertex& succ_vertex,
                                                      const Vertex& vertex,
                                                      const Arc& arc) const final {
            return Evaluation::label_holder_t(std::make_shared<py_type>(
                py_propagate_backward(succ_label.get<py_type>(), succ_vertex, vertex, arc)));
        }

        Evaluation::label_holder_t create_forward_label(const Vertex& vertex) final {
            return Evaluation::label_holder_t(
                std::make_shared<py_type>(py_create_forward_label(vertex)));
        }

        Evaluation::label_holder_t create_backward_label(const Vertex& vertex) final {
            return Evaluation::label_holder_t(
                std::make_shared<py_type>(py_create_backward_label(vertex)));
        }

        std::vector<resource_t> get_cost_components(
            const Evaluation::label_holder_t& label) const final {
            return py_get_cost_components(label.get<py_type>());
        }
    };

    class PyConcatenationBasedEvaluation : public routingblocks::ConcatenationBasedEvaluation {
      protected:
        using py_type = pybind11::object;

      public:
        using routingblocks::ConcatenationBasedEvaluation::ConcatenationBasedEvaluation;

        virtual cost_t py_concatenate(const py_type& fwd, const py_type& bwd, const Vertex& vertex)
            = 0;

        virtual cost_t py_compute_cost(const py_type& label) const = 0;
        virtual bool py_is_feasible(const py_type& label) const = 0;

        virtual py_type py_propagate_forward(const py_type& pred_label, const Vertex& pred_vertex,
                                             const Vertex& vertex, const Arc& arc) const
            = 0;

        virtual py_type py_propagate_backward(const py_type& succ_label, const Vertex& succ_vertex,
                                              const Vertex& vertex, const Arc& arc) const
            = 0;

        virtual py_type py_create_forward_label(const Vertex& vertex) = 0;

        virtual py_type py_create_backward_label(const Vertex& vertex) = 0;
        virtual std::vector<resource_t> py_get_cost_components(const py_type& label) const = 0;

      private:
        cost_t concatenate(const ConcatenationBasedEvaluation::label_holder_t& fwd,
                           const ConcatenationBasedEvaluation::label_holder_t& bwd,
                           const Vertex& vertex) final {
            return py_concatenate(fwd.get<py_type>(), bwd.get<py_type>(), vertex);
        }

        cost_t compute_cost(const ConcatenationBasedEvaluation::label_holder_t& label) const final {
            return py_compute_cost(label.get<py_type>());
        }

        bool is_feasible(const ConcatenationBasedEvaluation::label_holder_t& label) const final {
            return py_is_feasible(label.get<py_type>());
        }

        ConcatenationBasedEvaluation::label_holder_t propagate_forward(
            const ConcatenationBasedEvaluation::label_holder_t& pred_label,
            const Vertex& pred_vertex, const Vertex& vertex, const Arc& arc) const final {
            return ConcatenationBasedEvaluation::label_holder_t(std::make_shared<py_type>(
                py_propagate_forward(pred_label.get<py_type>(), pred_vertex, vertex, arc)));
        }

        ConcatenationBasedEvaluation::label_holder_t propagate_backward(
            const ConcatenationBasedEvaluation::label_holder_t& succ_label,
            const Vertex& succ_vertex, const Vertex& vertex, const Arc& arc) const final {
            return ConcatenationBasedEvaluation::label_holder_t(std::make_shared<py_type>(
                py_propagate_backward(succ_label.get<py_type>(), succ_vertex, vertex, arc)));
        }

        ConcatenationBasedEvaluation::label_holder_t create_forward_label(
            const Vertex& vertex) final {
            return ConcatenationBasedEvaluation::label_holder_t(
                std::make_shared<py_type>(py_create_forward_label(vertex)));
        }

        ConcatenationBasedEvaluation::label_holder_t create_backward_label(
            const Vertex& vertex) final {
            return ConcatenationBasedEvaluation::label_holder_t(
                std::make_shared<py_type>(py_create_backward_label(vertex)));
        }

        std::vector<resource_t> get_cost_components(
            const ConcatenationBasedEvaluation::label_holder_t& label) const final {
            return py_get_cost_components(label.get<py_type>());
        }
    };

}  // namespace routingblocks::bindings

BIND_LIFETIME_PYTHON(routingblocks::Evaluation, "Evaluation")
BIND_LIFETIME_PYTHON(routingblocks::bindings::PyEvaluation, "PyEvaluation")

namespace routingblocks::bindings {

    class PyConcatenationBasedEvaluationTramboline : public PyConcatenationBasedEvaluation {
        using PyConcatenationBasedEvaluation::PyConcatenationBasedEvaluation;

      public:
        cost_t py_concatenate(const py_type& fwd, const py_type& bwd,
                              const Vertex& vertex) override {
            PYBIND11_OVERRIDE_PURE_NAME(cost_t, PyConcatenationBasedEvaluation, "concatenate",
                                        py_concatenate, fwd, bwd, vertex);
        }

        bool py_is_feasible(const py_type& label) const override {
            PYBIND11_OVERRIDE_PURE_NAME(bool, PyConcatenationBasedEvaluation, "is_feasible",
                                        is_feasible, label);
        }

        cost_t py_compute_cost(const py_type& label) const override {
            PYBIND11_OVERRIDE_PURE_NAME(cost_t, PyConcatenationBasedEvaluation, "compute_cost",
                                        py_compute_cost, label);
        }

        py_type py_propagate_forward(const py_type& pred_label, const Vertex& pred_vertex,
                                     const Vertex& vertex, const Arc& arc) const override {
            PYBIND11_OVERRIDE_PURE_NAME(py_type, PyConcatenationBasedEvaluation,
                                        "propagate_forward", py_propagate_forward, pred_label,
                                        pred_vertex, vertex, arc);
        }

        py_type py_propagate_backward(const py_type& succ_label, const Vertex& succ_vertex,
                                      const Vertex& vertex, const Arc& arc) const override {
            PYBIND11_OVERRIDE_PURE_NAME(py_type, PyConcatenationBasedEvaluation,
                                        "propagate_backward", py_propagate_backward, succ_label,
                                        succ_vertex, vertex, arc);
        }

        py_type py_create_forward_label(const Vertex& vertex) override {
            PYBIND11_OVERRIDE_PURE_NAME(py_type, PyConcatenationBasedEvaluation,
                                        "create_forward_label", py_create_forward_label, vertex);
        }

        py_type py_create_backward_label(const Vertex& vertex) override {
            PYBIND11_OVERRIDE_PURE_NAME(py_type, PyConcatenationBasedEvaluation,
                                        "create_backward_label", py_create_backward_label, vertex);
        }

        std::vector<resource_t> py_get_cost_components(const py_type& label) const override {
            PYBIND11_OVERRIDE_PURE_NAME(std::vector<resource_t>, PyConcatenationBasedEvaluation,
                                        "get_cost_components", get_cost_components, label);
        }
    };

    auto bind_evaluation_interface(pybind11::module& m) {
        return pybind11::class_<Evaluation, std::shared_ptr<Evaluation>>(m, "Evaluation");
    }

    void bind_py_evaluation(pybind11::module_& m, auto& evaluation_interface) {
        pybind11::class_<PyConcatenationBasedEvaluation, PyConcatenationBasedEvaluationTramboline,
                         std::shared_ptr<PyConcatenationBasedEvaluation>>(
            m, "PyConcatenationBasedEvaluation", evaluation_interface)
            .def(pybind11::init<>())
            .def("propagate_forward", &PyConcatenationBasedEvaluation::py_propagate_forward)
            .def("propagate_backward", &PyConcatenationBasedEvaluation::py_propagate_backward)
            .def("concatenate", &PyConcatenationBasedEvaluation::py_concatenate)
            .def("compute_cost", &PyConcatenationBasedEvaluation::py_compute_cost)
            .def("is_feasible", &PyConcatenationBasedEvaluation::py_is_feasible)
            .def("create_forward_label", &PyConcatenationBasedEvaluation::py_create_forward_label)
            .def("create_backward_label",
                 &PyConcatenationBasedEvaluation::py_create_backward_label);
    }

    template <class T, class Binding> auto bind_evaluation(Binding& evaluation) {
        using fwd_label_t = typename T::fwd_label_t;
        using bwd_label_t = typename T::bwd_label_t;
        return evaluation.def("propagate_forward", &T::propagate_forward)
            .def("propagate_backward", &T::propagate_backward)
            .def("concatenate", &T::concatenate)
            .def("compute_cost", &T::compute_cost)
            .def("is_feasible", &T::is_feasible)
            .def("get_cost_components", &T::get_cost_components)
            .def("create_forward_label", &T::create_forward_label)
            .def("create_backward_label", &T::create_backward_label);
    }

    void bind_evaluation(pybind11::module& m) {
        auto evaluation_interface = bind_evaluation_interface(m);
        bind_py_evaluation(m, evaluation_interface);

        auto adptw_module = m.def_submodule("adptw");
        bind_evaluation<ADPTWEvaluation>(
            pybind11::class_<ADPTWEvaluation, std::shared_ptr<ADPTWEvaluation>>(
                adptw_module, "Evaluation", evaluation_interface)
                .def(pybind11::init<resource_t, resource_t>()))
            .def_property("penalty_factors", &ADPTWEvaluation::get_penalty_factors,
                          &ADPTWEvaluation::set_penalty_factors);
        adptw_module.attr("DistanceCostComponent")
            = pybind11::int_(static_cast<int>(ADPTWEvaluation::CostComponent::DIST_INDEX));
        adptw_module.attr("OverchargeCostComponent")
            = pybind11::int_(static_cast<int>(ADPTWEvaluation::CostComponent::OVERCHARGE_INDEX));
        adptw_module.attr("OverloadCostComponent")
            = pybind11::int_(static_cast<int>(ADPTWEvaluation::CostComponent::OVERLOAD_INDEX));
        adptw_module.attr("TimeShiftCostComponent")
            = pybind11::int_(static_cast<int>(ADPTWEvaluation::CostComponent::TIME_SHIFT_INDEX));

        auto niftw_module = m.def_submodule("niftw");
        bind_evaluation<NIFTWEvaluation>(
            pybind11::class_<routingblocks::NIFTWEvaluation, std::shared_ptr<NIFTWEvaluation>>(
                niftw_module, "Evaluation", evaluation_interface)
                .def(pybind11::init<resource_t, resource_t, resource_t>()))
            .def_property("penalty_factors", &NIFTWEvaluation::get_penalty_factors,
                          &NIFTWEvaluation::set_penalty_factors);

        niftw_module.attr("DistanceCostComponent")
            = pybind11::int_(static_cast<int>(NIFTWEvaluation::CostComponent::DIST_INDEX));
        niftw_module.attr("OverchargeCostComponent")
            = pybind11::int_(static_cast<int>(NIFTWEvaluation::CostComponent::OVERCHARGE_INDEX));
        niftw_module.attr("OverloadCostComponent")
            = pybind11::int_(static_cast<int>(NIFTWEvaluation::CostComponent::OVERLOAD_INDEX));
        niftw_module.attr("TimeShiftCostComponent")
            = pybind11::int_(static_cast<int>(NIFTWEvaluation::CostComponent::TIME_SHIFT_INDEX));
    }

}  // namespace routingblocks::bindings
