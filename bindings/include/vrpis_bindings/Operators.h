
#ifndef vrpis_BINDINGS_OPERATORS_H
#define vrpis_BINDINGS_OPERATORS_H

#include <pybind11/pybind11.h>

#include "vrpis/LocalSearch.h"
#include "vrpis_bindings/binding_helpers.hpp"

namespace vrpis::bindings {

    void bind_operators(pybind11::module& m);
}  // namespace vrpis::bindings

BIND_LIFETIME_PYTHON(vrpis::Operator, "Operator")
BIND_LIFETIME_PYTHON(vrpis::Move, "Move")

#endif  // vrpis_BINDINGS_OPERATORS_H
