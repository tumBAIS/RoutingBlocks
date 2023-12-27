// Copyright (c) 2023 Patrick S. Klein (@libklein)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <pybind11/pybind11.h>
#include <routingblocks_bindings/Evaluation.h>
#include <routingblocks_bindings/Labeling.h>
#include <routingblocks_bindings/LocalSearch.h>
#include <routingblocks_bindings/Operators.h>
#include <routingblocks_bindings/Solution.h>
#include <routingblocks_bindings/large_neighborhood.h>
#include <routingblocks_bindings/utility.h>

#include <routingblocks_bindings/Instance.hpp>

#include "routingblocks_bindings/specializations/ADPTW.h"
#include "routingblocks_bindings/specializations/NIFTW.h"

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
    bind_pivoting_rule(m);
    // LS Operators
    bind_operators(m);

    // Solution
    bind_node(m);
    bind_route(m);
    bind_solution(m);
    bind_solution_functions(m);

    // Labeling
    bind_labeling(m);

    // ALNS
    bind_large_neighborhood(m);

    // Specializations
    bind_adptw(m);
    bind_niftw(m);
}