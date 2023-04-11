
#ifndef ROUTINGBLOCKS_EVALUATION_H
#define ROUTINGBLOCKS_EVALUATION_H

#include <routingblocks/Instance.h>
#include <routingblocks/arc.h>
#include <routingblocks/node.h>
#include <routingblocks/types.h>
#include <routingblocks/vertex.h>

#include <memory>
#include <span>
#include <vector>

namespace routingblocks {

    class Evaluation : public std::enable_shared_from_this<Evaluation> {
      public:
        using label_holder_t = Node::label_holder_t;

        virtual cost_t evaluate(const Instance& instance, std::span<const route_segment> segments)
            = 0;

        [[nodiscard]] virtual cost_t compute_cost(const label_holder_t& label) const = 0;
        [[nodiscard]] virtual bool is_feasible(const label_holder_t& label) const = 0;
        [[nodiscard]] virtual std::vector<resource_t> get_cost_components(
            const label_holder_t& label) const
            = 0;

        [[nodiscard]] virtual label_holder_t propagate_forward(
            const label_holder_t& pred_label, const routingblocks::Vertex& pred_vertex,
            const routingblocks::Vertex& vertex, const routingblocks::Arc& arc) const
            = 0;

        [[nodiscard]] virtual label_holder_t propagate_backward(
            const label_holder_t& succ_label, const routingblocks::Vertex& succ_vertex,
            const routingblocks::Vertex& vertex, const routingblocks::Arc& arc) const
            = 0;

        [[nodiscard]] virtual label_holder_t create_forward_label(const Vertex& vertex) = 0;
        [[nodiscard]] virtual label_holder_t create_backward_label(const Vertex& vertex) = 0;

        virtual ~Evaluation() = default;
    };

    /**
     * Specializes the generic Evaluation class to evaluation functions that provide 2EVAL (cf.,
     * Vidal 2014, https://doi.org/10.1016/j.ejor.2013.09.045)
     */
    class ConcatenationBasedEvaluation : public Evaluation {
        virtual cost_t concatenate(const label_holder_t& fwd, const label_holder_t& bwd,
                                   const routingblocks::Vertex& vertex)
            = 0;

        cost_t evaluate(const Instance& instance,
                        const std::span<const route_segment> segments) final {
            auto next_segment = segments.begin();
            // Last segment with a valid forward label
            auto cur_segment = next_segment++;
            // First segment with a valid bwd label
            auto last_segment = std::prev(segments.end());
            // Last node with a valid forward label
            const Node* pred_node = &cur_segment->back();
            label_holder_t fwd_label = pred_node->forward_label();
            for (; next_segment != last_segment; cur_segment = next_segment++) {
                for (const Node& next_node : *next_segment) {
                    fwd_label = propagate_forward(
                        fwd_label, pred_node->vertex(), next_node.vertex(),
                        instance.getArc(pred_node->vertex_id(), next_node.vertex_id()));
                    pred_node = &next_node;
                }
            }
            // First node with a valid backward label
            const Node& concatenation_node = next_segment->front();
            fwd_label = propagate_forward(
                fwd_label, pred_node->vertex(), concatenation_node.vertex(),
                instance.getArc(pred_node->vertex_id(), concatenation_node.vertex_id()));
            return concatenate(fwd_label, concatenation_node.backward_label(),
                               concatenation_node.vertex());
        };
    };

    /**
     * Specializes the generic Evaluation class to evaluation functions that do not provide
     * efficient means to calculate the concatenation of a sequence of route segments. Essentially
     * computes the cost of concatenated route segments by propagating a forward label across
     * all route segments.
     */
    class ForwardBasedEvaluation : public Evaluation {
        cost_t evaluate(const Instance& instance,
                        const std::span<const route_segment> segments) final {
            auto next_segment = segments.begin();
            // Last segment with a valid forward label
            auto cur_segment = next_segment++;
            // First segment with a valid bwd label
            const Node* pred_node = &cur_segment->back();
            label_holder_t fwd_label = pred_node->forward_label();
            for (; next_segment != segments.end(); cur_segment = next_segment++) {
                for (const Node& next_node : *next_segment) {
                    fwd_label = propagate_forward(
                        fwd_label, pred_node->vertex(), next_node.vertex(),
                        instance.getArc(pred_node->vertex_id(), next_node.vertex_id()));
                    pred_node = &next_node;
                }
            }
            return compute_cost(fwd_label);
        };
    };

