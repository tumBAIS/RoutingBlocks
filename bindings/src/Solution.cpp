#include "vrpis_bindings/Solution.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "vrpis/Solution.h"
#include "vrpis_bindings/binding_helpers.hpp"

namespace vrpis::bindings {

    void bind_node(pybind11::module& m) {
        pybind11::class_<vrpis::Node>(m, "Node")
            .def(pybind11::init([](const vrpis::Vertex& vertex, pybind11::object fwd_label,
                                   pybind11::object bwd_label) {
                     return vrpis::Node{vertex,
                                        {std::make_shared<pybind11::object>(std::move(fwd_label))},
                                        {std::make_shared<pybind11::object>(std::move(bwd_label))}};
                 }),
                 "Creates a node tracking the given vertex and initializes forward and backward"
                 "labels.")
            .def("update_forward", &vrpis::Node::update_forward,
                 "Updates the forward label of this node using the given predecessor node and arc.")
            .def("update_backward", &vrpis::Node::update_backward,
                 "Updates the backward label of this node using the given successor node and arc.")
            .def_property_readonly("vertex_id", &vrpis::Node::vertex_id, "The vertex ID")
            .def_property_readonly("vertex_strid", &vrpis::Node::vertex_strid, "The vertex StrID")
            .def_property_readonly("vertex", &vrpis::Node::vertex,
                                   "The vertex associated with this node")
            .def("cost", &vrpis::Node::cost, "The total route cost up to this node")
            .def("cost_components", &vrpis::Node::cost_components,
                 "The cost components of the route up to this node")
            .def("feasible", &vrpis::Node::feasible, "Whether the route up to the node is feasible")
            .def_property_readonly(
                "forward_label",
                [](const Node& node) -> const pybind11::object& {
                    return node.forward_label().get<pybind11::object>();
                },
                "Forward label at the node", pybind11::return_value_policy::reference_internal)
            .def_property_readonly(
                "backward_label",
                [](const Node& node) -> const pybind11::object& {
                    return node.backward_label().get<pybind11::object>();
                },
                "Backward label at the node", pybind11::return_value_policy::reference_internal)
            .def_property_readonly("__str__", &vrpis::Node::vertex_strid);
    }

