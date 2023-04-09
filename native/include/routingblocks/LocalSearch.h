/*MIT License

Original HGS-CVRP code: Copyright(c) 2020 Thibaut Vidal
Additional contributions: Copyright(c) 2022 ORTEC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include <routingblocks/Instance.h>
#include <routingblocks/Solution.h>
#include <routingblocks/evaluation.h>
#include <routingblocks/utility/arc_set.h>
#include <routingblocks/utility/random.h>

#include <memory>
#include <set>
#include <vector>

namespace routingblocks {
    struct GeneratorArc {
        Solution::const_iterator origin_route;
        Solution::route_t::const_iterator origin_node;

        Solution::const_iterator target_route;
        Solution::route_t::const_iterator target_node;
    };

    class Move {
      public:
        [[nodiscard]] virtual cost_t get_cost_delta(Evaluation& evaluation,
                                                    const Instance& instance,
                                                    const Solution& solution) const
            = 0;
        virtual void apply(const Instance& instance, Solution& solution) const = 0;
        virtual ~Move() = default;
    };

    class Operator {
      public:
        using eval_t = routingblocks::Evaluation;

        virtual void prepare_search(const Solution& solution) = 0;

        [[nodiscard]] virtual std::shared_ptr<Move> find_next_improving_move(
            eval_t& evaluation, const Solution& solution, const Move* previous_move)
            = 0;

        virtual void finalize_search() = 0;

        virtual ~Operator() = default;
    };

    template <class move_t>
    concept specializes_exact_cost_computation
        = requires(move_t move) {
              move.evaluate_exact(std::declval<routingblocks::Evaluation&>(),
                                  std::declval<const Instance&>(), std::declval<const Solution&>());
          };

    template <class operator_t>
    concept specializes_move_construction
        = requires(operator_t op) {
              op.create_move(std::declval<NodeLocation>(), std::declval<NodeLocation>());
          };

    template <class Impl> class GeneratorArcMove : public Move {
        NodeLocation _origin, _target;

      public:
        GeneratorArcMove(NodeLocation origin, NodeLocation target)
            : _origin(origin), _target(target) {}

        [[nodiscard]] cost_t get_cost_delta(Evaluation& evaluation, const Instance& instance,
                                            const Solution& sol) const final {
            auto& impl = static_cast<const Impl&>(*this);
            return impl.evaluate(evaluation, instance, sol);
        }

        [[nodiscard]] NodeLocation origin() const { return _origin; }
        [[nodiscard]] NodeLocation target() const { return _target; }

        void apply(const Instance& instance, Solution& solution) const final {
            auto& impl = static_cast<const Impl&>(*this);
            impl.apply_to(instance, solution);
        }
    };

    class QuadraticNeighborhoodIterator {
        GeneratorArc _current_arc;
        const Solution* _solution;

        void _fix() {
            auto& origin_route = _current_arc.origin_route;
            auto& origin_node = _current_arc.origin_node;
            auto& target_route = _current_arc.target_route;
            auto& target_node = _current_arc.target_node;

            if (target_node == target_route->end()) {
                ++target_route;

                if (target_route == _solution->end()) {
                    ++origin_node;
                    target_route = _solution->begin();
                    target_node = target_route->begin();

                    if (origin_node == origin_route->end()) {
                        ++origin_route;
                        if (origin_route != _solution->end()) {
                            origin_node = origin_route->begin();
                        } else {
                            // End reached
                            _solution = nullptr;
                        }
                    }
                } else {
                    target_node = target_route->begin();
                }
            }
        }

        void _inc(int by) {
            // TODO Don't iterate over all arcs, but take symmetry into account
            for (; by != 0; --by) {
                ++_current_arc.target_node;
                _fix();
            }
        }

      public:
        QuadraticNeighborhoodIterator() : _solution(nullptr) {}
        QuadraticNeighborhoodIterator(const Solution& solution, const GeneratorArc& offset)
            : _current_arc(offset), _solution(&solution) {}

        const GeneratorArc& operator*() const { return _current_arc; }
        const GeneratorArc* operator->() const { return &_current_arc; }
        QuadraticNeighborhoodIterator operator++(int) {
            QuadraticNeighborhoodIterator copy(*this);
            ++(*this);
            return copy;
        }
        QuadraticNeighborhoodIterator& operator++() {
            _inc(1);
            return *this;
        }

        bool operator==(const QuadraticNeighborhoodIterator& other) const {
            if (_solution == nullptr && other._solution == nullptr) {
                return true;
            }
            return _current_arc.origin_route == other._current_arc.origin_route
                   && _current_arc.origin_node == other._current_arc.origin_node
                   && _current_arc.target_route == other._current_arc.target_route
                   && _current_arc.target_node == other._current_arc.target_node;
        }

        bool operator!=(const QuadraticNeighborhoodIterator& other) const {
            return !(*this == other);
        }
    };

    template <class move_t>
        requires std::is_base_of_v<GeneratorArcMove<move_t>, move_t>
    class GeneratorArcOperator : public Operator {
      protected:
        const Instance& _instance;
        const utility::arc_set* _arc_set;

        QuadraticNeighborhoodIterator _get_next_arc(const Solution& solution, const Move* move) {
            if (move == nullptr) {
                return QuadraticNeighborhoodIterator(
                    solution, GeneratorArc{solution.begin(), solution.begin()->begin(),
                                           solution.begin(), solution.begin()->begin()});
            } else {
                auto* arc_move = static_cast<const move_t*>(move);
                auto [origin_route, origin_node] = to_iter(arc_move->origin(), solution);
                auto [target_route, target_node] = to_iter(arc_move->target(), solution);
                auto iter = QuadraticNeighborhoodIterator(
                    solution, GeneratorArc{origin_route, origin_node, target_route, target_node});
                return ++iter;
            }
        }

      public:
        explicit GeneratorArcOperator(const Instance& instance, const utility::arc_set* arc_set)
            : _instance(instance), _arc_set(arc_set) {}

        void prepare_search(const Solution&) override {}

        std::shared_ptr<Move> find_next_improving_move(eval_t& evaluation, const Solution& solution,
                                                       const Move* previous_move) override {
            auto neighborhood_iter = _get_next_arc(solution, previous_move);

            // Iterate over all arcs in the solution
            const auto end_iter = QuadraticNeighborhoodIterator();
            for (; neighborhood_iter != end_iter; ++neighborhood_iter) {
                if (neighborhood_iter->origin_route == neighborhood_iter->target_route
                    && neighborhood_iter->origin_node == neighborhood_iter->target_node) {
                    continue;
                }
                if (_arc_set
                    && !_arc_set->includes_arc(neighborhood_iter->origin_node->vertex_id(),
                                               neighborhood_iter->target_node->vertex_id())) {
                    continue;
                }
                // TODO Can improve - carry index
                NodeLocation origin = location_cast(solution, neighborhood_iter->origin_route,
                                                    neighborhood_iter->origin_node);
                NodeLocation target = location_cast(solution, neighborhood_iter->target_route,
                                                    neighborhood_iter->target_node);
                if (const move_t& move = create_move(origin, target);
                    move.evaluate(evaluation, _instance, solution) < 0) {
                    return std::make_shared<move_t>(move);
                }
            }
            return {};
        }

        auto create_move(NodeLocation origin, NodeLocation target) const {
            return move_t(origin, target);
        }

        void finalize_search() override {}
    };

    // Main local learch structure
    class LocalSearch {
        using eval_t = routingblocks::Evaluation;
        using solution_t = routingblocks::Solution;

      private:
        const routingblocks::Instance* _instance;  // Problem instance
        std::shared_ptr<eval_t> _evaluation;
        std::shared_ptr<eval_t> _exact_evaluation;
        std::vector<Operator*> _operators;

        int loopID = 0;  // Current loop index
        bool use_best_improvement = false;

        solution_t _current_solution;

        void _apply_move(const Move& move);
        cost_t _test_move(const Move& move);
        [[nodiscard]] std::shared_ptr<Move> _explore_neighborhood();

      public:
        void set_use_best_improvement(bool best_improvement) {
            this->use_best_improvement = best_improvement;
        }

        // Run the local search with the specified penalty values
        template <class ForwardIterator>
            requires std::same_as<typename ForwardIterator::value_type, Operator*>
        void run(solution_t& sol, ForwardIterator operators_begin, ForwardIterator operators_end) {
            _current_solution = std::move(sol);

            // TODO Figure out a better way to do this.
            _operators.clear();
            std::copy(operators_begin, operators_end, std::back_inserter(_operators));

            for (loopID = 0; true; loopID++) {
                std::shared_ptr<Move> first_improving_move = _explore_neighborhood();
                // Stop search, no improvement found.
                if (!first_improving_move) break;

                _apply_move(*first_improving_move);
            }

            sol = std::move(_current_solution);
        }

        // Constructor
        LocalSearch(const routingblocks::Instance& instance, std::shared_ptr<eval_t> evaluation,
                    std::shared_ptr<eval_t> exact_evaluation);
    };

}  // namespace routingblocks

std::ostream& operator<<(std::ostream& out, const routingblocks::Node& node);

#endif
