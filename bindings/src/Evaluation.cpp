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

#include <pybind11/stl.h>
#include <routingblocks/evaluation.h>
#include <routingblocks_bindings/Evaluation.h>

#include <algorithm>
#include <routingblocks_bindings/binding_helpers.hpp>

namespace routingblocks::bindings {

    class PyEvaluation : public routingblocks::Evaluation {
      protected:
        using py_type = pybind11::object;
        // A tuple of (vertex, forward_label, backward_label)
        using py_segment_node_type = std::tuple<const Vertex*, py_type, py_type>;
        using py_segment_type = std::vector<py_segment_node_type>;

      public:
        using routingblocks::Evaluation::Evaluation;

        virtual cost_t py_evaluate(const Instance& instance,
                                   const std::vector<py_segment_type>& segments)
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
            return py_evaluate(instance, py_segments);
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

namespace routingblocks::bindings {

    class PyConcatenationBasedEvaluationTramboline : public PyConcatenationBasedEvaluation {
        using PyConcatenationBasedEvaluation::PyConcatenationBasedEvaluation;

      public:
        cost_t py_concatenate(const py_type& fwd, const py_type& bwd,
                              const Vertex& vertex) override {
            PYBIND11_OVERRIDE_PURE_NAME(cost_t, PyConcatenationBasedEvaluation, "concatenate",
                                        py_concatenate, &fwd, &bwd, &vertex);
        }

        bool py_is_feasible(const py_type& label) const override {
            PYBIND11_OVERRIDE_PURE_NAME(bool, PyConcatenationBasedEvaluation, "is_feasible",
                                        is_feasible, &label);
        }

        cost_t py_compute_cost(const py_type& label) const override {
            PYBIND11_OVERRIDE_PURE_NAME(cost_t, PyConcatenationBasedEvaluation, "compute_cost",
                                        py_compute_cost, &label);
        }

        py_type py_propagate_forward(const py_type& pred_label, const Vertex& pred_vertex,
                                     const Vertex& vertex, const Arc& arc) const override {
            PYBIND11_OVERRIDE_PURE_NAME(py_type, PyConcatenationBasedEvaluation,
                                        "propagate_forward", py_propagate_ & forward, &pred_label,
                                        &pred_vertex, &vertex, &arc);
        }

        py_type py_propagate_backward(const py_type& succ_label, const Vertex& succ_vertex,
                                      const Vertex& vertex, const Arc& arc) const override {
            PYBIND11_OVERRIDE_PURE_NAME(py_type, PyConcatenationBasedEvaluation,
                                        "propagate_backward", py_propagate_backward, &succ_label,
                                        &succ_vertex, &vertex, &arc);
        }

        py_type py_create_forward_label(const Vertex& vertex) override {
            PYBIND11_OVERRIDE_PURE_NAME(py_type, PyConcatenationBasedEvaluation,
                                        "create_forward_label", py_create_forward_label, &vertex);
        }

        py_type py_create_backward_label(const Vertex& vertex) override {
            PYBIND11_OVERRIDE_PURE_NAME(py_type, PyConcatenationBasedEvaluation,
                                        "create_backward_label", py_create_backward_label, &vertex);
        }

        std::vector<resource_t> py_get_cost_components(const py_type& label) const override {
            PYBIND11_OVERRIDE_PURE_NAME(std::vector<resource_t>, PyConcatenationBasedEvaluation,
                                        "get_cost_components", get_cost_components, &label);
        }
    };

    class PyEvaluationTramboline : public PyEvaluation {
        using PyEvaluation::PyEvaluation;

      public:
        cost_t py_evaluate(const Instance& instance,
                           const std::vector<py_segment_type>& segments) override {
            PYBIND11_OVERRIDE_PURE_NAME(cost_t, PyEvaluation, "evaluate", py_evaluate, &instance);
        }

        bool py_is_feasible(const py_type& label) const override {
            PYBIND11_OVERRIDE_PURE_NAME(bool, PyEvaluation, "is_feasible", is_feasible, &label);
        }

