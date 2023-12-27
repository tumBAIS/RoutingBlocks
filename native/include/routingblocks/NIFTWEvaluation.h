/*
 * Copyright (c) 2023 Patrick S. Klein (@libklein)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef routingblocks_NIFTWEVALUATION_H
#define routingblocks_NIFTWEVALUATION_H
#include <routingblocks/FRVCP.h>
#include <routingblocks/evaluation.h>
#include <routingblocks/types.h>

#include <optional>

#include "dynamic_bitset/dynamic_bitset.hpp"

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

      private:
        const resource_t _battery_capacity;
        const resource_t _storage_capacity;
        const resource_t _replenishment_time;

      public:
        NIFTWEvaluation(resource_t battery_capacity, resource_t storage_capacity,
                        resource_t replenishment_time);

      private:
        cost_t _compute_cost(resource_t distance, resource_t overload, resource_t overcharge,
                             resource_t time_shift) const;

      public:
        double overload_penalty_factor = 1.;
        double time_shift_penalty_factor = 1.;
        double overcharge_penalty_factor = 1.;

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

    struct NIFTWDPLabel {
      public:
        sul::dynamic_bitset<> visited_vertices;
        const NIFTWDPLabel* predecessor = nullptr;
        VertexID vertex_id = 0;
        resource_t cost = 0;
        resource_t t_min = 0;
        resource_t t_rt = 0;

        void clear_visits() { visited_vertices.reset(); }

        [[nodiscard]] bool visited(VertexID id) const { return visited_vertices.test(id); }

        void visit_vertex(VertexID v, bool is_station) { visited_vertices.set(v, true); }

        NIFTWDPLabel() = default;
        explicit NIFTWDPLabel(size_t number_of_vertices) : visited_vertices(number_of_vertices) {}
        NIFTWDPLabel(const NIFTWDPLabel& predecessor, VertexID vertex_id)
            : visited_vertices(predecessor.visited_vertices),
              predecessor(&predecessor),
              vertex_id(vertex_id),
              cost(predecessor.cost),
              t_min(predecessor.t_min),
              t_rt(predecessor.t_rt) {}

        [[nodiscard]] bool root_label() const { return predecessor == nullptr; }
        [[nodiscard]] resource_t earliest_arrival_time() const { return t_min; }

        friend std::ostream& operator<<(std::ostream& out, const NIFTWDPLabel& l) {
            out << "[c: " << l.cost << ", t_min: " << l.t_min << ", t_rt: " << l.t_rt << "]";
            return out;
        }
    };

    template <> class Propagator<NIFTWDPLabel> {
        const Instance* _instance;
        resource_t _battery_capacity;
        resource_t _replenishment_time;

      public:
        explicit Propagator(const Instance& instance, resource_t battery_capacity,
                            resource_t replenishment_time)
            : _instance(&instance),
              _battery_capacity(battery_capacity),
              _replenishment_time(replenishment_time){};

        std::optional<NIFTWDPLabel> propagate(const NIFTWDPLabel& predecessor, const Vertex& origin,
                                              const Vertex& target,
                                              [[maybe_unused]] const Arc& arc) {
            using std::max;
            using std::min;
            VertexID target_id = target.id;
            const auto& origin_vertex_data = origin.get_data<NIFTWVertexData>();
            const auto& target_vertex_data = origin.get_data<NIFTWVertexData>();
            const auto& arc_data = origin.get_data<NIFTWArcData>();

            const auto Q = _battery_capacity;
            const auto g = _replenishment_time;
            const auto e_j = target_vertex_data.earliest_arrival_time;
            const auto l_j = target_vertex_data.latest_arrival_time;
            const auto t_ij = arc_data.duration + origin_vertex_data.service_time;
            const auto q_ij = arc_data.consumption;

            // Avoid cycling
            if (predecessor.visited(target_id)) {
                return {};
            }

            NIFTWDPLabel label(predecessor, target_id);

            // Reset visited stations when reaching a customer.
            if (target.customer()) {
                label.clear_visits();
            }

            label.visit_vertex(target_id, target.station());

            label.cost += arc_data.cost;

            if (origin.is_station) {
                label.t_rt = q_ij;
                label.t_min = max(e_j, label.t_min + t_ij) + g;
            } else {
                label.t_rt += q_ij;
                label.t_min = max(e_j, label.t_min + t_ij);
            }

            if (label.t_min > l_j || label.t_rt > Q) {
                return {};
            }

            return label;
        }

        bool dominates(const NIFTWDPLabel& label, const NIFTWDPLabel& other) {
            return label.cost <= other.cost && label.t_min <= other.t_min
                   && label.t_rt <= other.t_rt;
        }

        bool cheaper_than(const NIFTWDPLabel& label, const NIFTWDPLabel& other) {
            return label.cost < other.cost;
        }

        bool should_order_before(const NIFTWDPLabel& label, const NIFTWDPLabel& other) {
            return label.earliest_arrival_time() < other.earliest_arrival_time();
        }

        std::vector<VertexID> extract_path(const NIFTWDPLabel& sink_label) {
            std::vector<VertexID> route;
            const auto* label = &reinterpret_cast<const NIFTWDPLabel&>(sink_label);
            for (;; label = label->predecessor) {
                route.push_back(label->vertex_id);
                if (label->root_label()) break;
            }
            std::reverse(route.begin(), route.end());
            return route;
        }

        bool is_final_label(const NIFTWDPLabel& _label) {
            const auto& label = reinterpret_cast<const NIFTWDPLabel&>(_label);
            return label.vertex_id == _instance->Depot().id && !label.root_label();
        }

        void prepare(const std::vector<VertexID>&) {}

        NIFTWDPLabel create_root_label() { return NIFTWDPLabel{_instance->NumberOfVertices()}; }
    };
}  // namespace routingblocks

#endif  // routingblocks_NIFTWEVALUATION_H
