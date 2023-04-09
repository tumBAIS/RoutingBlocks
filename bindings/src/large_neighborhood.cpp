#include <pybind11/stl.h>
#include <routingblocks/Instance.h>
#include <routingblocks/lns_operators.h>
#include <routingblocks/operators.h>
#include <routingblocks_bindings/large_neighborhood.h>

#include <routingblocks/adaptive_large_neighborhood.hpp>
#include <routingblocks_bindings/binding_helpers.hpp>

/*
 * Couple the lifetime of objects created in python to the shared_ptr lifetime in c++.
 */

BIND_LIFETIME_PYTHON(routingblocks::repair_operator, "RepairOperator")
BIND_LIFETIME_PYTHON(routingblocks::destroy_operator, "DestroyOperator")

namespace routingblocks::bindings {

    class py_repair_operator : public routingblocks::repair_operator {
      public:
        using routingblocks::repair_operator::repair_operator;

        void apply(Evaluation& evaluation, Solution& sol,
                   const std::vector<routingblocks::VertexID>& missing_vertices) override {
            PYBIND11_OVERRIDE_PURE(void, routingblocks::repair_operator, apply, evaluation, sol,
                                   missing_vertices);
        }
        std::string_view name() const override {
            PYBIND11_OVERRIDE_PURE(std::string_view, routingblocks::repair_operator, name, );
        }

        bool can_apply_to(const routingblocks::Solution& sol) const override {
            PYBIND11_OVERRIDE_PURE(bool, routingblocks::repair_operator, can_apply_to, sol);
        }
    };

    auto bind_repair_operator_interface(pybind11::module_& m) {
        return pybind11::class_<routingblocks::repair_operator, py_repair_operator,
                                std::shared_ptr<routingblocks::repair_operator>>(m,
                                                                                 "RepairOperator")
            .def(pybind11::init<>())
            .def("apply", &routingblocks::repair_operator::apply,
                 "Apply the repair operator to the passed solution.")
            .def("name", &routingblocks::repair_operator::name,
                 "Returns the name of the repair operator.")
            .def("can_apply_to", &routingblocks::repair_operator::can_apply_to,
                 "Returns true if the repair operator can be applied to the passed solution. False "
                 "otherwise.");
    }

    class py_destroy_operator : public routingblocks::destroy_operator {
      public:
        using routingblocks::destroy_operator::destroy_operator;

        std::vector<routingblocks::VertexID> apply(Evaluation& evaluation,
                                                   routingblocks::Solution& sol,
                                                   size_t numberOfRemovedCustomers) override {
            PYBIND11_OVERRIDE_PURE(std::vector<routingblocks::VertexID>,
                                   routingblocks::destroy_operator, apply, evaluation, sol,
                                   numberOfRemovedCustomers);
        }
        std::string_view name() const override {
            PYBIND11_OVERRIDE_PURE(std::string_view, routingblocks::destroy_operator, name, );
        }

        bool can_apply_to(const routingblocks::Solution& sol) const override {
            PYBIND11_OVERRIDE_PURE(bool, routingblocks::destroy_operator, can_apply_to, sol);
        };
    };

    auto bind_destroy_operator_interface(pybind11::module_& m) {
        return pybind11::class_<routingblocks::destroy_operator, py_destroy_operator,
                                std::shared_ptr<routingblocks::destroy_operator>>(m,
                                                                                  "DestroyOperator")
            .def(pybind11::init<>())
            .def("apply", &routingblocks::destroy_operator::apply,
                 "Apply the destroy operator to the passed solution and return the id's of any "
                 "removed vertices. May contain the same vertex several times.")
            .def("name", &routingblocks::destroy_operator::name,
                 "Returns the name of the destroy operator.")
            .def("can_apply_to", &routingblocks::destroy_operator::can_apply_to,
                 "Returns true if the destroy operator can be applied to the passed solution. "
                 "False otherwise.");
    }

    void bind_helpers(pybind11::module_& m) {
        m.def("sample_positions", &routingblocks::lns::operators::sample_positions,
              "Samples k positions with replacement from the solution. Can optionally include the "
              "start depot.");
    }

    auto bind_random_destory_operator(pybind11::module_& m, auto& interface) {
        using _operator = routingblocks::lns::operators::RandomRemoval;
        return pybind11::class_<_operator, std::shared_ptr<_operator>>(m, "RandomRemoveOperator",
                                                                       interface)
            .def(pybind11::init<routingblocks::utility::random&>())
            .def("apply", &_operator::apply, "Remove random vertices from the solution.")
            .def("name", &_operator::name)
            .def("can_apply_to", &_operator::can_apply_to,
                 "Returns true. Random remove is always possible.");
    }

