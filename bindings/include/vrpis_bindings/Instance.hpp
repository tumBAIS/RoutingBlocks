
#ifndef vrpis_vrpisINSTANCE_HPP
#define vrpis_vrpisINSTANCE_HPP

#include <pybind11/pybind11.h>

namespace vrpis::bindings {

    void bind_vrpis_instance(pybind11::module& m);
}

#endif  // vrpis_vrpisINSTANCE_HPP
