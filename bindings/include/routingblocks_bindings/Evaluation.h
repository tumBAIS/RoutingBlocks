
#ifndef routingblocks_EVALUATION_H
#define routingblocks_EVALUATION_H

#include <pybind11/smart_holder.h>
#include <routingblocks/evaluation.h>

PYBIND11_SMART_HOLDER_TYPE_CASTERS(routingblocks::Evaluation)

namespace routingblocks::bindings {

    void bind_evaluation(pybind11::module& m);
}

#endif  // routingblocks_EVALUATION_H