    void bind_route(pybind11::module& m) {
        pybind11::class_<vrpis::Route>(m, "Route")
            .def(pybind11::init<std::shared_ptr<Evaluation>, const Instance&>(),
                 "Creates an empty route.")
            .def_property_readonly("cost", &vrpis::Route::cost, "The cost of the route.")
            .def_property_readonly("cost_components", &vrpis::Route::cost_components,
                                   "The cost components of the route.")
            .def_property_readonly("feasible", &vrpis::Route::feasible,
                                   "Whether the route is feasible.")
            .def_property_readonly("empty", &vrpis::Route::empty, "Whether the route is empty.")
            .def_property_readonly("modification_timestamp", &vrpis::Route::modification_timestamp,
                                   "The route modification_timestamp. May be used for caching.")
            .def("__len__", &vrpis::Route::size, "The number of vertices in the route.")
            .def("__copy__", [](const Route& r) { return Route(r); })
            .def("__deepcopy__", [](const Route& r, const pybind11::dict&) { return Route(r); })
            .def_property_readonly(
                "end_depot", [](const Route& r) -> const vrpis::Node& { return *r.end_depot(); },
                "The depot at the end of the route.",
                pybind11::return_value_policy::reference_internal)
            .def_property_readonly(
                "depot", [](const Route& r) -> const vrpis::Node& { return *r.begin(); },
                "Starting depot of the route.", pybind11::return_value_policy::reference_internal)
            .def(
                "__getitem__",
                [](const vrpis::Route& route, size_t pos) -> const vrpis::Node& {
                    return *std::next(route.begin(), pos);
                },
                "The node at the given index.", pybind11::return_value_policy::reference_internal)
            .def(
                "__iter__",
                [](const Route& route) {
                    return pybind11::make_iterator(route.begin(), route.end());
                },
                pybind11::return_value_policy::reference_internal)
            .def("__str__", ::bindings::helpers::ostream_to_string<vrpis::Route>)
            .def("__repr__", ::bindings::helpers::ostream_to_string<vrpis::Route>)
            .def(
                "remove_segment",
                [](Route& route, size_t begin_pos, size_t end_pos) {
                    auto ret = route.remove_segment(std::next(route.begin(), begin_pos),
                                                    std::next(route.begin(), end_pos));
                    return std::distance(route.begin(), ret);
                },
                "Removes the segment of the route between the given iterators.")
            .def(
                "remove_vertices",
                [](Route& route, const std::vector<NodeLocation>& vertices) {
                    route.remove_vertices(vertices.begin(), vertices.end());
                },
                "Removes the given vertices from the route.")
            .def(
                "insert_segment_after",
                [](Route& route, size_t pos, const std::vector<Node>& nodes) {
                    auto ret = route.insert_segment_after(std::next(route.begin(), pos),
                                                          nodes.begin(), nodes.end());
                    return std::distance(route.begin(), ret);
                },
                "Inserts the given nodes after the given position.")
            .def("insert_vertices_after",
                 [](Route& route, pybind11::iterable& vertex_ids_and_locations_list) {
                     std::vector<std::pair<VertexID, NodeLocation>> vertex_ids_and_locations;

                     for (const auto& vertex_id_and_location : vertex_ids_and_locations_list) {
                         vertex_ids_and_locations.push_back(
                             vertex_id_and_location.cast<std::pair<VertexID, NodeLocation>>());
                     }

                     route.insert_vertices_after(vertex_ids_and_locations.begin(),
                                                 vertex_ids_and_locations.end());
                 })
            .def("exchange_segments",
                 [](Route& route, size_t begin_pos, size_t end_pos, size_t other_begin_pos,
                    size_t other_end_pos, Route& other) {
                     auto begin = std::next(route.begin(), begin_pos);
                     auto end = std::next(route.begin(), end_pos);
                     auto other_begin = std::next(other.begin(), other_begin_pos);
                     auto other_end = std::next(other.begin(), other_end_pos);
                     if (&route != &other) {
                         // Inter-route exchange
                         return route.exchange_segments(begin, end, other_begin, other_end, other);
                     } else {
                         // Intra-route exchange
                         return route.exchange_segments(begin, end, other_begin, other_end);
                     }
                 })
            .def("update", pybind11::overload_cast<>(&vrpis::Route::update), "Updates the route.")
            .def("__eq__", &vrpis::Route::operator==, "Whether the routes are equal.")
            .def("__ne__", &vrpis::Route::operator!=, "Whether the routes are not equal.");

        m.def("create_route", &vrpis::create_route_from_vector,
              "Creates a route from the given vertices.");
    }

    void bind_node_location(pybind11::module_& m) {
        pybind11::class_<vrpis::NodeLocation>(m, "NodeLocation")
            .def(pybind11::init<unsigned int, unsigned int>())
            .def("__getitem__",
                 [](const vrpis::NodeLocation& location, size_t pos) {
                     switch (pos) {
                         case 0:
                             return location.route;
                         case 1:
                             return location.position;
                         default:
                             throw std::out_of_range("Index out of range");
                     }
                 })
            .def("__len__", [](const vrpis::NodeLocation&) { return 2; })
            .def_readwrite("route", &vrpis::NodeLocation::route, "The route index.")
            .def_readwrite("position", &vrpis::NodeLocation::position, "The position in the route.")
            .def("__eq__", &vrpis::NodeLocation::operator==,
                 "Whether the node locations are equal.")
            .def("__ne__", &vrpis::NodeLocation::operator!=,
                 "Whether the node locations are not equal.")
            .def("__lt__", &vrpis::NodeLocation::operator<,
                 "Whether the node locations compare lexicographically smaller. Route index is "
                 "ordered before node position.")
            .def("__str__", ::bindings::helpers::ostream_to_string<vrpis::NodeLocation>)
            .def("__repr__", ::bindings::helpers::ostream_to_string<vrpis::NodeLocation>);
    }

