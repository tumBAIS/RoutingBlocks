
#ifndef routingblocks_INSERTSTATIONOPERATOR_H
#define routingblocks_INSERTSTATIONOPERATOR_H

#include <routingblocks/Instance.h>
#include <routingblocks/LocalSearch.h>
#include <routingblocks/Solution.h>
#include <routingblocks/evaluation.h>

namespace routingblocks {

    struct SolutionArc {
        Solution::const_iterator route;
        Solution::route_t::const_iterator origin_node;
        Solution::route_t::const_iterator target_node;

        SolutionArc(Solution::const_iterator route, Solution::route_t::const_iterator origin_node,
                    Solution::route_t::const_iterator target_node)
            : route(route), origin_node(origin_node), target_node(target_node) {}
        SolutionArc(Solution::const_iterator route, Solution::route_t::const_iterator origin_node)
            : route(route), origin_node(origin_node), target_node(std::next(origin_node)) {}
        SolutionArc() = default;

        bool operator==(const SolutionArc& other) const {
            return route == other.route && origin_node == other.origin_node
                   && target_node == other.target_node;
        }

        bool operator!=(const SolutionArc& other) const { return !(*this == other); }
    };

    class SolutionArcIterator {
        const Solution* _solution;
        SolutionArc _arc;

        void _fix() {
            if (_arc.target_node == _arc.route->end()) {
                ++_arc.route;
                if (_arc.route != _solution->end()) {
                    _arc.target_node = _arc.route->begin();
                    _arc.origin_node = _arc.target_node++;
                } else {
                    _solution = nullptr;
                }
            }
        }

        void _inc(int by) {
            for (; by != 0; --by) {
                _arc.origin_node = _arc.target_node++;
                _fix();
            }
        }

      public:
        SolutionArcIterator(const Solution& solution, const SolutionArc& arc)
            : _solution(&solution), _arc(arc) {}
        SolutionArcIterator() : _solution(nullptr), _arc{} {}

        SolutionArcIterator& operator++() {
            _inc(1);
            return *this;
        }

        void move_to_end_of_route() { _arc.target_node = std::prev(_arc.route->end()); }

        SolutionArcIterator operator++(int) {
            SolutionArcIterator tmp(*this);
            ++(*this);
            return tmp;
        }

        bool operator==(const SolutionArcIterator& rhs) const {
            if (_solution == nullptr && rhs._solution == nullptr) {
                return true;
            }
            return _arc == rhs._arc;
        }

        bool operator!=(const SolutionArcIterator& rhs) const { return !(*this == rhs); }

        const SolutionArc& operator*() const { return _arc; }
        const SolutionArc* operator->() const { return &_arc; }
    };

    class InsertStationOperator;

    class InsertStationMove : public Move {
        friend InsertStationOperator;
        NodeLocation _after_node;
        VertexID _station_id;

      public:
        InsertStationMove(const NodeLocation& afterNode, VertexID stationId);

        cost_t get_cost_delta(Evaluation& evaluation, const Instance& instance,
                              const Solution& solution) const override;
        void apply(const Instance& instance, Solution& solution) const override;
    };

    class InsertStationOperator : public Operator {
        const Instance& _instance;

        [[nodiscard]] std::pair<SolutionArcIterator, VertexID> _recover_move(
            const Solution& solution, const InsertStationMove* move) const;

      public:
        explicit InsertStationOperator(const Instance& instance);

        void prepare_search(const Solution& solution) override;
        std::shared_ptr<Move> find_next_improving_move(eval_t& evaluation, const Solution& solution,
                                                       const Move* previous_move) override;
        void finalize_search() override;
    };

}  // namespace routingblocks
#endif  // routingblocks_INSERTSTATIONOPERATOR_H
