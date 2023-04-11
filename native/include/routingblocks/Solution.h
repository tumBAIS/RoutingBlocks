
#ifndef routingblocks_SOLUTION_H
#define routingblocks_SOLUTION_H

#include <routingblocks/ADPTWEvaluation.h>
#include <routingblocks/evaluation.h>
#include <routingblocks/utility/algorithms.h>

#include <any>
#include <atomic>
#include <concepts>
#include <iterator>
#include <numeric>

namespace {
    template <class Iterator> constexpr bool has_efficient_size() {
        return std::is_same_v<typename std::iterator_traits<Iterator>::iterator_category,
                              std::random_access_iterator_tag>;
    }

    template <class Iterator> constexpr Iterator proper_end_iterator(auto&& container) {
        if constexpr (std::is_same_v<Iterator, decltype(container.end())>) {
            return container.end();
        } else {
            return container.rend();
        }
    }
}  // namespace

namespace routingblocks {
    class NodeLocation;

    inline Node create_node(Evaluation& evaluation, const Vertex& vertex) {
        return {vertex, evaluation.create_forward_label(vertex),
                evaluation.create_backward_label(vertex)};
    }

    inline Node create_node(Evaluation& evaluation, const Instance& instance, VertexID vertex) {
        return create_node(evaluation, instance.getVertex(vertex));
    }

    template <class InputIterator>
    concept NodeIterator = requires(InputIterator iter) {
                               { Node{*iter} } -> std::same_as<typename InputIterator::value_type>;
                           };

    template <class InputIterator>
    concept VertexIDIterator
        = std::is_same_v<std::decay_t<typename std::iterator_traits<InputIterator>::value_type>,
                         std::decay_t<VertexID>>;

    template <class InputIterator>
    concept NodeLocationIterator
        = std::is_same_v<std::decay_t<typename std::iterator_traits<InputIterator>::value_type>,
                         std::decay_t<NodeLocation>>;

    class Route {
      public:
        using eval_t = routingblocks::Evaluation;
        using node_t = Node;

      private:
        using node_container_t = std::vector<node_t>;
        using instance_t = Instance;

      public:
        using iterator = node_container_t::iterator;
        using const_iterator = node_container_t::const_iterator;
        using reverse_iterator = node_container_t::reverse_iterator;
        using const_reverse_iterator = node_container_t::const_reverse_iterator;

      private:
        const instance_t* _instance;
        std::shared_ptr<eval_t> _evaluation;
        node_container_t _nodes;
        size_t _modification_timestamp;
        static inline std::atomic<size_t> _next_modification_timestamp = 1;

        // removal of segments. Does not update
        iterator _remove_segment(const_iterator begin, const_iterator end) {
            assert(begin != this->begin());
            assert(end != this->end());
            return _nodes.erase(begin, end);
        }

        // insertion of segments. Does not update
        template <class InputIterator>
            requires NodeIterator<InputIterator>
        iterator _insert_segment_after(const_iterator pos, InputIterator begin, InputIterator end) {
            assert(pos != this->end_depot());
            return _nodes.insert(std::next(pos), begin, end);
        }

        template <class InputIterator>
        void _remove_vertices(InputIterator begin, InputIterator end) {
            if (begin == end) return;
            // TODO
            //  A better implementation could base on std::remove, begin->end is sorted in
            //  reverse order.
            //  Sorting in "normal" order would allow efficient partition
            for (; begin != end; ++begin) {
                _nodes.erase(std::next(
                    this->begin(),
                    begin->position));  // Will not invalidate any iterators that come after begin
            }
            update();
        }

        template <class input_iterator_t>
        void _insert_vertices(input_iterator_t locations_begin, input_iterator_t locations_end) {
            if (locations_begin == locations_end) return;
            // The function assumes that [begin, end) is sorted in reverse order, i.e., largest
            // position first
            for (; locations_begin != locations_end; ++locations_begin) {
                auto& [vertex_id, location] = *locations_begin;
                assert(location.position < _nodes.size() - 1);
                _nodes.insert(std::next(this->begin(), location.position + 1),
                              create_node(*_evaluation, *_instance, vertex_id));
            }
            update();
        }

