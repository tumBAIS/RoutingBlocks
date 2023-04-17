#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <routingblocks/LocalSearch.h>
#include <routingblocks/utility/random.h>
#include <routingblocks_bindings/LocalSearch.h>

namespace routingblocks::bindings {

    class PivotingRuleTrampoline : public PivotingRule {
      public:
        using PivotingRule::PivotingRule;

        std::shared_ptr<Move> select_move(const Solution& solution) override {
            PYBIND11_OVERLOAD_PURE(std::shared_ptr<Move>, PivotingRule, select_move, solution);
        }

        bool continue_search(const std::shared_ptr<Move>& move, cost_t cost,
                             const Solution& solution) override {
            PYBIND11_OVERLOAD_PURE(bool, PivotingRule, continue_search, move, cost, solution);
        }
    };

    void bind_pivoting_rule(pybind11::module_& m) {
        pybind11::class_<PivotingRule, PivotingRuleTrampoline>(m, "PivotingRule")
            .def(pybind11::init<>())
            .def("select_move", &PivotingRule::select_move)
            .def("continue_search", &PivotingRule::continue_search);

        pybind11::class_<BestImprovementPivotingRule, PivotingRule>(m,
                                                                    "BestImprovementPivotingRule")
            .def(pybind11::init<>())
            .def("select_move", &BestImprovementPivotingRule::select_move)
            .def("continue_search", &BestImprovementPivotingRule::continue_search);

        pybind11::class_<KBestImprovementPivotingRule, PivotingRule>(m,
                                                                     "KBestImprovementPivotingRule")
            .def(pybind11::init<size_t>())
            .def("select_move", &KBestImprovementPivotingRule::select_move)
            .def("continue_search", &KBestImprovementPivotingRule::continue_search);

        pybind11::class_<FirstImprovementPivotingRule, PivotingRule>(m,
                                                                     "FirstImprovementPivotingRule")
            .def(pybind11::init<>())
            .def("select_move", &FirstImprovementPivotingRule::select_move)
            .def("continue_search", &FirstImprovementPivotingRule::continue_search);
    }

    void bind_local_search(pybind11::module& m) {
        pybind11::class_<routingblocks::LocalSearch>(m, "LocalSearch")
            .def(pybind11::init<const routingblocks::Instance&, std::shared_ptr<Evaluation>,
                                std::shared_ptr<Evaluation>, PivotingRule*>(),
                 pybind11::keep_alive<1, 5>())
            .def(
                "optimize",
                [](LocalSearch& ls, Solution& sol, std::vector<Operator*> operators) -> void {
                    ls.run(sol, operators.begin(), operators.end());
                },
                "Optimizes the passed solution inplace.");
    }

    template <class T> void bind_generator_arc(pybind11::module& m, const char* name) {
        pybind11::class_<T>(m, name)
            .def(pybind11::init([](const Solution& solution, int origin_route, int origin_node,
                                   int target_route, int target_node) {
                auto origin_route_iterator = std::next(solution.begin(), origin_route);
                auto target_route_iterator = std::next(solution.begin(), target_route);
                auto origin_node_iterator = std::next(origin_route_iterator->begin(), origin_node);
                auto target_node_iterator = std::next(target_route_iterator->begin(), target_node);
                return GeneratorArc{origin_route_iterator, origin_node_iterator,
                                    target_route_iterator, target_node_iterator};
            }))
            .def(pybind11::init([](const Solution& solution, NodeLocation origin_location,
                                   NodeLocation target_location) {
                auto [origin_route_iterator, origin_node_iterator]
                    = routingblocks::to_iter(origin_location, solution);
                auto [target_route_iterator, target_node_iterator]
                    = routingblocks::to_iter(target_location, solution);
                return GeneratorArc{origin_route_iterator, origin_node_iterator,
                                    target_route_iterator, target_node_iterator};
            }))
            .def_property_readonly("origin_route",
                                   [](const routingblocks::GeneratorArc& arc) -> const Route& {
                                       return *arc.origin_route;
                                   })
            .def_property_readonly("target_route",
                                   [](const routingblocks::GeneratorArc& arc) -> const Route& {
                                       return *arc.target_route;
                                   })
            .def_property_readonly("origin_node",
                                   [](const routingblocks::GeneratorArc& arc) -> const Node& {
                                       return *arc.origin_node;
                                   })
            .def_property_readonly("target_node",
                                   [](const routingblocks::GeneratorArc& arc) -> const Node& {
                                       return *arc.target_node;
                                   });
    }

    void bind_neighborhood_structures(pybind11::module& m) {
        bind_generator_arc<GeneratorArc>(m, "GeneratorArc");

        pybind11::class_<QuadraticNeighborhoodIterator>(m, "QuadraticNeighborhoodIterator");
        m.def("iter_neighborhood", [](const Solution& solution) {
            return pybind11::make_iterator(
                QuadraticNeighborhoodIterator(
                    solution, {solution.begin(), solution.begin()->begin(), solution.begin(),
                               solution.begin()->begin()}),
                QuadraticNeighborhoodIterator());
        });
    }

}  // namespace routingblocks::bindings
