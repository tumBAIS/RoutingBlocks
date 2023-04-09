
#ifndef routingblocks_ADPTWEVALUATION_H
#define routingblocks_ADPTWEVALUATION_H

#include <routingblocks/FRVCP.h>
#include <routingblocks/evaluation.h>
#include <routingblocks/types.h>

#include <array>
#include <dynamic_bitset/dynamic_bitset.hpp>
#include <optional>
#include <vector>

namespace routingblocks {
    struct ADPTWVertexData {
        float x_coord;
        float y_coord;
        resource_t demand;
        resource_t earliest_arrival_time;
        resource_t latest_arrival_time;
        resource_t service_time;
        ADPTWVertexData(float xCoord, float yCoord, resource_t demand,
                        resource_t earliestArrivalTime, resource_t latestArrivalTime,
                        resource_t serviceTime)
            : x_coord(xCoord),
              y_coord(yCoord),
              demand(demand),
              earliest_arrival_time(earliestArrivalTime),
              latest_arrival_time(latestArrivalTime),
              service_time(serviceTime) {}
    };

    struct ADPTWArcData {
        resource_t cost;
        resource_t consumption;
        resource_t duration;
        ADPTWArcData(resource_t cost, resource_t consumption, resource_t duration)
            : cost(cost), consumption(consumption), duration(duration) {}
    };

    struct ADPTWResourceLabel {
        // Earliest and latest arrival times
        resource_t earliest_arrival;
        resource_t latest_arrival;
        // Shifted arrivals - retain feasibility
        resource_t shifted_earliest_arrival;
        resource_t shifted_latest_arrival;
        // Residual charge expressed in time
        resource_t residual_charge_in_time;

        int f;

        resource_t cum_distance;

        resource_t cum_load;
        resource_t cum_time_shift;
        resource_t cum_overcharge;
        friend std::ostream& operator<<(std::ostream& os, const ADPTWResourceLabel& label);

        bool operator==(const ADPTWResourceLabel& other) const {
            return earliest_arrival == other.earliest_arrival
                   && latest_arrival == other.latest_arrival
                   && shifted_earliest_arrival == other.shifted_earliest_arrival
                   && shifted_latest_arrival == other.shifted_latest_arrival
                   && residual_charge_in_time == other.residual_charge_in_time
                   && cum_distance == other.cum_distance && cum_load == other.cum_load
                   && cum_time_shift == other.cum_time_shift
                   && cum_overcharge == other.cum_overcharge;
        }

        bool operator!=(const ADPTWResourceLabel& other) const { return !(*this == other); }
    };

    struct ADPTWForwardResourceLabel : public ADPTWResourceLabel {
        resource_t prev_time_shift;
        resource_t prev_overcharge;
        ADPTWForwardResourceLabel() = default;
        explicit ADPTWForwardResourceLabel(const routingblocks::Vertex& depot,
                                           resource_t battery_capacity);
    };

    struct ADPTWBackwardResourceLabel : public ADPTWResourceLabel {
        ADPTWBackwardResourceLabel() = default;
        explicit ADPTWBackwardResourceLabel(const routingblocks::Vertex& depot,
                                            resource_t battery_capacity);
    };