      public:
        // Cost
        [[nodiscard]] cost_t cost() const { return _nodes.back().cost(*_evaluation); }
        [[nodiscard]] std::vector<resource_t> cost_components() const {
            return _nodes.back().cost_components(*_evaluation);
        }
        [[nodiscard]] bool feasible() const { return _nodes.back().feasible(*_evaluation); }
        [[nodiscard]] node_container_t::size_type size() const noexcept { return _nodes.size(); }
        [[nodiscard]] bool empty() const noexcept { return _nodes.size() == 2; }
        [[nodiscard]] auto modification_timestamp() const noexcept {
            return _modification_timestamp;
        }

        /**
         * Sets the evaluation function to be used for this route.
         * @param evaluation A shared pointer to the evaluation function.
         */
        void set_evaluation(std::shared_ptr<eval_t> evaluation) {
            _evaluation = std::move(evaluation);
        }

        // Construction
        Route(std::shared_ptr<eval_t> evaluation, const Instance& instance)
            : _instance(&instance), _evaluation(std::move(evaluation)) {
            _nodes.reserve(2);
            // Create properly initialized depot nodes
            const auto& depot = _instance->Depot();
            _nodes.emplace_back(depot, _evaluation->create_forward_label(depot),
                                _evaluation->create_backward_label(depot));
            _nodes.emplace_back(depot, _evaluation->create_forward_label(depot),
                                _evaluation->create_backward_label(depot));
            update();
            _modification_timestamp = 0;  // Empty routes always get an id of 0
        }

        template <class InputIterator>
            requires VertexIDIterator<InputIterator>
        explicit Route(std::shared_ptr<eval_t> evaluation, const Instance& instance,
                       InputIterator begin, InputIterator end)
            : _instance(&instance), _evaluation(std::move(evaluation)) {
#ifndef NDEBUG
            if (begin != end) {
                assert(*begin != _instance->Depot().id);
                assert(*std::prev(end) != _instance->Depot().id);
            }
#endif
            if constexpr (has_efficient_size<InputIterator>()) {
                _nodes.reserve(std::distance(begin, end) + 2);
            }

            const auto& depot = instance.Depot();

            // Start depot
            _nodes.emplace_back(depot, _evaluation->create_forward_label(depot),
                                _evaluation->create_backward_label(depot));
            // Route
            std::transform(begin, end, std::back_inserter(_nodes), [this](VertexID vertex) {
                return node_t(_instance->getVertex(vertex),
                              _evaluation->create_forward_label(_instance->getVertex(vertex)),
                              _evaluation->create_backward_label(_instance->getVertex(vertex)));
            });
            // End depot
            _nodes.emplace_back(depot, _evaluation->create_forward_label(depot),
                                _evaluation->create_backward_label(depot));
            // Update labels/cost
            update();
        }

        // Iteration
        [[nodiscard]] iterator begin() { return _nodes.begin(); };
        [[nodiscard]] const_iterator begin() const { return _nodes.begin(); };
        [[nodiscard]] iterator end() { return _nodes.end(); };
        [[nodiscard]] const_iterator end() const { return _nodes.end(); };

        [[nodiscard]] iterator end_depot() { return std::prev(_nodes.end()); };
        [[nodiscard]] const_iterator end_depot() const { return std::prev(_nodes.end()); };

        [[nodiscard]] reverse_iterator rbegin() { return _nodes.rbegin(); };
        [[nodiscard]] const_reverse_iterator rbegin() const { return _nodes.rbegin(); };
        [[nodiscard]] reverse_iterator rend() { return _nodes.rend(); };
        [[nodiscard]] const_reverse_iterator rend() const { return _nodes.rend(); };

        iterator remove_segment(const_iterator begin, const_iterator end) {
            auto past_erase = _remove_segment(begin, end);
            update();  // TODO Update only modified segment
            return past_erase;
        }

