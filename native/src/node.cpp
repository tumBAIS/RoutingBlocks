#include <routingblocks/evaluation.h>
#include <routingblocks/node.h>

namespace routingblocks {

    void Node::update_forward(Evaluation& evaluation, const Node& pred_node, const Arc& arc) {
        _forward_label = evaluation.propagate_forward(pred_node._forward_label, *pred_node._vertex,
                                                      *_vertex, arc);
    }
    void Node::update_backward(Evaluation& evaluation, const Node& succ_node, const Arc& arc) {
        _backward_label = evaluation.propagate_backward(succ_node._backward_label,
                                                        *succ_node._vertex, *_vertex, arc);
    }
    cost_t Node::cost(Evaluation& evaluation) const {
        return evaluation.compute_cost(_forward_label);
    }
    std::vector<resource_t> Node::cost_components(Evaluation& evaluation) const {
        return evaluation.get_cost_components(_forward_label);
    }
    bool Node::feasible(Evaluation& evaluation) const {
        return evaluation.is_feasible(_forward_label);
    }
}  // namespace routingblocks