    class ADPTWEvaluation
        : public ConcatenationBasedEvaluationImpl<ADPTWEvaluation, ADPTWForwardResourceLabel,
                                                  ADPTWBackwardResourceLabel, ADPTWVertexData,
                                                  ADPTWArcData> {
        const resource_t _battery_capacity;
        const resource_t _storage_capacity;

        double _overcharge_penalty_factor = 1.;
        double _time_shift_penalty_factor = 1.;
        double _overload_penalty_factor = 1.;

      public:
        using fwd_label_t = routingblocks::ADPTWForwardResourceLabel;
        using bwd_label_t = routingblocks::ADPTWBackwardResourceLabel;
        using vertex_data_t = ADPTWVertexData;
        using arc_data_t = ADPTWArcData;

        ADPTWEvaluation(resource_t batteryCapacity, resource_t storageCapacity);

        enum CostComponent {
            DIST_INDEX = 0,
            OVERLOAD_INDEX = 1,
            OVERCHARGE_INDEX = 2,
            TIME_SHIFT_INDEX = 3
        };

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

        double concatenate(const fwd_label_t& fwd, const bwd_label_t& bwd,
                           const routingblocks::Vertex& vertex, const vertex_data_t& vertex_data) {
            using std::max;
            using std::min;

            resource_t distance = fwd.cum_distance + bwd.cum_distance;
            resource_t overload
                = max(fwd.cum_load + bwd.cum_load - vertex_data.demand - _storage_capacity,
                      resource_t(0));

            resource_t additional_overcharge, additional_time_shift;

            additional_time_shift
                = max(resource_t(0),
                      fwd.earliest_arrival - vertex_data.latest_arrival_time
                          - max(resource_t(0), fwd.earliest_arrival - fwd.latest_arrival))
                  + max(resource_t(0),
                        min(vertex_data.latest_arrival_time,
                            max(vertex_data.earliest_arrival_time, fwd.earliest_arrival))
                            - bwd.earliest_arrival
                            - max(bwd.latest_arrival - bwd.earliest_arrival, resource_t(0)));

            if (vertex.is_station) {
                additional_overcharge
                    = min({fwd.residual_charge_in_time,
                           max(resource_t(0),
                               bwd.earliest_arrival - fwd.earliest_arrival
                                   - max(resource_t(0), fwd.latest_arrival - bwd.latest_arrival)),
                           max(resource_t(0), fwd.earliest_arrival - bwd.latest_arrival)});
            } else {
                additional_overcharge
                    = min(_battery_capacity,
                          min(max(resource_t(0), bwd.earliest_arrival - fwd.earliest_arrival),
                              max(resource_t(0), fwd.latest_arrival - fwd.earliest_arrival)
                                  + max(resource_t(0), bwd.earliest_arrival - bwd.latest_arrival)));
            }

            additional_overcharge
                = max(resource_t(0), fwd.earliest_arrival - fwd.latest_arrival)
                  + max(resource_t(0), fwd.residual_charge_in_time + bwd.residual_charge_in_time
                                           - _battery_capacity - additional_overcharge);

            resource_t time_shift
                = fwd.prev_time_shift + bwd.cum_time_shift + additional_time_shift;
            resource_t overcharge
                = fwd.prev_overcharge + bwd.cum_overcharge + additional_overcharge;
            return _compute_cost(distance, overload, overcharge, time_shift);
        }

        [[nodiscard]] cost_t compute_cost(const fwd_label_t& label) const {
            return _compute_cost(label.cum_distance,
                                 std::max(resource_t(0), label.cum_load - _storage_capacity),
                                 label.cum_overcharge, label.cum_time_shift);
        };

        [[nodiscard]] std::vector<resource_t> get_cost_components(const fwd_label_t& fwd) const {
            return {fwd.cum_distance, std::max(resource_t(0), fwd.cum_load - _storage_capacity),
                    fwd.cum_overcharge, fwd.cum_time_shift};
        };

        [[nodiscard]] bool is_feasible(const fwd_label_t& label) const {
            return label.cum_overcharge == 0 && label.cum_time_shift == 0
                   && label.cum_load <= _storage_capacity;
        };

        [[nodiscard]] fwd_label_t propagate_forward(
            const fwd_label_t& pred_label, const routingblocks::Vertex& pred_vertex,
            const vertex_data_t& pred_vertex_data,
            [[maybe_unused]] const routingblocks::Vertex& vertex, const vertex_data_t& vertex_data,
            [[maybe_unused]] const routingblocks::Arc& arc, const arc_data_t& arc_data) const {
            using std::max;
            using std::min;
            ADPTWForwardResourceLabel propagated_label;
            auto t_ij = arc_data.duration;
            auto q_ij = arc_data.consumption;
            auto c_ij = arc_data.cost;
            resource_t l_j = vertex_data.latest_arrival_time;
            resource_t e_j = vertex_data.earliest_arrival_time;
            resource_t s_i = pred_vertex_data.service_time;

            propagated_label.cum_distance = pred_label.cum_distance + c_ij;
            propagated_label.cum_load = pred_label.cum_load + vertex_data.demand;

            propagated_label.prev_time_shift = pred_label.cum_time_shift;
            propagated_label.prev_overcharge = pred_label.cum_overcharge;

            /*
             * ###############################################################
             */
            resource_t slack
                = max(resource_t(0), e_j - pred_label.shifted_earliest_arrival - t_ij - s_i);
            resource_t add;
            if (pred_vertex.is_station) {
                assert(pred_vertex_data.service_time == 0);
                propagated_label.residual_charge_in_time
                    = min(_battery_capacity,
                          max(resource_t(0), pred_label.residual_charge_in_time - slack) + q_ij);
                add = max(resource_t(0),
                          max(resource_t(0), pred_label.residual_charge_in_time - slack) + q_ij
                              - _battery_capacity);
                propagated_label.latest_arrival
                    = max(e_j, pred_label.shifted_earliest_arrival
                                   + pred_label.residual_charge_in_time + t_ij + s_i);
            } else {
                propagated_label.residual_charge_in_time = min(
                    _battery_capacity,
                    max(resource_t(0), pred_label.residual_charge_in_time
                                           - min(slack, pred_label.shifted_latest_arrival
                                                            - pred_label.shifted_earliest_arrival))
                        + q_ij);
                add = max(
                    resource_t(0),
                    max(resource_t(0), pred_label.residual_charge_in_time
                                           - min(slack, pred_label.shifted_latest_arrival
                                                            - pred_label.shifted_earliest_arrival))
                        + q_ij - _battery_capacity);
                propagated_label.latest_arrival
                    = max(e_j, pred_label.shifted_latest_arrival + t_ij + s_i);
            }

            propagated_label.earliest_arrival
                = max(e_j, pred_label.shifted_earliest_arrival + t_ij + s_i) + add;
            propagated_label.shifted_earliest_arrival
                = min(propagated_label.earliest_arrival, min(propagated_label.latest_arrival, l_j));
            assert(propagated_label.shifted_earliest_arrival >= e_j);
            propagated_label.shifted_latest_arrival = min(l_j, propagated_label.latest_arrival);
            /*propagated_label.shifted_latest_arrival = min(
                l_j, min(propagated_label.earliest_arrival, min(propagated_label.latest_arrival,
               l_j))
                         + max(propagated_label.latest_arrival - propagated_label.earliest_arrival,
               0));*/

            propagated_label.cum_time_shift
                = pred_label.cum_time_shift
                  + max(
                      min(propagated_label.earliest_arrival, propagated_label.latest_arrival) - l_j,
                      resource_t(0));
            propagated_label.cum_overcharge
                = pred_label.cum_overcharge
                  + max(propagated_label.earliest_arrival - propagated_label.latest_arrival,
                        resource_t(0));

            return propagated_label;
        };

        [[nodiscard]] bwd_label_t propagate_backward(
            const bwd_label_t& succ_label, const routingblocks::Vertex& succ_vertex,
            [[maybe_unused]] const vertex_data_t& succ_vertex_data,
            [[maybe_unused]] const routingblocks::Vertex& vertex, const vertex_data_t& vertex_data,
            [[maybe_unused]] const routingblocks::Arc& arc, const arc_data_t& arc_data) const {
            using std::max;
            using std::min;
            ADPTWBackwardResourceLabel propagated_label;

            const auto t_ij = arc_data.duration + vertex_data.service_time;
            const auto q_ij = arc_data.consumption;
            const auto c_ij = arc_data.cost;
            resource_t l_i = vertex_data.latest_arrival_time;
            resource_t e_i = vertex_data.earliest_arrival_time;

            propagated_label.cum_distance = succ_label.cum_distance + c_ij;
            propagated_label.cum_load = succ_label.cum_load + vertex_data.demand;

            /*
             * ################################################################
             */

            resource_t slack = max(resource_t(0), succ_label.shifted_earliest_arrival - t_ij - l_i);
            resource_t add;

            if (succ_vertex.is_station) {
                assert(succ_vertex_data.service_time == 0);
                propagated_label.residual_charge_in_time
                    = min(_battery_capacity,
                          max(resource_t(0), succ_label.residual_charge_in_time - slack) + q_ij);
                add = max(resource_t(0),
                          max(resource_t(0), succ_label.residual_charge_in_time - slack) + q_ij
                              - _battery_capacity);
                propagated_label.latest_arrival
                    = min(l_i, succ_label.shifted_earliest_arrival - t_ij
                                   - propagated_label.residual_charge_in_time);
            } else {
                propagated_label.residual_charge_in_time = min(
                    _battery_capacity,
                    max(resource_t(0), succ_label.residual_charge_in_time
                                           - min(slack, succ_label.shifted_earliest_arrival
                                                            - succ_label.shifted_latest_arrival))
                        + q_ij);
                add = max(
                    resource_t(0),
                    max(resource_t(0), succ_label.residual_charge_in_time
                                           - min(slack, succ_label.shifted_earliest_arrival
                                                            - succ_label.shifted_latest_arrival))
                        + q_ij - _battery_capacity);
                propagated_label.latest_arrival
                    = min(l_i, succ_label.shifted_latest_arrival - t_ij);
            }

            propagated_label.earliest_arrival
                = min(l_i, succ_label.shifted_earliest_arrival - t_ij) - add;
            propagated_label.shifted_earliest_arrival
                = max(propagated_label.earliest_arrival, max(propagated_label.latest_arrival, e_i));
            propagated_label.shifted_latest_arrival = max(e_i, propagated_label.latest_arrival);

            propagated_label.cum_time_shift
                = succ_label.cum_time_shift
                  + max(resource_t(0), e_i
                                           - max(propagated_label.latest_arrival,
                                                 propagated_label.earliest_arrival));
            propagated_label.cum_overcharge
                = succ_label.cum_overcharge
                  + max(propagated_label.latest_arrival - propagated_label.earliest_arrival,
                        resource_t(0));

            return propagated_label;
        };

        [[nodiscard]] fwd_label_t create_forward_label(
            const Vertex& vertex, [[maybe_unused]] const vertex_data_t& vertex_data) {
            return ADPTWForwardResourceLabel(vertex, _battery_capacity);
        };
        [[nodiscard]] bwd_label_t create_backward_label(
            const Vertex& vertex, [[maybe_unused]] const vertex_data_t& vertex_data) {
            return ADPTWBackwardResourceLabel(vertex, _battery_capacity);
        };
    };

