#include <routingblocks/NIFTWEvaluation.h>

#include <iostream>

namespace routingblocks {

    auto NIFTWEvaluation::propagate_backward(const bwd_label_t &succ_label,
                                             const routingblocks::Vertex &succ_vertex,
                                             [[maybe_unused]] const vertex_data_t &succ_vertex_data,
                                             [[maybe_unused]] const routingblocks::Vertex &vertex,
                                             const vertex_data_t &vertex_data,
                                             [[maybe_unused]] const routingblocks::Arc &arc,
                                             const arc_data_t &arc_data) const -> bwd_label_t {
        using std::max;
        using std::min;

        routingblocks::NIFTWBackwardLabel propagated_label;

        const auto t_ij = arc_data.duration + vertex_data.service_time;
        const auto q_ij = arc_data.consumption;
        const auto c_ij = arc_data.cost;

        propagated_label.cum_distance = succ_label.cum_distance + c_ij;
        propagated_label.cum_load = succ_label.cum_load + vertex_data.demand;

        if (succ_vertex.is_station) {
            propagated_label.earliest_arrival
                = min(vertex_data.latest_arrival_time,
                      succ_label.shifted_earliest_arrival - t_ij - _replenishment_time);
            propagated_label.residual_charge_in_time = q_ij;
        } else {
            propagated_label.earliest_arrival
                = min(vertex_data.latest_arrival_time, succ_label.shifted_earliest_arrival - t_ij);
            propagated_label.residual_charge_in_time
                = min(_battery_capacity, succ_label.residual_charge_in_time) + q_ij;
        }
        propagated_label.shifted_earliest_arrival
            = max(propagated_label.earliest_arrival, vertex_data.earliest_arrival_time);

        propagated_label.cum_time_shift
            = succ_label.cum_time_shift
              + max(resource_t(0),
                    vertex_data.earliest_arrival_time - propagated_label.earliest_arrival);
        propagated_label.cum_overcharge
            = succ_label.cum_overcharge
              + max(resource_t(0), propagated_label.residual_charge_in_time - _battery_capacity);

        return propagated_label;
    }

    auto NIFTWEvaluation::propagate_forward(const fwd_label_t &pred_label,
                                            const routingblocks::Vertex &pred_vertex,
                                            const vertex_data_t &pred_vertex_data,
                                            [[maybe_unused]] const routingblocks::Vertex &vertex,
                                            const vertex_data_t &vertex_data,
                                            [[maybe_unused]] const routingblocks::Arc &arc,
                                            const arc_data_t &arc_data) const -> fwd_label_t {
        using std::max;
        using std::min;

        routingblocks::NIFTWForwardLabel propagated_label;
        auto t_ij = arc_data.duration + pred_vertex_data.service_time;
        auto q_ij = arc_data.consumption;
        auto c_ij = arc_data.cost;

        propagated_label.cum_distance = pred_label.cum_distance + c_ij;
        propagated_label.cum_load = pred_label.cum_load + vertex_data.demand;

        if (pred_vertex.is_station) {
            propagated_label.earliest_arrival
                = max(vertex_data.earliest_arrival_time, pred_label.shifted_earliest_arrival + t_ij)
                  + _replenishment_time;
            propagated_label.residual_charge_in_time = q_ij;
        } else {
            propagated_label.earliest_arrival = max(vertex_data.earliest_arrival_time,
                                                    pred_label.shifted_earliest_arrival + t_ij);
            propagated_label.residual_charge_in_time
                = min(pred_label.residual_charge_in_time, _battery_capacity) + q_ij;
        }

        propagated_label.shifted_earliest_arrival
            = min(propagated_label.earliest_arrival, vertex_data.latest_arrival_time);

        propagated_label.cum_time_shift
            = pred_label.cum_time_shift
              + max(propagated_label.earliest_arrival - vertex_data.latest_arrival_time,
                    resource_t(0));
        propagated_label.cum_overcharge
            = pred_label.cum_overcharge
              + max(propagated_label.residual_charge_in_time - _battery_capacity, resource_t(0));

        propagated_label.prev_time_shift = pred_label.cum_time_shift;
        propagated_label.prev_overcharge = pred_label.cum_overcharge;

        return propagated_label;
    }

    cost_t NIFTWEvaluation::concatenate(const fwd_label_t &fwd, const bwd_label_t &bwd,
                                        const routingblocks::Vertex &vertex,
                                        const vertex_data_t &vertex_data) {
        using std::max;
        using std::min;

        resource_t distance = fwd.cum_distance + bwd.cum_distance;
        resource_t overload = max(
            fwd.cum_load + bwd.cum_load - vertex_data.demand - _storage_capacity, resource_t(0));

        resource_t additional_overcharge, additional_time_shift;

        additional_time_shift
            = max(resource_t(0), fwd.shifted_earliest_arrival - bwd.shifted_earliest_arrival);

        if (vertex.is_station) {
            additional_overcharge
                = max(fwd.residual_charge_in_time - _battery_capacity, resource_t(0));
        } else {
            additional_overcharge
                = max(resource_t(0), fwd.residual_charge_in_time
                                         + min(_battery_capacity, bwd.residual_charge_in_time)
                                         - _battery_capacity);
        }

        resource_t time_shift = fwd.cum_time_shift + bwd.cum_time_shift + additional_time_shift;
        resource_t overcharge = fwd.prev_overcharge + bwd.cum_overcharge + additional_overcharge;

        return _compute_cost(distance, overload, overcharge, time_shift);
    }

    NIFTWEvaluation::NIFTWEvaluation(resource_t battery_capacity, resource_t storage_capacity,
                                     resource_t replenishment_time)
        : _battery_capacity(battery_capacity),
          _storage_capacity(storage_capacity),
          _replenishment_time(replenishment_time) {}

    std::ostream &operator<<(std::ostream &os, const NIFTWLabel &label) {
        os << "{earliest_arrival: " << label.earliest_arrival
           << ", latest_arrival: " << label.latest_arrival
           << ", shifted_earliest_arrival: " << label.shifted_earliest_arrival
           << ", residual_charge_in_time: " << label.residual_charge_in_time
           << ", cum_distance: " << label.cum_distance << ", cum_load: " << label.cum_load
           << ", cum_time_shift: " << label.cum_time_shift
           << ", cum_overcharge: " << label.cum_overcharge << "}";
        return os;
    }

    cost_t NIFTWEvaluation::compute_cost(const fwd_label_t &label) const {
        return _compute_cost(label.cum_distance,
                             std::max(resource_t(0), label.cum_load - _storage_capacity),
                             label.cum_overcharge, label.cum_time_shift);
    }

    auto NIFTWEvaluation::create_forward_label(const Vertex &vertex,
                                               [[maybe_unused]] const vertex_data_t &vertex_data)
        -> fwd_label_t {
        return NIFTWForwardLabel(vertex);
    }

    auto NIFTWEvaluation::create_backward_label(const Vertex &vertex,
                                                [[maybe_unused]] const vertex_data_t &vertex_data)
        -> bwd_label_t {
        return NIFTWBackwardLabel(vertex);
    }

    cost_t NIFTWEvaluation::_compute_cost(resource_t distance, resource_t overload,
                                          resource_t overcharge, resource_t time_shift) const {
        return static_cast<cost_t>(static_cast<double>(distance)
                                   + static_cast<double>(overload) * _overload_penalty_factor
                                   + static_cast<double>(time_shift) * _time_shift_penalty_factor
                                   + static_cast<double>(overcharge) * _overcharge_penalty_factor);
    }
}  // namespace routingblocks
