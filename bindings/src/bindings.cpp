#include <pybind11/pybind11.h>
#include <routingblocks_bindings/Evaluation.h>
#include <routingblocks_bindings/Labeling.h>
#include <routingblocks_bindings/LocalSearch.h>
#include <routingblocks_bindings/Operators.h>
#include <routingblocks_bindings/Solution.h>
#include <routingblocks_bindings/large_neighborhood.h>
#include <routingblocks_bindings/utility.h>

#include <routingblocks_bindings/Instance.hpp>

using namespace routingblocks::bindings;

#define STRINGIFY(x) #x
#define PREPROCESSOR_TO_STRING(x) STRINGIFY(x)

#ifndef routingblocks_VERSION
#    define routingblocks_VERSION "dev"
#endif

PYBIND11_MODULE(routingblocks_MODULE_NAME, m) {
    m.attr("__version__") = PREPROCESSOR_TO_STRING(routingblocks_VERSION);

    bind_utility(m);
    // Bind classes
    bind_routingblocks_instance(m);

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

    // routingblocks
    bind_large_neighborhood(m);
}