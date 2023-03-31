#include "vrpis_bindings/large_neighborhood.h"

#include <pybind11/stl.h>
#include <vrpis/Instance.h>
#include <vrpis/lns_operators.h>

#include <vrpis/adaptive_large_neighborhood.hpp>

#include "vrpis/operators.h"
#include "vrpis_bindings/binding_helpers.hpp"

/*
 * Couple the lifetime of objects created in python to the shared_ptr lifetime in c++.
 */

BIND_LIFETIME_PYTHON(vrpis::repair_operator, "RepairOperator")
BIND_LIFETIME_PYTHON(vrpis::destroy_operator, "DestroyOperator")

namespace vrpis::bindings {

    class py_repair_operator : public vrpis::repair_operator {
      public:
        using vrpis::repair_operator::repair_operator;

        void apply(Evaluation& evaluation, Solution& sol,
                   const std::vector<vrpis::VertexID>& missing_vertices) override {
            PYBIND11_OVERRIDE_PURE(void, vrpis::repair_operator, apply, evaluation, sol,
                                   missing_vertices);
        }
        std::string_view name() const override {
            PYBIND11_OVERRIDE_PURE(std::string_view, vrpis::repair_operator, name, );
        }

        bool can_apply_to(const vrpis::Solution& sol) const override {
            PYBIND11_OVERRIDE_PURE(bool, vrpis::repair_operator, can_apply_to, sol);
        }
    };

    auto bind_repair_operator_interface(pybind11::module_& m) {
        return pybind11::class_<vrpis::repair_operator, py_repair_operator,
                                std::shared_ptr<vrpis::repair_operator>>(m, "RepairOperator")
            .def(pybind11::init<>())
            .def("apply", &vrpis::repair_operator::apply,
                 "Apply the repair operator to the passed solution.")
            .def("name", &vrpis::repair_operator::name, "Returns the name of the repair operator.")
            .def("can_apply_to", &vrpis::repair_operator::can_apply_to,
                 "Returns true if the repair operator can be applied to the passed solution. False "
                 "otherwise.");
    }

    class py_destroy_operator : public vrpis::destroy_operator {
      public:
        using vrpis::destroy_operator::destroy_operator;

        std::vector<vrpis::VertexID> apply(Evaluation& evaluation, vrpis::Solution& sol,
                                           size_t numberOfRemovedCustomers) override {
            PYBIND11_OVERRIDE_PURE(std::vector<vrpis::VertexID>, vrpis::destroy_operator, apply,
                                   evaluation, sol, numberOfRemovedCustomers);
        }
        std::string_view name() const override {
            PYBIND11_OVERRIDE_PURE(std::string_view, vrpis::destroy_operator, name, );
        }

        bool can_apply_to(const vrpis::Solution& sol) const override {
            PYBIND11_OVERRIDE_PURE(bool, vrpis::destroy_operator, can_apply_to, sol);
        };
    };

    auto bind_destroy_operator_interface(pybind11::module_& m) {
        return pybind11::class_<vrpis::destroy_operator, py_destroy_operator,
                                std::shared_ptr<vrpis::destroy_operator>>(m, "DestroyOperator")
            .def(pybind11::init<>())
            .def("apply", &vrpis::destroy_operator::apply,
                 "Apply the destroy operator to the passed solution and return the id's of any "
                 "removed vertices. May contain the same vertex several times.")
            .def("name", &vrpis::destroy_operator::name,
                 "Returns the name of the destroy operator.")
            .def("can_apply_to", &vrpis::destroy_operator::can_apply_to,
                 "Returns true if the destroy operator can be applied to the passed solution. "
                 "False otherwise.");
    }

    void bind_helpers(pybind11::module_& m) {
        m.def("sample_positions", &vrpis::lns::operators::sample_positions,
              "Samples k positions with replacement from the solution. Can optionally include the "
              "start depot.");
    }

    auto bind_random_destory_operator(pybind11::module_& m, auto& interface) {
        using _operator = vrpis::lns::operators::RandomRemoval;
        return pybind11::class_<_operator, std::shared_ptr<_operator>>(m, "RandomRemoveOperator",
                                                                       interface)
            .def(pybind11::init<vrpis::utility::random&>())
            .def("apply", &_operator::apply, "Remove random vertices from the solution.")
            .def("name", &_operator::name)
            .def("can_apply_to", &_operator::can_apply_to,
                 "Returns true. Random remove is always possible.");
    }

    void bind_random_insertion_operator(pybind11::module_& m, auto& interface) {
        using _operator = vrpis::lns::operators::RandomInsertion;
        pybind11::class_<_operator, std::shared_ptr<_operator>>(m, "RandomInsertionOperator",
                                                                interface)
            .def(pybind11::init<vrpis::utility::random&>())
            .def("apply", &_operator ::apply,
                 "Inserts the passed vertices in order at random locations.")
            .def("name", &_operator ::name)
            .def("can_apply_to", &_operator ::can_apply_to,
                 "Return true: random insertion is always possible.");
    }

    void bind_large_neighborhood(pybind11::module_& m) {
        using lns_t = vrpis::adaptive_large_neighborhood;
        using destroy_operator_t = lns_t::destroy_operator_type;
        using repair_operator_t = lns_t::repair_operator_type;
        bind_helpers(m);
        pybind11::class_<vrpis::adaptive_large_neighborhood>(m, "AdaptiveLargeNeighborhood")
            .def(pybind11::init<vrpis::utility::random, double>())
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
            .def_property_readonly("destroy_operators",
                                   [](const vrpis::adaptive_large_neighborhood& vrpis) {
                                       return pybind11::make_iterator(
                                           vrpis.destroy_operators_begin(),
                                           vrpis.destroy_operators_end());
                                   })
            .def_property_readonly("repair_operators",
                                   [](const vrpis::adaptive_large_neighborhood& vrpis) {
                                       return pybind11::make_iterator(
                                           vrpis.repair_operators_begin(),
                                           vrpis.repair_operators_end());
                                   })
            .def("reset_operator_weights",
                 &vrpis::adaptive_large_neighborhood::reset_operator_weights,
                 "Sets the weights of all operators to 1 and resets collected scores.")
            .def("adapt_operator_weights",
                 &vrpis::adaptive_large_neighborhood::adapt_operator_weights,
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

}  // namespace vrpis::bindings