    template <class Impl, class fwd_label_t, class bwd_label_t, class vertex_data_t,
              class arc_data_t>
    class ConcatenationBasedEvaluationImpl : public ConcatenationBasedEvaluation {
        Impl& get_impl() { return static_cast<Impl&>(*this); }
        const Impl& get_impl() const { return static_cast<const Impl&>(*this); }

      public:
        using label_holder_t = detail::label_holder;

        [[nodiscard]] cost_t concatenate(const label_holder_t& fwd, const label_holder_t& bwd,
                                         const routingblocks::Vertex& vertex) final {
            return get_impl().concatenate(fwd.get<fwd_label_t>(), bwd.get<bwd_label_t>(), vertex,
                                          vertex.get_data<vertex_data_t>());
        }

        [[nodiscard]] cost_t compute_cost(const label_holder_t& label) const final {
            return get_impl().compute_cost(label.get<fwd_label_t>());
        }

        [[nodiscard]] std::vector<resource_t> get_cost_components(
            const label_holder_t& label) const final {
            return get_impl().get_cost_components(label.get<fwd_label_t>());
        }

        [[nodiscard]] bool is_feasible(const label_holder_t& label) const final {
            return get_impl().is_feasible(label.get<fwd_label_t>());
        }

        [[nodiscard]] label_holder_t propagate_forward(const label_holder_t& pred_label,
                                                       const routingblocks::Vertex& pred_vertex,
                                                       const routingblocks::Vertex& vertex,
                                                       const routingblocks::Arc& arc) const final {
            const auto& pred_vertex_data = pred_vertex.get_data<vertex_data_t>();
            const auto& vertex_data = vertex.get_data<vertex_data_t>();
            const auto& arc_data = arc.get_data<arc_data_t>();
            return label_holder_t(std::make_shared<fwd_label_t>(get_impl().propagate_forward(
                pred_label.get<fwd_label_t>(), pred_vertex, pred_vertex_data, vertex, vertex_data,
                arc, arc_data)));
        }

        [[nodiscard]] label_holder_t propagate_backward(const label_holder_t& succ_label,
                                                        const routingblocks::Vertex& succ_vertex,
                                                        const routingblocks::Vertex& vertex,
                                                        const routingblocks::Arc& arc) const final {
            const auto& succ_vertex_data = succ_vertex.get_data<vertex_data_t>();
            const auto& vertex_data = vertex.get_data<vertex_data_t>();
            const auto& arc_data = arc.get_data<arc_data_t>();
            return label_holder_t(std::make_shared<bwd_label_t>(get_impl().propagate_backward(
                succ_label.get<bwd_label_t>(), succ_vertex, succ_vertex_data, vertex, vertex_data,
                arc, arc_data)));
        }

        [[nodiscard]] label_holder_t create_forward_label(const Vertex& vertex) final {
            const auto& vertex_data = vertex.get_data<vertex_data_t>();
            return label_holder_t(std::make_shared<fwd_label_t>(
                get_impl().create_forward_label(vertex, vertex_data)));
        }

        [[nodiscard]] label_holder_t create_backward_label(const Vertex& vertex) final {
            const auto& vertex_data = vertex.get_data<vertex_data_t>();
            return label_holder_t(std::make_shared<bwd_label_t>(
                get_impl().create_backward_label(vertex, vertex_data)));
        }
    };

    template <class eval_t>
    concept is_evaluation = std::derived_from<eval_t, Evaluation>;
}  // namespace routingblocks

#endif  // ROUTINGBLOCKS_EVALUATION_H
