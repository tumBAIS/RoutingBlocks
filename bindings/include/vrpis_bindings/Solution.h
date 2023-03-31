
#ifndef vrpis_BINDINGS_SOLUTION_H
#define vrpis_BINDINGS_SOLUTION_H

#include <pybind11/pybind11.h>

namespace vrpis::bindings {
    void bind_node(pybind11::module& m);
    void bind_route(pybind11::module& m);
    void bind_solution(pybind11::module& m);

    void bind_solution_functions(pybind11::module& m);
}  // namespace vrpis::bindings

#endif  // vrpis_BINDINGS_SOLUTION_H
