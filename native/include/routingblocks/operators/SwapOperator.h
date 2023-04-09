
#ifndef routingblocks_SWAPOPERATOR_H
#define routingblocks_SWAPOPERATOR_H

#include <routingblocks/Instance.h>
#include <routingblocks/LocalSearch.h>
#include <routingblocks/Solution.h>
#include <routingblocks/evaluation.h>

namespace routingblocks {

    // "Proper" swaps
    template <size_t origin_segment_length, size_t target_segment_length> class SwapMove
        : public GeneratorArcMove<SwapMove<origin_segment_length, target_segment_length>> {
        /**
         * Generator arc is (origin, target). Our goal is to include this arc into the solution.
         * This operator swaps two sequences, hence the most straightforward way to do this is to
         * swap as follows:
         * [..., origin] [origin + 1, ..., origin + origin_segment_length + 1] [origin +
         * origin_segment_length + 2, ...]
         * [..., target- 1] [target, ..., target + target_segment_length] [target +
         * target_segment_length + 1, ...] to
         * [..., origin] [target, ..., target + target_segment_length] [origin +
         * origin_segment_length + 2, ...]
         * [..., target- 1] [origin + 1, ..., origin + origin_segment_length + 1] [target +
         * target_segment_length + 1,
         * ...]
         */
      public:
        using GeneratorArcMove<SwapMove>::GeneratorArcMove;

        void apply_to(const Instance&, Solution& solution) const {
            const auto& origin_location = this->origin();
            const auto& target_location = this->target();
            auto [swap_origin_route, origin_node] = to_iter(origin_location, solution);
            auto [swap_target_route, target_node] = to_iter(target_location, solution);
            // First swapped node in origin route
            auto swap_origin_begin = std::next(origin_node);
            auto swap_origin_end = std::next(swap_origin_begin, origin_segment_length);

            auto swap_target_begin = target_node;
            auto swap_target_end = std::next(swap_target_begin, target_segment_length);

            solution.exchange_segment(swap_origin_route, swap_origin_begin, swap_origin_end,
                                      swap_target_route, swap_target_begin, swap_target_end);
        }

        [[nodiscard]] cost_t evaluate(Evaluation& evaluation, const Instance& instance,
                                      const Solution& solution) const {
            cost_t delta_cost = 0.0;

            const auto& origin_location = this->origin();
            const auto& target_location = this->target();
            auto [origin_route, origin_node] = to_iter(origin_location, solution);
            auto [target_route, target_node] = to_iter(target_location, solution);
            // First swapped node in origin route
            auto swap_origin_begin = std::next(origin_node);
            auto swap_origin_last = std::next(swap_origin_begin, origin_segment_length - 1);
            auto swap_origin_end = std::next(swap_origin_last);

            auto before_swap_target_begin = std::prev(target_node);
            auto swap_target_begin = target_node;
            auto swap_target_last = std::next(swap_target_begin, target_segment_length - 1);
            auto swap_target_end = std::next(swap_target_last);

            // Any move that would swap the start or end depot is invalid.
            if (swap_origin_begin == origin_route->begin()
                || swap_target_begin == target_route->begin()
                || origin_node == origin_route->end_depot()) {
                return delta_cost;
            } else if (std::distance(swap_target_begin, target_route->end_depot())
                       < target_segment_length) {
                return delta_cost;
            } else if (std::distance(swap_origin_begin, origin_route->end_depot())
                       < origin_segment_length) {
                return delta_cost;
            }

            // Skip moves for symmetric operators
            if constexpr (origin_segment_length == target_segment_length) {
                // In general, comparison by pointer leads to non-deterministic behavior across
                // different machines for iterators from different containers. Thats why we compare
                // by index here.
                if (std::distance(solution.begin(), origin_route)
                    > std::distance(solution.begin(), target_route)) {
                    return delta_cost;
                }
                if (origin_route == target_route) {
                    if (std::distance(origin_route->begin(), origin_node)
                        > std::distance(origin_route->begin(), target_node)) {
                        return delta_cost;
                    }
                }
            }

            if (origin_route != target_route) {
                // Straightforward case. Individual exchanges can be evaluated independently.
                /*delta_cost += concatenate(evaluation, instance, origin_node, swap_origin_end,
                                          route_segment{swap_target_begin, swap_target_end});*/
                delta_cost += concatenate(
                    evaluation, instance, route_segment{origin_route->begin(), swap_origin_begin},
                    route_segment{swap_target_begin, swap_target_end},
                    route_segment{swap_origin_end, origin_route->end_depot()});

                delta_cost += concatenate(
                    evaluation, instance, route_segment{target_route->begin(), swap_target_begin},
                    route_segment{swap_origin_begin, swap_origin_end},
                    route_segment{swap_target_end, target_route->end_depot()});

                delta_cost -= origin_route->cost();
                delta_cost -= target_route->cost();
            } else {
                // Skip overlapping segments
                // Skip moves that would include moving the insertion position itself
                if (!(swap_target_last < swap_origin_begin
                      || swap_origin_last < swap_target_begin)) {
                    return delta_cost;
                }

                if (swap_target_last < swap_origin_begin) {
                    // T - O swap:
                    // bob: before_swap_origin_begin
                    // tb: target_begin | tl: target_last | te: target_end
                    // ob: origin_begin | ol: origin_last | oe: origin_end
                    // Before swap
                    // [...x...] [tb, ..., tl] [te, ..., bob] [ob, ..., ol] [oe, ...]
                    // After swap
                    // [...x...] [ob, ..., ol] [te, ..., bob] [tb, ..., tl] [oe, ...]

                    /*delta_cost += concatenate(
                        evaluation, instance, last_valid_fwd, last_valid_bwd,
                        route_segment{swap_origin_begin, swap_origin_end},  // ob ... ol
                        route_segment{swap_target_end, swap_origin_begin},  // te ... bob
                        route_segment{swap_target_begin, swap_target_end}   // tb ... tl
                        */

                    delta_cost += concatenate(
                        evaluation, instance,
                        route_segment{target_route->begin(), swap_target_begin},  // ...x...
                        route_segment{swap_origin_begin, swap_origin_end},        // ob ... ol
                        route_segment{swap_target_end, swap_origin_begin},        // te ... bob
                        route_segment{swap_target_begin, swap_target_end},        // tb ... tl
                        route_segment{swap_origin_end, origin_route->end()});     // oe ...
                } else {
                    // O - T swap:
                    // btb: before_swap_target_begin
                    // tb: target_begin | tl: target_last | te: target_end
                    // ob: origin_begin | ol: origin_last | oe: origin_end
                    // Before swap
                    // [...x...] [ob, ..., ol] [oe, ..., btb] [tb, ..., tl] [te, ...]
                    // After swap
                    // [...x...] [tb, ..., tl] [oe, ..., btb] [ob, ..., ol] [te, ...]

                    /*delta_cost += concatenate(
                        evaluation, instance, last_valid_fwd, last_valid_bwd,
                        route_segment{swap_target_begin, swap_target_end},  // tb, ..., tl
                        route_segment{swap_origin_end, swap_target_begin},  // oe, ..., btb
                        route_segment{swap_origin_begin, swap_origin_end}   // ob, ..., ol
                    );*/

                    delta_cost += concatenate(
                        evaluation, instance,
                        route_segment{origin_route->begin(), swap_origin_begin},  // ...x...
                        route_segment{swap_target_begin, swap_target_end},        // tb, ..., tl
                        route_segment{swap_origin_end, swap_target_begin},        // oe, ..., btb
                        route_segment{swap_origin_begin, swap_origin_end},        // ob, ..., ol
                        route_segment{swap_target_end, target_route->end()});     // te, ...
                }

                delta_cost -= origin_route->cost();
            }

            return delta_cost;
        };
    };

