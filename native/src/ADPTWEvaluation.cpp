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
                                   + static_cast<double>(overload) * _overload_penalty_factor
                                   + static_cast<double>(time_shift) * _time_shift_penalty_factor
                                   + static_cast<double>(overcharge) * _overcharge_penalty_factor);
    }
}  // namespace routingblocks
