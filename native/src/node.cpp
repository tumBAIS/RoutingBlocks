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