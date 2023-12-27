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
            PYBIND11_OVERRIDE_PURE(std::shared_ptr<Move>, PivotingRule, select_move, &solution);
        }

        bool continue_search(const std::shared_ptr<Move>& move, cost_t cost,
                             const Solution& solution) override {
            PYBIND11_OVERRIDE_PURE(bool, PivotingRule, continue_search, move, cost, &solution);
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
                 pybind11::keep_alive<1, 2>(), pybind11::keep_alive<1, 5>())
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
