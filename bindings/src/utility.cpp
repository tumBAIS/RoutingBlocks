#include <pybind11/stl.h>
#include <routingblocks/insertion_cache.h>
#include <routingblocks/lns_operators.h>
#include <routingblocks/removal_cache.h>
#include <routingblocks/utility/random.h>
#include <routingblocks_bindings/utility.h>

namespace routingblocks::bindings {
    void bind_removal_cache(pybind11::module_& m) {
        using cache_t = routingblocks::utility::removal_cache<>;

        pybind11::class_<cache_t::move_t>(m, "RemovalMove")
            .def(pybind11::init<VertexID, NodeLocation, resource_t>())
            .def_readwrite("vertex_id", &cache_t::move_t::vertex_id)
            .def_readwrite("node_location", &cache_t::move_t::node_location,
                           pybind11::return_value_policy::reference_internal)
            .def_readwrite("delta_cost", &cache_t::move_t::delta_cost)
            .def("__eq__",
                 [](const cache_t::move_t& lhs, const cache_t::move_t& rhs) { return lhs == rhs; });

        pybind11::class_<cache_t>(m, "RemovalCache")
            .def(pybind11::init<const Instance&>(), pybind11::keep_alive<1, 2>())
            .def("clear", &cache_t::clear, "Resets the cache.")
            .def("rebuild", &cache_t::rebuild, "Rebuilds the cache from the given solution.")
            .def("invalidate_route", &cache_t::invalidate_route,
                 "Removes any moves that were on the passed route and adds moves according to the "
                 "new route.")
            .def_property_readonly(
                "moves_in_order",
                [](const cache_t& cache) {
                    return std::vector<cache_t::move_t>(cache.begin(), cache.end());
                },
                "Returns the list of moves in the cache ordered by their cost delta in "
                "increasing order.");
    }

    void bind_insertion_cache(pybind11::module_& m) {
        using cache_t = routingblocks::utility::insertion_cache<>;

        pybind11::class_<cache_t::move_t>(m, "InsertionMove")
            .def(pybind11::init<VertexID, NodeLocation, resource_t>())
            .def_readwrite("vertex_id", &cache_t::move_t::vertex_id)
            .def_readwrite("after_node", &cache_t::move_t::after_node,
                           pybind11::return_value_policy::reference_internal)
            .def_readwrite("delta_cost", &cache_t::move_t::delta_cost)
            .def("__eq__",
                 [](const cache_t::move_t& lhs, const cache_t::move_t& rhs) { return lhs == rhs; });

        pybind11::class_<cache_t>(m, "InsertionCache")
            .def(pybind11::init<const Instance&>(), pybind11::keep_alive<1, 2>())
            .def("clear", &cache_t::clear, "Resets the cache.")
            .def(
                "rebuild",
                [](cache_t& cache, Evaluation& evaluation, const Solution& solution,
                   const std::vector<VertexID>& tracked_vertices) {
                    cache.rebuild(evaluation, solution, tracked_vertices.begin(),
                                  tracked_vertices.end());
                },
                "Rebuilds the cache from the given solution, tracking insertions of the passed "
                "vertex ids.")
            .def("invalidate_route", &cache_t::invalidate_route,
                 "Removes any moves that were on the passed route and adds moves according to the "
                 "new route.")
            .def(
                "get_best_insertions_for_vertex",
                [](const cache_t& cache, VertexID vertex_id) {
                    return std::vector<cache_t::move_t>(
                        cache.best_insertions_for_vertex_begin(vertex_id),
                        cache.best_insertions_for_vertex_end(vertex_id));
                },
                "Returns the list of insertions of the corresponding vertex ordered by their cost "
                "delta in increasing order.")
            .def("stop_tracking", &cache_t::stop_tracking,
                 "Stops tracking insertions of the passed vertex id.")
            .def("tracks_vertex", &cache_t::tracks,
                 "Returns whether the cache is tracking insertions of the passed vertex id.")
            .def_property_readonly(
                "tracked_vertices",
                [](const cache_t& cache) -> std::vector<VertexID> {
                    return std::vector<VertexID>(cache.tracked_vertices_begin(),
                                                 cache.tracked_vertices_end());
                },
                "Returns the list of vertex ids that are currently tracked.")
            .def_property_readonly(
                "moves_in_order",
                [](const cache_t& cache) {
                    return std::vector<cache_t::move_t>(cache.begin(), cache.end());
                },
                "Returns the list of moves in the cache ordered by their cost delta in "
                "increasing order.");
    }

    void bind_random(pybind11::module_& m) {
        pybind11::class_<routingblocks::utility::random>(m, "Random")
            .def(pybind11::init<>(),
                 "Initialize random number generator with a seed based on the current time.")
            .def(pybind11::init<uint64_t>(), "Initialize the random number generator with a seed.")
            .def(
                "randint",
                [](routingblocks::utility::random& r, size_t min, size_t max) {
                    return r.generateInt(min, max);
                },
                "Generates a random integer between min and max")
            .def(
                "uniform",
                [](routingblocks::utility::random& r, double min, double max) {
                    return r.uniform(min, max);
                },
                "Generates a random float between min and max");
    }

    void bind_algorithms(pybind11::module_& m) {
        m.def("sample_locations", &routingblocks::lns::operators::sample_positions,
              "Samples node locations for the passed solution.");
    }

    void bind_utility(pybind11::module_& m) {
        bind_random(m);
        bind_removal_cache(m);
        bind_insertion_cache(m);
        bind_algorithms(m);
    }
}  // namespace routingblocks::bindings