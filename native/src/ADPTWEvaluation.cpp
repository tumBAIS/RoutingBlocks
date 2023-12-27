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

#include <routingblocks/ADPTWEvaluation.h>

#include <iostream>

namespace routingblocks {

    ADPTWForwardResourceLabel::ADPTWForwardResourceLabel(
        const routingblocks::Vertex &depot, [[maybe_unused]] resource_t battery_capacity)
        : ADPTWResourceLabel{.earliest_arrival
                             = depot.get_data<ADPTWVertexData>().earliest_arrival_time,
                             .latest_arrival
                             = depot.get_data<ADPTWVertexData>().earliest_arrival_time,
                             .shifted_earliest_arrival
                             = depot.get_data<ADPTWVertexData>().earliest_arrival_time,
                             .shifted_latest_arrival
                             = depot.get_data<ADPTWVertexData>().earliest_arrival_time,
                             .residual_charge_in_time = 0,
                             .f = 0,
                             .cum_distance = 0,
                             .cum_load = 0,
                             .cum_time_shift = 0,
                             .cum_overcharge = 0},
          prev_time_shift(0),
          prev_overcharge(0) {}

    ADPTWBackwardResourceLabel::ADPTWBackwardResourceLabel(
        const routingblocks::Vertex &depot, [[maybe_unused]] resource_t battery_capacity)
        : ADPTWResourceLabel{
            .earliest_arrival = depot.get_data<ADPTWVertexData>().latest_arrival_time,
            .latest_arrival = depot.get_data<ADPTWVertexData>().latest_arrival_time,
            .shifted_earliest_arrival = depot.get_data<ADPTWVertexData>().latest_arrival_time,
            .shifted_latest_arrival = depot.get_data<ADPTWVertexData>().latest_arrival_time,
            .residual_charge_in_time = 0,
            .f = 0,
            .cum_distance = 0,
            .cum_load = 0,
            .cum_time_shift = 0,
            .cum_overcharge = 0} {}

    ADPTWEvaluation::ADPTWEvaluation(resource_t batteryCapacity, resource_t storageCapacity)
        : _battery_capacity(batteryCapacity), _storage_capacity(storageCapacity) {}

    std::ostream &operator<<(std::ostream &os, const ADPTWResourceLabel &label) {
        os << "{earliest_arrival: " << label.earliest_arrival
           << ", latest_arrival: " << label.latest_arrival
           << ", shifted_earliest_arrival: " << label.shifted_earliest_arrival
           << ", shifted_latest_arrival: " << label.shifted_latest_arrival
           << ", residual_charge_in_time: " << label.residual_charge_in_time
           << ", cum_distance: " << label.cum_distance << ", cum_load: " << label.cum_load
           << ", cum_time_shift: " << label.cum_time_shift
           << ", cum_overcharge: " << label.cum_overcharge << "}";
        return os;
    }

    cost_t ADPTWEvaluation::_compute_cost(resource_t distance, resource_t overload,
                                          resource_t overcharge, resource_t time_shift) const {
        return static_cast<cost_t>(static_cast<double>(distance)
                                   + static_cast<double>(overload) * overload_penalty_factor
                                   + static_cast<double>(time_shift) * time_shift_penalty_factor
                                   + static_cast<double>(overcharge) * overcharge_penalty_factor);
    }
}  // namespace routingblocks