        template <class InputIterator>
            requires NodeLocationIterator<InputIterator>
        void remove_vertices(InputIterator locations_begin, InputIterator locations_end) {
            if (std::is_sorted(locations_begin, locations_end, std::greater<>())) {
                _remove_vertices(locations_begin, locations_end);
            } else {
                std::vector<typename InputIterator::value_type> locations(locations_begin,
                                                                          locations_end);
                std::sort(locations.begin(), locations.end(), std::greater<>());
                _remove_vertices(locations.begin(), locations.end());
            }
        }

        /**
         * Inserts several vertices into the route at the given locations. Assumes unique locations,
         * otherwise behavior is undefined,
         * @tparam input_iterator_t
         * @tparam vertex_iterator_t
         * @param locations_begin
         * @param locations_end
         * @param factors
         */
        template <class input_iterator_t>
            requires std::is_same_v<std::remove_cvref_t<typename input_iterator_t::value_type>,
                                    std::pair<VertexID, NodeLocation>>
        void insert_vertices_after(input_iterator_t locations_begin,
                                   input_iterator_t locations_end) {
            auto comp = [](const auto& lhs, const auto& rhs) { return lhs.second > rhs.second; };
            if (std::is_sorted(locations_begin, locations_end, comp)) {
                _insert_vertices(locations_begin, locations_end);
            } else {
                std::vector<typename input_iterator_t::value_type> locations(locations_begin,
                                                                             locations_end);
                std::sort(locations.begin(), locations.end(), comp);
                _insert_vertices(locations.begin(), locations.end());
            }
        }

        // insertion of segments
        template <class InputIterator>
            requires NodeIterator<InputIterator>
        iterator insert_segment_after(const_iterator pos, InputIterator begin, InputIterator end) {
            auto first_inserted_element = _insert_segment_after(pos, begin, end);
            update();  // TODO Update only modified segment
            return first_inserted_element;
        }

        void exchange_segments(iterator begin, iterator end, iterator other_begin,
                               iterator other_end, Route& other) {
            assert(this != &other);
            assert(end != this->end());
            assert(other_end != other.end());

            static_assert(has_efficient_size<iterator>());
            if (std::distance(begin, end) > std::distance(other_begin, other_end)) {
                return other.exchange_segments(other_begin, other_end, begin, end, *this);
            }
            // [begin, end) is the shortest of both ranges
            auto other_first_unchanged = std::swap_ranges(begin, end, other_begin);
            // Insert [other_first_unchanged, other_end) before end
            _insert_segment_after(std::prev(end), other_first_unchanged, other_end);
            // Remove [other_first_unchanged, other_end) from other
            other._remove_segment(other_first_unchanged, other_end);
            // Finally update both routes
            update();
            other.update();
        }

        void exchange_segments(iterator begin, iterator end, iterator other_begin,
                               iterator other_end) {
            // Ensure no overlap between segments
            assert(begin >= other_end || other_end >= begin);
            static_assert(has_efficient_size<iterator>());
            if (std::distance(begin, end) > std::distance(other_begin, other_end)) {
                return exchange_segments(other_begin, other_end, begin, end);
            }
            // We cannot rely on swap_ranges here because the ranges might overlap
            // Hand rolled implementation of iter swap.
            for (; begin != end; ++begin, ++other_begin) {
                std::iter_swap(begin, other_begin);
            }
            assert(begin == end);
            // All that remains is to insert the remaining elements [other_begin, other_end) before
            // "end"
            if (other_end < end) {
                // Move [other_begin, other_end) backwards, i.e., *increase* the position.
                std::rotate(other_begin, other_end, end);
            } else {
                // Move [other_begin, other_end) forwards, i.e., *decrease* the position.
                std::rotate(end, other_begin, other_end);
            }
            // Finally update the route
            update();
        }

        // Update nodes
        void update() { update(begin(), end_depot()); }
        void update(iterator last_valid_forward, iterator first_valid_backward) {
            auto next_forward = last_valid_forward;
            for (++next_forward; next_forward != end(); last_valid_forward = next_forward++) {
                next_forward->update_forward(
                    *_evaluation, *last_valid_forward,
                    _instance->getArc(last_valid_forward->vertex_id(), next_forward->vertex_id()));
            }

            auto next_backward = first_valid_backward;
            for (--next_backward; first_valid_backward != begin();
                 first_valid_backward = next_backward--) {
                next_backward->update_backward(*_evaluation, *first_valid_backward,
                                               _instance->getArc(first_valid_backward->vertex_id(),
                                                                 next_backward->vertex_id()));
            }
            _modification_timestamp = _next_modification_timestamp++;
        }