    void bind_solution(pybind11::module_& m) {
        bind_node_location(m);

        pybind11::class_<vrpis::Solution>(m, "Solution")
            .def(pybind11::init<std::shared_ptr<Evaluation>, const Instance&, size_t>(),
                 "Creates an empty solution with the specified number of routes.")
            .def(pybind11::init<std::shared_ptr<Evaluation>, const Instance&, std::vector<Route>>(),
                 "Creates a solution from the specified routes.")
            .def_property_readonly("cost", &vrpis::Solution::cost, "The cost of the solution.")
            .def_property_readonly("cost_components", &vrpis::Solution::cost_components,
                                   "The cost components of the solution.")
            .def_property_readonly("feasible", &vrpis::Solution::feasible,
                                   "Whether the solution is "
                                   "feasible.")
            .def("__copy__", [](const Solution& s) { return Solution(s); })
            .def("__deepcopy__",
                 [](const Solution& s, const pybind11::dict&) -> Solution { return Solution(s); })
            .def(
                "__iter__",
                [](const Solution& solution) {
                    return pybind11::make_iterator(solution.begin(), solution.end());
                },
                pybind11::return_value_policy::reference_internal)
            .def_property_readonly(
                "routes",
                [](const Solution& solution) {
                    return pybind11::make_iterator(solution.begin(), solution.end());
                },
                "Iterator over the routes in the solution.",
                pybind11::return_value_policy::reference_internal)
            .def("__len__", &vrpis::Solution::size, "The number of routes in the solution.")
            .def_property_readonly(
                "number_of_non_depot_nodes",
                [](const Solution& sol) { return vrpis::number_of_nodes(sol, false); },
                "The number of non-depot nodes in the solution.")
            .def_property_readonly(
                "number_of_insertion_points",
                [](const Solution& sol) { return vrpis::number_of_nodes(sol, true); },
                "The number of possible insertion points in the solution.")
            .def_property_readonly(
                "insertion_points",
                [](const Solution& sol) {
                    std::vector<NodeLocation> locations;
                    locations.reserve(vrpis::number_of_nodes(sol, true));
                    size_t route_index = 0;
                    for (auto route = sol.begin(); route != sol.end(); ++route, ++route_index) {
                        size_t node_position = 0;
                        for (auto node = route->begin(); node != route->end_depot();
                             ++node, ++node_position) {
                            locations.emplace_back(route_index, node_position);
                        }
                    }
                    return locations;
                },
                pybind11::return_value_policy::move,
                "A list of possible insertion points in the solution.")
            .def_property_readonly(
                "non_depot_nodes",
                [](const Solution& sol) {
                    std::vector<NodeLocation> locations;
                    locations.reserve(vrpis::number_of_nodes(sol, true));
                    size_t route_index = 0;
                    for (auto route = sol.begin(); route != sol.end(); ++route, ++route_index) {
                        size_t node_position = 1;
                        for (auto node = std::next(route->begin()); node != route->end_depot();
                             ++node, ++node_position) {
                            locations.emplace_back(route_index, node_position);
                        }
                    }
                    return locations;
                },
                pybind11::return_value_policy::move,
                "Returns a list of all non-depot nodes in the solution.")
            .def(
                "__getitem__",
                [](const Solution& solution, size_t pos) -> const vrpis::Route& {
                    return *std::next(solution.begin(), pos);
                },
                "The route at the given index.", pybind11::return_value_policy::reference_internal)
            .def(
                "lookup",
                [](const Solution& sol, const NodeLocation& location) -> const Node* {
                    return to_ref(location, sol).second;
                },
                pybind11::return_value_policy::reference_internal,
                "Get the node at the given "
                "location.")
            .def("find", &vrpis::Solution::find,
                 "Finds locations where the given vertex occurs in the solution.")
            .def(
                "exchange_segment",
                [](Solution& solution, size_t route_index, size_t begin_pos, size_t end_pos,
                   size_t other_route_index, size_t other_begin_pos, size_t other_end_pos) {
                    auto route = std::next(solution.begin(), route_index);
                    auto other = std::next(solution.begin(), other_route_index);
                    auto begin = std::next(route->begin(), begin_pos);
                    auto end = std::next(route->begin(), end_pos);
                    auto other_begin = std::next(other->begin(), other_begin_pos);
                    auto other_end = std::next(other->begin(), other_end_pos);
                    solution.exchange_segment(route, begin, end, other, other_begin, other_end);
                },
                "Exchanges the given segments between the given routes.")
            .def(
                "insert_vertex_after",
                [](Solution& solution, NodeLocation& location, VertexID vertex) {
                    auto route = std::next(solution.begin(), location.route);
                    auto inserted_pos = solution.insert_vertex_after(
                        route, std::next(route->begin(), location.position), vertex);
                    return std::distance(route->begin(), inserted_pos);
                },
                "Inserts the given vertex after the given position in the given route.")
            .def("insert_vertices_after",
                 [](Solution& sol, pybind11::iterable& vertex_ids_and_locations_list) {
                     std::vector<std::pair<VertexID, NodeLocation>> vertex_ids_and_locations;

                     for (const auto& vertex_id_and_location : vertex_ids_and_locations_list) {
                         vertex_ids_and_locations.push_back(
                             vertex_id_and_location.cast<std::pair<VertexID, NodeLocation>>());
                     }

                     sol.insert_vertices_after(vertex_ids_and_locations.begin(),
                                               vertex_ids_and_locations.end());
                 })
            .def(
                "remove_vertex",
                [](Solution& solution, const NodeLocation& location) {
                    auto [route, pos] = to_iter(location, solution);
                    solution.remove_vertex(route, pos);
                },
                "Removes the vertex at the given position in the given route.")
            .def(
                "remove_vertices",
                [](Solution& sol, const std::vector<NodeLocation>& vertices) {
                    sol.remove_vertices(vertices.begin(), vertices.end());
                },
                "Removes the given vertices from the route.")
            .def(
                "__delitem__",
                [](Solution& sol, size_t index) {
                    sol.remove_route(std::next(sol.begin(), index));
                },
                "Removes the route at the given index from the solution")
            .def(
                "remove_route",
                [](Solution& sol, const Route& route) {
                    auto route_iter = std::find(sol.begin(), sol.end(), route);
                    sol.remove_route(route_iter);
                },
                "Removes the route at the given index from the solution")
            .def(
                "add_route",
                [](Solution& sol, std::optional<const Route*> route = std::nullopt) {
                    if (route.has_value()) {
                        sol.add_route(**route);
                    } else {
                        sol.add_route();
                    }
                },
                pybind11::arg("route") = std::nullopt, "Adds an empty route to the solution.")
            .def("__str__", &::bindings::helpers::ostream_to_string<vrpis::Solution>)
            .def("__eq__", &vrpis::Solution::operator==, "Whether the solutions are equal.")
            .def("__ne__", &vrpis::Solution::operator!=, "Whether the solutions are not equal.");
    }

