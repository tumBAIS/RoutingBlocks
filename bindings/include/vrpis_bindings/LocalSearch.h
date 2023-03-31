
#ifndef vrpis_BINDINGS_LOCALSEARCH_H
#define vrpis_BINDINGS_LOCALSEARCH_H

#include <pybind11/pybind11.h>

namespace vrpis::bindings {

    void bind_local_search(pybind11::module& m);
    void bind_neighborhood_structures(pybind11::module& m);
}  // namespace vrpis::bindings

#endif  // vrpis_BINDINGS_LOCALSEARCH_H