        cost_t py_compute_cost(const py_type& label) const override {
            PYBIND11_OVERRIDE_PURE_NAME(cost_t, PyEvaluation, "compute_cost", py_compute_cost,
                                        &label);
        }

        py_type py_propagate_forward(const py_type& pred_label, const Vertex& pred_vertex,
                                     const Vertex& vertex, const Arc& arc) const override {
            PYBIND11_OVERRIDE_PURE_NAME(py_type, PyEvaluation, "propagate_forward",
                                        py_propagate_forward, &pred_label, &pred_vertex, &vertex,
                                        &arc);
        }

        py_type py_propagate_backward(const py_type& succ_label, const Vertex& succ_vertex,
                                      const Vertex& vertex, const Arc& arc) const override {
            PYBIND11_OVERRIDE_PURE_NAME(py_type, PyEvaluation, "propagate_backward",
                                        py_propagate_backward, &succ_label, &succ_vertex, &vertex,
                                        &arc);
        }

        py_type py_create_forward_label(const Vertex& vertex) override {
            PYBIND11_OVERRIDE_PURE_NAME(py_type, PyEvaluation, "create_forward_label",
                                        py_create_forward_label, &vertex);
        }

        py_type py_create_backward_label(const Vertex& vertex) override {
            PYBIND11_OVERRIDE_PURE_NAME(py_type, PyEvaluation, "create_backward_label",
                                        py_create_backward_label, &vertex);
        }

        std::vector<resource_t> py_get_cost_components(const py_type& label) const override {
            PYBIND11_OVERRIDE_PURE_NAME(std::vector<resource_t>, PyEvaluation,
                                        "get_cost_components", get_cost_components, &label);
        }
    };

    auto bind_evaluation_interface(pybind11::module& m) {
        return pybind11::class_<Evaluation>(m, "Evaluation");
    }

    void bind_py_evaluation(pybind11::module_& m, auto& evaluation_interface) {
        pybind11::class_<PyConcatenationBasedEvaluation, PyConcatenationBasedEvaluationTramboline>(
            m, "PyConcatenationBasedEvaluation", evaluation_interface)
            .def(pybind11::init<>())
            .def("propagate_forward", &PyConcatenationBasedEvaluation::py_propagate_forward)
            .def("propagate_backward", &PyConcatenationBasedEvaluation::py_propagate_backward)
            .def("concatenate", &PyConcatenationBasedEvaluation::py_concatenate)
            .def("compute_cost", &PyConcatenationBasedEvaluation::py_compute_cost)
            .def("get_cost_components", &PyConcatenationBasedEvaluation::py_get_cost_components)
            .def("is_feasible", &PyConcatenationBasedEvaluation::py_is_feasible)
            .def("create_forward_label", &PyConcatenationBasedEvaluation::py_create_forward_label)
            .def("create_backward_label",
                 &PyConcatenationBasedEvaluation::py_create_backward_label);

        pybind11::class_<PyEvaluation, PyEvaluationTramboline>(m, "PyEvaluation",
                                                               evaluation_interface)
            .def(pybind11::init<>())
            .def("propagate_forward", &PyEvaluation::py_propagate_forward)
            .def("propagate_backward", &PyEvaluation::py_propagate_backward)
            .def("concatenate", &PyEvaluation::py_evaluate)
            .def("compute_cost", &PyEvaluation::py_compute_cost)
            .def("get_cost_components", &PyEvaluation::py_get_cost_components)
            .def("is_feasible", &PyEvaluation::py_is_feasible)
            .def("create_forward_label", &PyEvaluation::py_create_forward_label)
            .def("create_backward_label", &PyEvaluation::py_create_backward_label);
    }

    void bind_evaluation(pybind11::module& m) {
        auto evaluation_interface = bind_evaluation_interface(m);
        bind_py_evaluation(m, evaluation_interface);
    }

}  // namespace routingblocks::bindings