        [[nodiscard]] bool operator==(const Route& other) const { return _nodes == other._nodes; }
        [[nodiscard]] bool operator!=(const Route& other) const { return !(other == *this); }

        friend std::ostream& operator<<(std::ostream& out, const Route& route) {
            std::ostream_iterator<node_t> iter(out, ",");
            out << "[";
            std::copy(route.begin(), route.end(), iter);
            out << "]";
            return out;
        }
    };

    template <class InputIterator>
    concept RouteIterator = requires(InputIterator iter) {
                                {
                                    Route{*iter}
                                } -> std::same_as<typename InputIterator::value_type>;
                            };

    inline Route create_route_from_vector(std::shared_ptr<Evaluation> evaluation,
                                          const Instance& instance,
                                          const std::vector<VertexID>& vertices) {
        return Route(std::move(evaluation), instance, vertices.begin(), vertices.end());
    }

    class NodeLocation {
      public:
        size_t route;
        size_t position;

        NodeLocation(size_t route, size_t position) : route(route), position(position) {}

        bool operator==(const NodeLocation& rhs) const;
        bool operator!=(const NodeLocation& rhs) const;

        friend std::ostream& operator<<(std::ostream& out, const NodeLocation& location) {
            out << "(" << location.route << "," << location.position << ")";
            return out;
        }

        [[nodiscard]] bool operator<(const NodeLocation& other) const noexcept {
            if (route == other.route) return position < other.position;
            return route < other.route;
        }

        [[nodiscard]] bool operator>(const NodeLocation& other) const noexcept {
            if (route == other.route) return position > other.position;
            return route > other.route;
        }
    };

    class Solution {
      public:
        using eval_t = Evaluation;
        using route_t = Route;

      private:
        using route_container_t = std::vector<route_t>;

      public:
        using iterator = route_container_t::iterator;
        using const_iterator = route_container_t::const_iterator;

      private:
        route_container_t _routes;
        std::vector<std::vector<NodeLocation>> _vertex_lookup;
        const Instance* _instance;
        std::shared_ptr<eval_t> _evaluation;

        void _update_vertex_lookup(unsigned int route_index);

        void _update_vertex_lookup();

        template <class input_iterator>
        void _remove_vertices(input_iterator begin, input_iterator end) {
            if (begin == end) return;
            auto last_route_pos_begin = begin;
            while (last_route_pos_begin != end) {
                auto last_route_pos_end = std::find_if(
                    last_route_pos_begin, end, [last_route_pos_begin](const auto& location) {
                        return location.route != last_route_pos_begin->route;
                    });
                auto next_route = std::next(this->begin(), last_route_pos_begin->route);
                next_route->remove_vertices(last_route_pos_begin, last_route_pos_end);
                last_route_pos_begin = last_route_pos_end;
            }
            _update_vertex_lookup();
        }

        template <class input_iterator_t>
        void _insert_vertices_after(input_iterator_t locations_begin,
                                    input_iterator_t locations_end) {
            if (locations_begin == locations_end) return;
            // The function assumes that [begin, end) is sorted in reverse order, i.e., largest
            // position first
            auto last_route_pos_begin = locations_begin;
            while (last_route_pos_begin != locations_end) {
                auto last_route_pos_end
                    = std::find_if(last_route_pos_begin, locations_end,
                                   [last_route_pos_begin](const auto& vertex_and_location) {
                                       return vertex_and_location.second.route
                                              != last_route_pos_begin->second.route;
                                   });
                auto next_route = std::next(this->begin(), last_route_pos_begin->second.route);
                next_route->insert_vertices_after(last_route_pos_begin, last_route_pos_end);
                last_route_pos_begin = last_route_pos_end;
            }
            _update_vertex_lookup();
        }