    struct ADPTWLabel {
      public:
        sul::dynamic_bitset<> visited_vertices;
        const ADPTWLabel* predecessor = nullptr;
        VertexID vertex_id = 0;
        resource_t cost = 0;
        resource_t t_min = 0;
        resource_t t_max = 0;
        resource_t rt_max = 0;
        resource_t num_stations = 0;

        void mark_station_visit() {}

        [[nodiscard]] bool visited_station() const { return num_stations > 0; }

        void clear_visits() {
            if (visited_station()) {
                visited_vertices.reset();
            } else {
                visited_vertices.reset();
                mark_station_visit();
            }
        }

        [[nodiscard]] bool visited(VertexID id) const { return visited_vertices.test(id); }

        void visit_vertex(VertexID v, bool is_station) {
            visited_vertices.set(v, true);
            num_stations += is_station;
        }

        ADPTWLabel() = default;
        explicit ADPTWLabel(size_t number_of_vertices) : visited_vertices(number_of_vertices) {}
        ADPTWLabel(const ADPTWLabel& predecessor, VertexID vertex_id)
            : visited_vertices(predecessor.visited_vertices),
              predecessor(&predecessor),
              vertex_id(vertex_id),
              cost(predecessor.cost),
              t_min(predecessor.t_min),
              t_max(predecessor.t_max),
              rt_max(predecessor.rt_max),
              num_stations(predecessor.num_stations) {}

