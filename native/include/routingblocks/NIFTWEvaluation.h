
#ifndef routingblocks_NIFTWEVALUATION_H
#define routingblocks_NIFTWEVALUATION_H
#include <routingblocks/evaluation.h>
#include <routingblocks/types.h>

#include <array>

namespace routingblocks {

    struct NIFTWVertexData {
        float x_coord;
        float y_coord;
        resource_t demand;
        resource_t earliest_arrival_time;
        resource_t latest_arrival_time;
        resource_t service_time;
        bool is_station;
        bool is_depot;
    };

    struct NIFTWArcData {
        resource_t cost;
        resource_t consumption;
        resource_t duration;
    };

    struct NIFTWLabel {
        // Earliest and latest arrival times
        resource_t earliest_arrival = 0;
        resource_t latest_arrival = 0;
        // Shifted arrivals - retain feasibility
        resource_t shifted_earliest_arrival = 0;
        // Residual charge expressed in time
        resource_t residual_charge_in_time = 0;
        resource_t cum_distance = 0;

        resource_t cum_load = 0;
        resource_t cum_time_shift = 0;
        resource_t cum_overcharge = 0;
        friend std::ostream& operator<<(std::ostream& os, const NIFTWLabel& label);
    };

    struct NIFTWForwardLabel : public NIFTWLabel {
        resource_t prev_time_shift = 0;
        resource_t prev_overcharge = 0;

        NIFTWForwardLabel() = default;
        explicit NIFTWForwardLabel(const Vertex& v) {
            const auto& data = v.get_data<NIFTWVertexData>();
            earliest_arrival = data.earliest_arrival_time;
            shifted_earliest_arrival = data.earliest_arrival_time;
            latest_arrival = data.earliest_arrival_time;
        }
    };
    struct NIFTWBackwardLabel : public NIFTWLabel {
        NIFTWBackwardLabel() = default;
        explicit NIFTWBackwardLabel(const Vertex& v) {
            const auto& data = v.get_data<NIFTWVertexData>();
            earliest_arrival = data.latest_arrival_time;
            shifted_earliest_arrival = data.latest_arrival_time;
            latest_arrival = data.latest_arrival_time;
        }
    };

    class NIFTWEvaluation
        : public ConcatenationBasedEvaluationImpl<NIFTWEvaluation, NIFTWForwardLabel,
                                                  NIFTWBackwardLabel, NIFTWVertexData,
                                                  NIFTWArcData> {
      public:
        using fwd_label_t = NIFTWForwardLabel;
        using bwd_label_t = NIFTWBackwardLabel;
        using vertex_data_t = NIFTWVertexData;
        using arc_data_t = NIFTWArcData;

        enum CostComponent {
            DIST_INDEX = 0,
            OVERLOAD_INDEX = 1,
            OVERCHARGE_INDEX = 2,
            TIME_SHIFT_INDEX = 3
        };

      private:
        const resource_t _battery_capacity;
        const resource_t _storage_capacity;
        const resource_t _replenishment_time;

        double _overload_penalty_factor = 1.;
        double _time_shift_penalty_factor = 1.;
        double _overcharge_penalty_factor = 1.;

      public:
        NIFTWEvaluation(resource_t battery_capacity, resource_t storage_capacity,
                        resource_t replenishment_time);

      private:
        cost_t _compute_cost(resource_t distance, resource_t overload, resource_t overcharge,
                             resource_t time_shift) const;

      public:
        std::array<double, 4> get_penalty_factors() const {
            auto vector = std::array<double, 4>();
            vector[DIST_INDEX] = 1.;
            vector[OVERLOAD_INDEX] = _overload_penalty_factor;
            vector[OVERCHARGE_INDEX] = _overcharge_penalty_factor;
            vector[TIME_SHIFT_INDEX] = _time_shift_penalty_factor;
            return vector;
        };

        void set_penalty_factors(const std::array<double, 4>& factors) {
            _overload_penalty_factor = factors[OVERLOAD_INDEX];
            _overcharge_penalty_factor = factors[OVERCHARGE_INDEX];
            _time_shift_penalty_factor = factors[TIME_SHIFT_INDEX];
        };

        cost_t concatenate(const fwd_label_t& fwd, const bwd_label_t& bwd,
                           const routingblocks::Vertex& vertex, const vertex_data_t& vertex_data);

        [[nodiscard]] std::vector<resource_t> get_cost_components(const fwd_label_t& fwd) const {
            return {fwd.cum_distance, std::max(resource_t(0), fwd.cum_load - _storage_capacity),
                    fwd.cum_overcharge, fwd.cum_time_shift};
        };

        [[nodiscard]] cost_t compute_cost(const fwd_label_t& label) const;

        [[nodiscard]] bool is_feasible(const fwd_label_t& fwd) const {
            return fwd.cum_overcharge == 0 && fwd.cum_time_shift == 0
                   && fwd.cum_load <= _storage_capacity;
        };

        [[nodiscard]] fwd_label_t propagate_forward(const fwd_label_t& pred_label,
                                                    const routingblocks::Vertex& pred_vertex,
                                                    const vertex_data_t& pred_vertex_data,
                                                    const routingblocks::Vertex& vertex,
                                                    const vertex_data_t& vertex_data,
                                                    const routingblocks::Arc& arc,
                                                    const arc_data_t& arc_data) const;

        [[nodiscard]] bwd_label_t propagate_backward(const bwd_label_t& succ_label,
                                                     const routingblocks::Vertex& succ_vertex,
                                                     const vertex_data_t& succ_vertex_data,
                                                     const routingblocks::Vertex& vertex,
                                                     const vertex_data_t& vertex_data,
                                                     const routingblocks::Arc& arc,
                                                     const arc_data_t& arc_data) const;

        fwd_label_t create_forward_label(const Vertex& vertex, const vertex_data_t& vertex_data);
        bwd_label_t create_backward_label(const Vertex& vertex, const vertex_data_t& vertex_data);
    };
}  // namespace routingblocks

#endif  // routingblocks_NIFTWEVALUATION_H