    void bind_random_insertion_operator(pybind11::module_& m, auto& interface) {
        using _operator = routingblocks::lns::operators::RandomInsertion;
        pybind11::class_<_operator, std::shared_ptr<_operator>>(m, "RandomInsertionOperator",
                                                                interface)
            .def(pybind11::init<routingblocks::utility::random&>())
            .def("apply", &_operator ::apply,
                 "Inserts the passed vertices in order at random locations.")
            .def("name", &_operator ::name)
            .def("can_apply_to", &_operator ::can_apply_to,
                 "Return true: random insertion is always possible.");
    }

    void bind_large_neighborhood(pybind11::module_& m) {
        using lns_t = routingblocks::adaptive_large_neighborhood;
        using destroy_operator_t = lns_t::destroy_operator_type;
        using repair_operator_t = lns_t::repair_operator_type;
        bind_helpers(m);
        pybind11::class_<routingblocks::adaptive_large_neighborhood>(m, "AdaptiveLargeNeighborhood")
            .def(pybind11::init<routingblocks::utility::random, double>())
            .def(
                "generate",
                [](lns_t& lns, Evaluation& evaluation, Solution& sol,
                   size_t num_removed_customers) {
                    auto operator_pick = lns.generate(evaluation, sol, num_removed_customers);
                    return std::make_pair(*operator_pick.first, *operator_pick.second);
                },
                "Generates a solution from the neighborhood of the passed solution using the "
                "configured operators.",
                pybind11::return_value_policy::reference_internal)
            .def(
                "add_repair_operator",
                [](lns_t& lns, const repair_operator_t& repair_operator) {
                    return *(lns.add_operator(repair_operator));
                },
                "Adds the passed repair operator to the large neighborhood.",
                pybind11::return_value_policy::reference_internal)
            .def(
                "add_destroy_operator",
                [](lns_t& lns, const destroy_operator_t& destroy_operator) {
                    return *(lns.add_operator(destroy_operator));
                },
                "Adds the passed destroy operator to the large neighborhood.",
                pybind11::return_value_policy::reference_internal)
            .def(
                "remove_repair_operator",
                [](lns_t& lns, const repair_operator_t& repair_operator) {
                    auto iter = std::find(lns.repair_operators_begin(), lns.repair_operators_end(),
                                          repair_operator);
                    assert(iter != lns.repair_operators_end());
                    lns.remove_operator(iter);
                },
                "Removes the referenced repair operator from the large neighborhood.")
            .def(
                "remove_destroy_operator",
                [](lns_t& lns, const destroy_operator_t& destroy_operator) {
                    auto iter = std::find(lns.destroy_operators_begin(),
                                          lns.destroy_operators_end(), destroy_operator);
                    assert(iter != lns.destroy_operators_end());
                    lns.remove_operator(iter);
                },
                "Removes the references destroy operator from the large neighborhood.")
            .def_property_readonly(
                "destroy_operators",
                [](const routingblocks::adaptive_large_neighborhood& routingblocks) {
                    return pybind11::make_iterator(routingblocks.destroy_operators_begin(),
                                                   routingblocks.destroy_operators_end());
                })
            .def_property_readonly(
                "repair_operators",
                [](const routingblocks::adaptive_large_neighborhood& routingblocks) {
                    return pybind11::make_iterator(routingblocks.repair_operators_begin(),
                                                   routingblocks.repair_operators_end());
                })
            .def("reset_operator_weights",
                 &routingblocks::adaptive_large_neighborhood::reset_operator_weights,
                 "Sets the weights of all operators to 1 and resets collected scores.")
            .def("adapt_operator_weights",
                 &routingblocks::adaptive_large_neighborhood::adapt_operator_weights,
                 "Adapts the weights of all operators based on the recorded performance. Resets "
                 "collected scores.")
            .def(
                "collect_score",
                [](lns_t& neighborhood, const destroy_operator_t& destroy_operator,
                   const repair_operator_t& repair_operator, double score) {
                    neighborhood.collect_score(
                        std::make_pair(
                            std::find(neighborhood.destroy_operators_begin(),
                                      neighborhood.destroy_operators_end(), destroy_operator),
                            std::find(neighborhood.repair_operators_begin(),
                                      neighborhood.repair_operators_end(), repair_operator)),
                        score);
                },
                "Collects the score archived by the selected operators.");

        auto destroy_operator_interface = bind_destroy_operator_interface(m);
        auto repair_operator_interface = bind_repair_operator_interface(m);

        bind_random_insertion_operator(m, repair_operator_interface);
        bind_random_destory_operator(m, destroy_operator_interface);
    }

}  // namespace routingblocks::bindings