      public:
        Solution(std::shared_ptr<Evaluation> evaluation, const Instance& instance,
                 size_t num_routes)
            : _routes(num_routes, Route(evaluation, instance)),
              _vertex_lookup(instance.NumberOfVertices()),
              _instance(&instance),
              _evaluation(std::move(evaluation)) {
            assert(_evaluation);
            _update_vertex_lookup();
        };

        Solution(std::shared_ptr<Evaluation> evaluation, const Instance& instance,
                 std::vector<route_t> routes)
            : _routes(std::move(routes)),
              _vertex_lookup(instance.NumberOfVertices()),
              _instance(&instance),
              _evaluation(std::move(evaluation)) {
            _update_vertex_lookup();
        };

        [[nodiscard]] const std::vector<NodeLocation>& find(VertexID vertex_id) const {
            return _vertex_lookup[vertex_id];
        }

        [[nodiscard]] cost_t cost() const {
            return std::accumulate(
                _routes.begin(), _routes.end(), cost_t(0.0),
                [](cost_t acc, const route_t& route) { return acc + route.cost(); });
        }

        [[nodiscard]] std::vector<cost_t> cost_components() const {
            if (_routes.empty()) return {};
            auto next_route = begin();
            std::vector<cost_t> result = next_route->cost_components();
            for (; next_route != end(); ++next_route) {
                const auto& next_cost_components = next_route->cost_components();
                for (size_t i = 0; i < next_cost_components.size(); ++i) {
                    result[i] += next_cost_components[i];
                }
            }
            return result;
        }

        [[nodiscard]] bool feasible() const {
            return std::all_of(_routes.begin(), _routes.end(),
                               [](const route_t& route) { return route.feasible(); });
        }

        [[nodiscard]] Route& operator[](int i) { return _routes[i]; }

        [[nodiscard]] const Route& operator[](int i) const { return _routes[i]; }

        // Iteration
        [[nodiscard]] iterator begin() { return _routes.begin(); };
        [[nodiscard]] const_iterator begin() const { return _routes.begin(); };
        [[nodiscard]] iterator end() { return _routes.end(); };
        [[nodiscard]] const_iterator end() const { return _routes.end(); };

        [[nodiscard]] const_iterator cbegin() const { return _routes.begin(); };
        [[nodiscard]] const_iterator cend() const { return _routes.end(); };

        // Exchange segment
        void exchange_segment(iterator from_route, route_t::iterator from_route_segment_begin,
                              route_t::iterator from_route_segment_end, iterator to_route,
                              route_t::iterator to_route_segment_begin,
                              route_t::iterator to_route_segment_end);

        route_t::iterator insert_vertex_after(iterator route, route_t::iterator pos,
                                              VertexID vertex_id);

        route_t::iterator remove_route_segment(iterator route, route_t::iterator begin,
                                               route_t::iterator end);

        route_t::iterator remove_vertex(iterator route, route_t::iterator position);

        friend std::ostream& operator<<(std::ostream& stream, const Solution& solution) {
            stream << "Solution(cost=" << solution.cost() << ", routes=[";
            for (const auto& route : solution) {
                stream << route << std::endl;
            }
            stream << "])";
            return stream;
        }

        [[nodiscard]] route_container_t::size_type size() const { return _routes.size(); }

        [[nodiscard]] bool operator==(const Solution& rhs) const { return _routes == rhs._routes; }

        [[nodiscard]] bool operator!=(const Solution& rhs) const { return !(rhs == *this); }

        template <class input_iterator>
            requires NodeLocationIterator<input_iterator>
        void remove_vertices(input_iterator positions_begin, input_iterator positions_end) {
            if (std::is_sorted(positions_begin, positions_end, std::greater<>())) {
                _remove_vertices(positions_begin, positions_end);
            } else {
                std::vector<NodeLocation> locations(positions_begin, positions_end);
                std::sort(locations.begin(), locations.end(), std::greater<>());
                _remove_vertices(locations.begin(), locations.end());
            }
        }

