
#ifndef routingblocks_BINDINGS_SOLUTION_H
#define routingblocks_BINDINGS_SOLUTION_H

#include <pybind11/pybind11.h>

namespace routingblocks::bindings {
    void bind_node(pybind11::module& m);
    void bind_route(pybind11::module& m);
    void bind_solution(pybind11::module& m);

    void bind_solution_functions(pybind11::module& m);
}  // namespace routingblocks::bindings

#endif  // routingblocks_BINDINGS_SOLUTION_H
