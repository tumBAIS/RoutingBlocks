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

#include <routingblocks/operators/InterRouteTwoOptOperator.h>

namespace routingblocks {

    cost_t InterRouteTwoOptMove::evaluate(Evaluation& evaluation, const Instance& instance,
                                          const Solution& solution) const {
        const auto& origin_location = origin();
        const auto& target_location = target();
        auto [origin_route, origin_node] = to_iter(origin_location, solution);
        auto [target_route, target_node] = to_iter(target_location, solution);

        if (origin_route >= target_route) return {};
        if (origin_node >= std::prev(origin_route->end_depot())) return {};
        if (target_node >= std::prev(target_route->end_depot())) return {};
        if (target_node == std::next(target_route->begin())) return {};

        auto move_cost = concatenate(evaluation, instance,
                                     route_segment{origin_route->begin(), std::next(origin_node)},
                                     route_segment{std::next(target_node), target_route->end()});
        move_cost += concatenate(evaluation, instance,
                                 route_segment{target_route->begin(), std::next(target_node)},
                                 route_segment{std::next(origin_node), origin_route->end()});
        move_cost -= origin_route->cost();
        move_cost -= target_route->cost();
        return move_cost;
    }

    void InterRouteTwoOptMove::apply_to([[maybe_unused]] const Instance& instance,
                                        Solution& solution) const {
        const auto& origin_location = origin();
        const auto& target_location = target();
        auto [origin_route, origin_node] = to_iter(origin_location, solution);
        auto [target_route, target_node] = to_iter(target_location, solution);

        solution.exchange_segment(origin_route, std::next(origin_node), origin_route->end_depot(),
                                  target_route, std::next(target_node), target_route->end_depot());
    }

}  // namespace routingblocks