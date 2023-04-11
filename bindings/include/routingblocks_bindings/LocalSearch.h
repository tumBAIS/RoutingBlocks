
#ifndef routingblocks_BINDINGS_LOCALSEARCH_H
#define routingblocks_BINDINGS_LOCALSEARCH_H

#include <pybind11/pybind11.h>

namespace routingblocks::bindings {

    void bind_local_search(pybind11::module& m);
    void bind_neighborhood_structures(pybind11::module& m);
}  // namespace routingblocks::bindings

#endif  // routingblocks_BINDINGS_LOCALSEARCH_H
