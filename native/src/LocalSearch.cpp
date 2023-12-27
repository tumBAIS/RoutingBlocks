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

#include <routingblocks/FRVCP.h>
#include <routingblocks/LocalSearch.h>
#include <routingblocks/Solution.h>

#include <set>

namespace routingblocks {
    std::shared_ptr<Move> LocalSearch::_explore_neighborhood() {
        std::shared_ptr<Move> next_move;
        // Discard moves that do not have an impact on the objective function to avoid
        // routing errors.
        bool skip_remaining_operators = false;
        for (auto& next_op : _operators) {
            next_op->prepare_search(_current_solution);
            while (true) {
                next_move = next_op->find_next_improving_move(*_evaluation, _current_solution,
                                                              next_move.get());
                if (next_move == nullptr) {
                    break;
                } else if (auto cost = _test_move(*next_move); cost < -1e-2) {
                    if (!_pivoting_rule->continue_search(next_move, cost, _current_solution)) {
                        skip_remaining_operators = true;
                        break;
                    }
                }
            }
            next_op->finalize_search();
            if (skip_remaining_operators) {
                break;
            }
        }
        return _pivoting_rule->select_move(_current_solution);
    }

    cost_t LocalSearch::_test_move(const Move& move) {
        if (_exact_evaluation) {
            return move.get_cost_delta(*_exact_evaluation, *_instance, _current_solution);
        } else {
            Solution sol = _current_solution;
            move.apply(*_instance, sol);
            return sol.cost() - _current_solution.cost();
        }
    }

    void LocalSearch::_apply_move(const Move& move) { move.apply(*_instance, _current_solution); }

    LocalSearch::LocalSearch(const routingblocks::Instance& instance,
                             std::shared_ptr<eval_t> evaluation,
                             std::shared_ptr<eval_t> exact_evaluation, PivotingRule* pivoting_rule)
        : _instance(&instance),
          _evaluation(std::move(evaluation)),
          _exact_evaluation(std::move(exact_evaluation)),
          _pivoting_rule(pivoting_rule),
          _current_solution(_evaluation, *_instance, _instance->FleetSize()) {}

}  // namespace routingblocks
