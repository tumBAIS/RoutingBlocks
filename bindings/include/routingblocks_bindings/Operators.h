
#ifndef routingblocks_BINDINGS_OPERATORS_H
#define routingblocks_BINDINGS_OPERATORS_H

#include <pybind11/pybind11.h>
#include <routingblocks/LocalSearch.h>

#include <routingblocks_bindings/binding_helpers.hpp>

namespace routingblocks::bindings {

    void bind_operators(pybind11::module& m);
}  // namespace routingblocks::bindings

BIND_LIFETIME_PYTHON(routingblocks::Operator, "Operator")
BIND_LIFETIME_PYTHON(routingblocks::Move, "Move")

#endif  // routingblocks_BINDINGS_OPERATORS_H
