#include <pybind11/pybind11.h>

#include "vrpis_bindings/Evaluation.h"
#include "vrpis_bindings/Instance.hpp"
#include "vrpis_bindings/Labeling.h"
#include "vrpis_bindings/LocalSearch.h"
#include "vrpis_bindings/Operators.h"
#include "vrpis_bindings/Solution.h"
#include "vrpis_bindings/large_neighborhood.h"
#include "vrpis_bindings/utility.h"

using namespace vrpis::bindings;

#define STRINGIFY(x) #x
#define PREPROCESSOR_TO_STRING(x) STRINGIFY(x)

#ifndef VRPIS_VERSION
#    define VRPIS_VERSION "dev"
#endif

PYBIND11_MODULE(VRPIS_MODULE_NAME, m) {
    m.attr("__version__") = PREPROCESSOR_TO_STRING(VRPIS_VERSION);

    bind_utility(m);
    // Bind classes
    bind_vrpis_instance(m);

    // Evaluation
    bind_evaluation(m);

    // Local search
    bind_neighborhood_structures(m);
    bind_local_search(m);
    // LS Operators
    bind_operators(m);

    // Solution
    bind_node(m);
    bind_route(m);
    bind_solution(m);
    bind_solution_functions(m);

    // Labeling
    bind_labeling(m);

    // vrpis
    bind_large_neighborhood(m);
}