    // Relocate moves
    template <size_t target_segment_length> class SwapMove<0, target_segment_length>
        : public GeneratorArcMove<SwapMove<0, target_segment_length>> {
      public:
        using GeneratorArcMove<SwapMove<0, target_segment_length>>::GeneratorArcMove;

        void apply_to(const Instance&, Solution& solution) const {
            const auto& origin_location = this->origin();
            const auto& target_location = this->target();
            auto [insert_route, insert_after_node] = to_iter(origin_location, solution);
            auto [removal_route, moved_segment_begin] = to_iter(target_location, solution);
            auto moved_segment_last = std::next(moved_segment_begin, target_segment_length - 1);
            auto moved_segment_end = std::next(moved_segment_last);

            // Exchange segment [moved_segment_begin, moved_segment_last] with [origin_node)
            solution.exchange_segment(insert_route, std::next(insert_after_node),
                                      std::next(insert_after_node), removal_route,
                                      moved_segment_begin, moved_segment_end);
        }

        [[nodiscard]] cost_t evaluate(Evaluation& evaluation, const Instance& instance,
                                      const Solution& solution) const {
            cost_t delta_cost{};

            const auto& origin_location = this->origin();
            const auto& target_location = this->target();
            auto [insert_route, insert_after_node] = to_iter(origin_location, solution);
            auto [removal_route, moved_segment_begin] = to_iter(target_location, solution);
            auto moved_segment_last = std::next(moved_segment_begin, target_segment_length - 1);
            auto moved_segment_end = std::next(moved_segment_last);

            size_t max_segment_length
                = std::distance(moved_segment_begin, removal_route->end_depot());
            // If the segment moves the end_depot, then the move is not applicable
            if (max_segment_length < target_segment_length) {
                return delta_cost;
            }
            // It is also not allowed to move the begin depot
            if (moved_segment_begin == removal_route->begin()) {
                return delta_cost;
            }
            // Do not allow insertion after the end depot
            if (insert_after_node == insert_route->end_depot()) {
                return delta_cost;
            }

            // Try to add arc (insert_after_node, moved_segment_begin) to the solution.
            // I.e., move [moved_segment_begin, moved_segment_last] to insert_after_node.
            // Inter-Route case: Removal/Insertion is independent
            if (insert_route != removal_route) {
                // Calculate the cost of removing moved_segment_begin from its original route
                delta_cost = concatenate(evaluation, instance,
                                         route_segment{removal_route->begin(), moved_segment_begin},
                                         route_segment{moved_segment_end, removal_route->end()});
                // Add the cost of inserting into the origin route
                delta_cost += concatenate(
                    evaluation, instance,
                    route_segment{insert_route->begin(), std::next(insert_after_node)},
                    route_segment(moved_segment_begin, moved_segment_end),
                    route_segment{std::next(insert_after_node), insert_route->end()});

                delta_cost -= insert_route->cost();
                delta_cost -= removal_route->cost();
            } else {
                // Skip moves that would include moving the insertion position itself
                if (insert_after_node >= moved_segment_begin
                    && insert_after_node <= moved_segment_last) {
                    return delta_cost;
                }
                // Avoid superfluous moves
                if (insert_after_node == std::prev(moved_segment_begin)) {
                    return delta_cost;
                }
                // Calculate move cost
                if (insert_after_node < moved_segment_begin) {
                    // Before relocate:
                    // [...x...] [...y...] [b, ..., e] [...z...]
                    // After relocate:
                    // [...x...] [b, ..., e] [...y...] [...z...]

                    auto segment_y_begin = std::next(insert_after_node);
                    auto segment_y_end = moved_segment_begin;
                    auto segment_z_begin = moved_segment_end;

                    delta_cost = concatenate(evaluation, instance,
                                             route_segment{insert_route->begin(), segment_y_begin},
                                             route_segment{moved_segment_begin, moved_segment_end},
                                             route_segment{segment_y_begin, segment_y_end},
                                             route_segment{segment_z_begin, insert_route->end()});
                } else {
                    // Before relocate:
                    // [...x...] [b, ..., e] [...y...] [...z...]
                    // After relocate:
                    // [...x...] [...y...] [b, ..., e] [...z...]

                    auto segment_x_end = moved_segment_begin;
                    auto segment_y_begin = moved_segment_end;
                    auto segment_y_end = std::next(insert_after_node);
                    auto segment_z_begin = segment_y_end;

                    delta_cost = concatenate(evaluation, instance,
                                             route_segment{insert_route->begin(), segment_x_end},
                                             route_segment{segment_y_begin, segment_y_end},
                                             route_segment{moved_segment_begin, moved_segment_end},
                                             route_segment{segment_z_begin, insert_route->end()});
                }
                delta_cost -= insert_route->cost();
            }

            return delta_cost;
        }
    };

    template <size_t origin_segment_length, size_t target_segment_length> class SwapOperator
        : public GeneratorArcOperator<SwapMove<origin_segment_length, target_segment_length>> {
      public:
        using GeneratorArcOperator<
            SwapMove<origin_segment_length, target_segment_length>>::GeneratorArcOperator;
    };

}  // namespace routingblocks
#endif  // routingblocks_SWAPOPERATOR_H