        [[nodiscard]] bool root_label() const { return predecessor == nullptr; }
        [[nodiscard]] resource_t earliest_arrival_time() const { return t_min; }

        friend std::ostream& operator<<(std::ostream& out, const ADPTWLabel& l) {
            out << "[c: " << l.cost << ", t_min: " << l.t_min << ", t_max: " << l.t_max
                << ", rt_max: " << l.rt_max << " {" << l.num_stations << "}]";
            return out;
        }
    };

    template <> class Propagator<ADPTWLabel> {
        const Instance* _instance;
        resource_t _battery_capacity;

      public:
        explicit Propagator(const Instance& instance, resource_t battery_capacity)
            : _instance(&instance), _battery_capacity(battery_capacity){};

        std::optional<ADPTWLabel> propagate(const ADPTWLabel& predecessor, const Vertex& origin,
                                            const Vertex& target, [[maybe_unused]] const Arc& arc) {
            using std::max;
            using std::min;
            VertexID target_id = target.id;
            const auto& origin_vertex_data = origin.get_data<ADPTWVertexData>();
            const auto& target_vertex_data = origin.get_data<ADPTWVertexData>();
            const auto& arc_data = origin.get_data<ADPTWArcData>();

            const auto Q = _battery_capacity;
            const auto e_j = target_vertex_data.earliest_arrival_time;
            const auto l_j = target_vertex_data.latest_arrival_time;
            const auto t_ij = arc_data.duration + origin_vertex_data.service_time;
            const auto q_ij = arc_data.consumption;

            // Avoid cycling
            if (predecessor.visited(target_id)) {
                return {};
            }

            ADPTWLabel label(predecessor, target_id);

            // Reset visited stations when reaching a customer.
            if (target.customer()) {
                label.clear_visits();
            }

            label.visit_vertex(target_id, target.station());

            label.cost += arc_data.cost;

            resource_t s_ij;

            if (origin.is_station) {
                s_ij
                    = max(resource_t(0), min(e_j - (predecessor.t_min + t_ij), predecessor.rt_max));
                label.t_max = min(l_j, max(e_j, predecessor.t_min + predecessor.rt_max + t_ij));
            } else {
                s_ij = max(resource_t(0), min(e_j - (predecessor.t_min + t_ij),
                                              predecessor.t_max - predecessor.t_min));
                label.t_max = min(l_j, max(e_j, predecessor.t_max + t_ij));
            }

            if (!predecessor.visited_station()) {  // S_p == 0
                label.t_min = max(e_j, predecessor.t_min + t_ij);
                label.rt_max = predecessor.rt_max + q_ij;
            } else {  // S_p > 0
                auto r_ij
                    = max(resource_t(0), max(resource_t(0), predecessor.rt_max - s_ij) + q_ij - Q);
                label.t_min = max(e_j, predecessor.t_min + t_ij) + r_ij;
                label.rt_max = std::min(Q, max(resource_t(0), predecessor.rt_max + s_ij + q_ij));
            }

            if (label.t_min > l_j || label.t_min > label.t_max || label.rt_max > Q) {
                return {};
            }

            return label;
        }

        bool dominates(const ADPTWLabel& label, const ADPTWLabel& other) {
            return label.cost <= other.cost && label.t_min <= other.t_min
                   && (label.rt_max - (label.t_max - label.t_min)
                       <= (other.rt_max - (other.t_max - other.t_min)))
                   && label.rt_max + label.t_min <= other.rt_max + other.t_min;
        }

        bool cheaper_than(const ADPTWLabel& label, const ADPTWLabel& other) {
            if (label.cost == other.cost) {
                return label.num_stations < other.num_stations;
            }
            return label.cost < other.cost;
        }

        bool should_order_before(const ADPTWLabel& label, const ADPTWLabel& other) {
            return reinterpret_cast<const ADPTWLabel&>(label).earliest_arrival_time()
                   < reinterpret_cast<const ADPTWLabel&>(other).earliest_arrival_time();
        }

        std::vector<VertexID> extract_path(const ADPTWLabel& sink_label) {
            std::vector<VertexID> route;
            const ADPTWLabel* label = &reinterpret_cast<const ADPTWLabel&>(sink_label);
            for (;; label = label->predecessor) {
                route.push_back(label->vertex_id);
                if (label->root_label()) break;
            }
            std::reverse(route.begin(), route.end());
            return route;
        }

        bool is_final_label(const ADPTWLabel& _label) {
            const auto& label = reinterpret_cast<const ADPTWLabel&>(_label);
            return label.vertex_id == _instance->Depot().id && !label.root_label();
        }

        void prepare(const std::vector<VertexID>&) {}

        ADPTWLabel create_root_label() { return ADPTWLabel{_instance->NumberOfVertices()}; }
    };

}  // namespace routingblocks
#endif  // routingblocks_ADPTWEVALUATION_H