    class RouteSegment {
        // TODO
        friend vrpis::route_segment cast_route_segment(const RouteSegment&);
        const Route& route;
        size_t begin;
        size_t end;

      public:
        constexpr RouteSegment(const vrpis::Route& route, size_t begin, size_t end)
            : route(route), begin(begin), end(end){};

        /*operator vrpis::route_segment<vrpis::Route::const_iterator>() const {
            return vrpis::route_segment(std::next(route.begin(), begin),
                                        std::next(route.begin(), end));
        }*/
    };
}  // namespace vrpis::bindings

namespace {
    vrpis::route_segment cast_route_segment(const vrpis::bindings::RouteSegment& segment) {
        throw std::runtime_error("Not implemented!");
        /*return vrpis::route_segment(std::next(segment.route.begin(), segment.begin),
                                    std::next(segment.route.begin(), segment.end));*/
    }
}  // namespace

namespace vrpis::bindings {

    void bind_solution_functions(pybind11::module& m) {
        pybind11::class_<RouteSegment>(m, "RouteSegment")
            .def(pybind11::init<const Route&, size_t, size_t>());

        m.def("evaluate_insertion",
              [](Evaluation& evaluation, const Instance& instance, const Route& route,
                 int after_pos, std::variant<VertexID, const Vertex*, const Node*> node) -> cost_t {
                  auto after_iter = std::next(route.begin(), after_pos);

                  if (auto* v_id_ptr = std::get_if<VertexID>(&node)) {
                      return evaluate_insertion(evaluation, instance, route, after_iter,
                                                instance.getVertex(*v_id_ptr));
                  } else if (auto* vertex_ptr = std::get_if<const Vertex*>(&node)) {
                      return evaluate_insertion(evaluation, instance, route, after_iter,
                                                **vertex_ptr);
                  } else if (auto* node_ptr = std::get_if<const Node*>(&node)) {
                      return evaluate_insertion(evaluation, instance, route, after_iter,
                                                (*node_ptr)->vertex());
                  } else {
                      throw std::runtime_error("Invalid node type!");
                  }
              });

        m.def(
            "evaluate_splice",
            [](Evaluation& evaluation, const Instance& instance, const Route& route,
               size_t pred_index, size_t succ_index) -> cost_t {
                return concatenate(
                    evaluation, instance, route_segment{route.begin(), pred_index + 1},
                    route_segment{std::next(route.begin(), succ_index), route.end()});
            },
            "Compute the cost of the route resulting from concatenating the route segment ending "
            "at pred with the route segment starting at succ. Shorthand method for concatenate.");
    }
}  // namespace vrpis::bindings