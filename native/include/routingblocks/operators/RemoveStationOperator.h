/*
 * Copyright (c) 2023 Patrick S. Klein (@libklein)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef routingblocks_REMOVESTATIONOPERATOR_H
#define routingblocks_REMOVESTATIONOPERATOR_H

#include <routingblocks/Instance.h>
#include <routingblocks/LocalSearch.h>
#include <routingblocks/Solution.h>
#include <routingblocks/evaluation.h>
#include <routingblocks/operators/InsertStationOperator.h>

namespace routingblocks {
    class RemoveStationOperator;

    class RemoveStationMove : public Move {
        friend RemoveStationOperator;
        NodeLocation _node;

      public:
        RemoveStationMove(const NodeLocation& node) : _node(node) {}

        cost_t get_cost_delta(Evaluation& evaluation, const Instance& instance,
                              const Solution& solution) const override {
            auto [route, removed_node] = to_iter(_node, solution);
            auto succ_node = std::next(removed_node);

            cost_t cost
                = concatenate(evaluation, instance, route_segment{route->begin(), removed_node},
                              route_segment{succ_node, route->end()});
            return cost - route->cost();
        };
        void apply(const Instance&, Solution& solution) const override {
            auto [route, node] = to_iter(_node, solution);
            solution.remove_vertex(route, node);
        };
    };

    class RemoveStationOperator : public Operator {
        const Instance& _instance;

        [[nodiscard]] SolutionArcIterator _recover_move(const Solution& solution,
                                                        const RemoveStationMove* move) const {
            if (move) {
                auto [route, after_node] = to_iter(move->_node, solution);
                auto arc_iterator = SolutionArcIterator(solution, {route, after_node});
                return arc_iterator;
            } else {
                return SolutionArcIterator(solution, {solution.begin(), solution.begin()->begin()});
            }
        };

      public:
        explicit RemoveStationOperator(const Instance& instance) : _instance(instance) {}

        void prepare_search(const Solution&) override{};
        std::shared_ptr<Move> find_next_improving_move(eval_t& evaluation, const Solution& solution,
                                                       const Move* previous_move) override {
            SolutionArcIterator arc_iterator_end;
            auto arc_iterator
                = _recover_move(solution, static_cast<const RemoveStationMove*>(previous_move));

            for (; arc_iterator != arc_iterator_end; ++arc_iterator) {
                if (!arc_iterator->target_node->vertex().station()) {
                    continue;
                }

                RemoveStationMove move(
                    location_cast(solution, arc_iterator->route, arc_iterator->target_node));

                if (move.get_cost_delta(evaluation, _instance, solution) < 0.) {
                    return std::make_shared<RemoveStationMove>(move);
                }
            }

            return nullptr;
        };
        void finalize_search() override{};
    };

}  // namespace routingblocks
#endif  // routingblocks_REMOVESTATIONOPERATOR_H
