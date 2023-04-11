#include <routingblocks/FRVCP.h>
#include <routingblocks/LocalSearch.h>
#include <routingblocks/Solution.h>

#include <algorithm>
#include <chrono>
#include <set>
#include <vector>

namespace routingblocks {
    std::shared_ptr<Move> LocalSearch::_explore_neighborhood() {
        std::shared_ptr<Move> next_move, best_move;
        // Discard moves that do not have an impact on the objective function to avoid
        // routing errors.
        auto best_cost = -1e-2;
        for (auto& next_op : _operators) {
            next_op->prepare_search(_current_solution);
            while (true) {
                next_move = next_op->find_next_improving_move(*_evaluation, _current_solution,
                                                              next_move.get());
                if (next_move == nullptr) {
                    break;
                } else if (auto cost = _test_move(*next_move); cost < best_cost) {
                    if (!use_best_improvement) {
                        return next_move;
                    }
                    best_cost = cost;
                    best_move = std::move(next_move);
                }
            }
            next_op->finalize_search();
        }
        return best_move;
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
                             std::shared_ptr<eval_t> exact_evaluation)
        : _instance(&instance),
          _evaluation(std::move(evaluation)),
          _exact_evaluation(std::move(exact_evaluation)),
          _current_solution(_evaluation, *_instance, _instance->FleetSize()) {}

}  // namespace routingblocks