        template <class input_iterator_t>
            requires std::is_same_v<std::remove_cvref_t<typename input_iterator_t::value_type>,
                                    std::pair<VertexID, NodeLocation>>
        void insert_vertices_after(input_iterator_t locations_begin,
                                   input_iterator_t locations_end) {
            auto comp = [](const auto& lhs, const auto& rhs) { return lhs.second > rhs.second; };
            if (std::is_sorted(locations_begin, locations_end, comp)) {
                _insert_vertices_after(locations_begin, locations_end);
            } else {
                std::vector<typename input_iterator_t::value_type> locations(locations_begin,
                                                                             locations_end);
                std::sort(locations.begin(), locations.end(), comp);
                _insert_vertices_after(locations.begin(), locations.end());
            }
        }

        void remove_route(const_iterator route) {
            _routes.erase(route);
            _update_vertex_lookup();
        }

        const_iterator add_route() {
            _routes.emplace_back(_evaluation, *_instance);
            _update_vertex_lookup(_routes.size() - 1);
            return std::prev(_routes.end());
        }

        const_iterator add_route(Route route) {
            _routes.push_back(std::move(route));
            _update_vertex_lookup(_routes.size() - 1);
            return std::prev(_routes.end());
        }
    };

    inline auto to_iter(const NodeLocation& location, const Solution& sol) {
        auto route_iter = std::next(sol.begin(), location.route);
        return std::make_pair(route_iter, std::next(route_iter->begin(), location.position));
    }

    inline auto to_iter(const NodeLocation& location, Solution& sol) {
        auto route_iter = std::next(sol.begin(), location.route);
        return std::make_pair(route_iter, std::next(route_iter->begin(), location.position));
    }

    inline auto to_ref(const NodeLocation& location, Solution& sol) {
        auto [route_iter, node_iter] = to_iter(location, sol);
        return std::make_pair(&*route_iter, &*node_iter);
    }

    inline auto to_ref(const NodeLocation& location, const Solution& sol) {
        auto [route_iter, node_iter] = to_iter(location, sol);
        return std::make_pair(&*route_iter, &*node_iter);
    }

    template <class route_iterator_t, class node_iterator_t>
        requires NodeIterator<node_iterator_t> and RouteIterator<route_iterator_t>
    NodeLocation location_cast(const Solution& sol, route_iterator_t r, node_iterator_t n) {
        return NodeLocation(std::distance(sol.begin(), r), std::distance(r->begin(), n));
    }

    template <std::same_as<route_segment>... Segments>
    cost_t concatenate(Evaluation& evaluation, const Instance& instance, Segments&&... params) {
        // TODO Is it safe to assume that segments are pushed on the stack? If so, we could
        //  avoid creating the array.
        const std::array<const route_segment, sizeof...(params)> storage{
            std::forward<Segments>(params)...};
        return evaluation.evaluate(instance, storage);
    }

    template <class node_iterator_t>
        requires NodeIterator<node_iterator_t>
    inline cost_t evaluate_insertion(Evaluation& evaluation, const Instance& instance,
                                     const Route& route, node_iterator_t after,
                                     const Vertex& vertex) {
        Node n = create_node(evaluation, vertex);
        return concatenate(evaluation, instance, route_segment{route.begin(), std::next(after)},
                           singleton_route_segment(n),
                           route_segment{std::next(after), route.end()});
    }

    template <class node_iterator_t>
        requires NodeIterator<node_iterator_t>
    inline cost_t evaluate_insertion(Evaluation& evaluation, const Instance& instance,
                                     const Route& route, node_iterator_t after, const Node& n) {
        return concatenate(evaluation, instance, route_segment{route.begin(), std::next(after)},
                           singleton_route_segment(n),
                           route_segment{std::next(after), route.end()});
    }

    inline size_t number_of_nodes(const Route& route, bool include_start_depot = false) {
        return route.size() - (2 - include_start_depot);
    }

    inline size_t number_of_nodes(const Solution& solution, bool include_start_depot = false) {
        return std::accumulate(solution.begin(), solution.end(), 0,
                               [include_start_depot](size_t acc, const auto& route) {
                                   return acc + number_of_nodes(route, include_start_depot);
                               });
    }
}  // namespace routingblocks
#endif  // routingblocks_SOLUTION_H
