#pragma once

#include <routingblocks/Solution.h>
#include <routingblocks/operators.h>
#include <routingblocks/utility/adaptive_priority_list.h>
#include <routingblocks/utility/random.h>

#include <type_traits>
#include <vector>

namespace routingblocks {
    class adaptive_large_neighborhood {
      private:
        template <typename T> using OperatorList = utility::adaptive_priority_list<T>;

        using destroy_operator_list = OperatorList<std::shared_ptr<destroy_operator>>;
        using repair_operator_list = OperatorList<std::shared_ptr<repair_operator>>;

        routingblocks::utility::random _random;

        destroy_operator_list _destroy_operators;
        repair_operator_list _repair_operators;

        std::pair<decltype(_destroy_operators)::const_iterator,
                  decltype(_repair_operators)::const_iterator>
            _last_pick;

      public:
        using destroy_operator_type = destroy_operator_list::value_type;
        using repair_operator_type = repair_operator_list::value_type;
        using repair_operator_iterator = decltype(_repair_operators)::iterator;
        using destroy_operator_iterator = decltype(_destroy_operators)::iterator;
        using repair_operator_const_iterator = decltype(_repair_operators)::const_iterator;
        using destroy_operator_const_iterator = decltype(_destroy_operators)::const_iterator;

        adaptive_large_neighborhood(routingblocks::utility::random random,
                                    destroy_operator_list destroy_operator_list,
                                    repair_operator_list repair_operator_list)
            : _random(random),
              _destroy_operators(std::move(destroy_operator_list)),
              _repair_operators(std::move(repair_operator_list)) {}

        adaptive_large_neighborhood(routingblocks::utility::random random, double smoothingFactor)
            : _random(random),
              _destroy_operators(_random, smoothingFactor),
              _repair_operators(_random, smoothingFactor) {}

        void collect_score(
            std::pair<destroy_operator_list::iterator, repair_operator_list::iterator> pick,
            double score) {
            _destroy_operators.update(pick.first, score);
            _repair_operators.update(pick.second, score);
        };

        void adapt_operator_weights() {
            _destroy_operators.adapt();
            _repair_operators.adapt();
        }

        void reset_operator_weights() {
            _destroy_operators.reset_weights();
            _repair_operators.reset_weights();
        }

        decltype(_destroy_operators)::iterator add_operator(std::shared_ptr<destroy_operator> op) {
            return _destroy_operators.add(std::move(op));
        }

        decltype(_repair_operators)::iterator add_operator(std::shared_ptr<repair_operator> op) {
            return _repair_operators.add(std::move(op));
        }

        void remove_operator(repair_operator_list::const_iterator elem) {
            _repair_operators.erase(elem);
        }

        void remove_operator(destroy_operator_list::const_iterator elem) {
            _destroy_operators.erase(elem);
        }

        std::pair<destroy_operator_list::iterator, repair_operator_list::iterator> generate(
            routingblocks::Evaluation& evaluation, routingblocks::Solution& sol,
            size_t num_removed_customers) {
            if (_destroy_operators.empty() || _repair_operators.empty()) {
                throw std::runtime_error(
                    "Tried to generate a neighbourhood without any operators registered");
            }

            // pick operators
            auto destroy_op = _destroy_operators.end();
            do {
                destroy_op = _destroy_operators.pick();
            } while (!(*destroy_op)->can_apply_to(sol));

            // pick a operator to apply
            assert(routingblocks::number_of_nodes(sol) > 0);

            auto removed_vertices = (*destroy_op)->apply(evaluation, sol, num_removed_customers);

            auto repair_op = _repair_operators.end();
            do {
                repair_op = _repair_operators.pick();
            } while (!(*repair_op)->can_apply_to(sol));

            (*repair_op)->apply(evaluation, sol, removed_vertices);

            return {destroy_op, repair_op};
        };

        [[nodiscard]] auto destroy_operators_begin() const { return _destroy_operators.begin(); }
        [[nodiscard]] auto destroy_operators_end() const { return _destroy_operators.end(); }
        [[nodiscard]] auto destroy_operators_begin() { return _destroy_operators.begin(); }
        [[nodiscard]] auto destroy_operators_end() { return _destroy_operators.end(); }

        [[nodiscard]] auto repair_operators_begin() const { return _repair_operators.begin(); }
        [[nodiscard]] auto repair_operators_end() const { return _repair_operators.end(); }
        [[nodiscard]] auto repair_operators_begin() { return _repair_operators.begin(); }
        [[nodiscard]] auto repair_operators_end() { return _repair_operators.end(); }
    };
}  // namespace routingblocks
