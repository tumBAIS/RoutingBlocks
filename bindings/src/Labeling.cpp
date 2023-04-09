#include <pybind11/stl.h>
#include <routingblocks/ADPTWEvaluation.h>
#include <routingblocks/FRVCP.h>
#include <routingblocks/Instance.h>
#include <routingblocks_bindings/Labeling.h>

#include <routingblocks_bindings/binding_helpers.hpp>

namespace routingblocks {
    template <> class Propagator<pybind11::object> {
      public:
        using value_type = pybind11::object;
        virtual std::optional<value_type> propagate(const value_type& predecessor,
                                                    const Vertex& origin, const Vertex& target,
                                                    const Arc& arc)
            = 0;

        virtual bool dominates(const value_type& label, const value_type& other) = 0;

        virtual bool cheaper_than(const value_type& label, const value_type& other) = 0;

        virtual bool should_order_before(const value_type& label, const value_type& other) = 0;

        virtual std::vector<VertexID> extract_path(const value_type& sink_label) = 0;

        virtual bool is_final_label(const value_type& _label) = 0;

        virtual void prepare(const std::vector<VertexID>&) = 0;

        virtual value_type create_root_label() = 0;
    };
    using PyPropagator = Propagator<pybind11::object>;
}  // namespace routingblocks

BIND_LIFETIME_PYTHON(routingblocks::Propagator<pybind11::object>, "Propagator")

namespace routingblocks::bindings {
    class PyPropagatorTramboline : public PyPropagator {
      public:
        using value_type = PyPropagator::value_type;
        using PyPropagator::PyPropagator;

        std::optional<value_type> propagate(const value_type& predecessor, const Vertex& origin,
                                            const Vertex& target, const Arc& arc) override {
            PYBIND11_OVERRIDE_PURE(std::optional<value_type>, PyPropagator, propagate, predecessor,
                                   origin, target, arc);
        }

        bool dominates(const value_type& label, const value_type& other) override {
            PYBIND11_OVERRIDE_PURE(bool, PyPropagator, dominates, label, other);
        }

        bool cheaper_than(const value_type& label, const value_type& other) override {
            PYBIND11_OVERRIDE_PURE(bool, PyPropagator, cheaper_than, label, other);
        }

        bool should_order_before(const value_type& label, const value_type& other) override {
            PYBIND11_OVERRIDE_PURE(bool, PyPropagator, should_order_before, label, other);
        }

        std::vector<VertexID> extract_path(const value_type& sink_label) override {
            PYBIND11_OVERRIDE_PURE(std::vector<VertexID>, PyPropagator, extract_path, sink_label);
        }

        bool is_final_label(const value_type& _label) override {
            PYBIND11_OVERRIDE_PURE(bool, PyPropagator, is_final_label, _label);
        }

        void prepare(const std::vector<VertexID>& vector) override {
            PYBIND11_OVERRIDE_PURE(void, PyPropagator, prepare, vector);
        }

        value_type create_root_label() override {
            PYBIND11_OVERRIDE_PURE(value_type, PyPropagator, create_root_label, );
        }
    };

    using PyFRVCP = routingblocks::FRVCP<pybind11::object>;

    template <class PropagatorClass> auto bind_propagator(auto& propagator) {
        return propagator.def("propagate", &PropagatorClass::propagate)
            .def("dominates", &PropagatorClass::dominates, "Returns true if label dominates other.")
            .def("cheaper_than", &PropagatorClass::cheaper_than,
                 "Returns true if label is cheaper than other, i.e., has lower cost.")
            .def("extract_path", &PropagatorClass::extract_path,
                 "Extracts the path taken by the label.")
            .def("order_before", &PropagatorClass::should_order_before,
                 "Returns true if label should be ordered before other.")
            .def("is_final_label", &PropagatorClass::is_final_label,
                 "Returns true if the label is final, i.e., the path is complete.")
            .def("prepare", &PropagatorClass::prepare, "Prepares the propagator for a new route.")
            .def("create_root_label", &PropagatorClass::create_root_label,
                 "Creates the root label for the propagator.");
    }

    void bind_labeling(pybind11::module_& m) {
        auto adptw = m.def_submodule("adptw");
        pybind11::class_<FRVCP<ADPTWLabel>>(adptw, "FRVCP")
            .def(pybind11::init<>([](const Instance& instance, resource_t battery_capacity) {
                return FRVCP<ADPTWLabel>(
                    instance, std::make_shared<Propagator<ADPTWLabel>>(instance, battery_capacity));
            }))
            .def("optimize", &FRVCP<ADPTWLabel>::optimize, "Solve FRVCP for the specified route.");

        auto propagator_interface
            = pybind11::class_<PyPropagator, PyPropagatorTramboline, std::shared_ptr<PyPropagator>>(
                  m, "Propagator")
                  .def(pybind11::init<>());
        bind_propagator<PyPropagator>(propagator_interface);

        pybind11::class_<FRVCP<PyPropagator::value_type>>(m, "FRVCP")
            .def(pybind11::init<const Instance&, std::shared_ptr<PyPropagator>>())
            .def("optimize", &FRVCP<PyPropagator::value_type>::optimize,
                 "Solve FRVCP for the specified route.");

        /*auto propagator_interface
            = bind_propagator<routingblocks::Propagator, PyPropagator>(m, "Propagator")
                  .def(pybind11::init<>());

        bind_propagator<routingblocks::ADPTWPropagation>(m, "ADPTWPropagation",
        propagator_interface) .def(pybind11::init<const routingblocks::Instance&>());

        m.def("create_adptw_propagator", [](const routingblocks::Instance& instance) {
            return std::make_shared<routingblocks::ADPTWPropagation>(instance);
        });

        pybind11::class_<routingblocks::Label, std::shared_ptr<routingblocks::Label>>(m, "DPLabel",
                                                                        pybind11::dynamic_attr())
            .def(pybind11::init<>());

        */
    }

}  // namespace routingblocks::bindings