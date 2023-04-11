
#ifndef routingblocks_INSERTION_CACHE_H
#define routingblocks_INSERTION_CACHE_H

#include <routingblocks/Instance.h>
#include <routingblocks/Solution.h>
#include <routingblocks/evaluation.h>
#include <routingblocks/types.h>

#include <dynamic_bitset/dynamic_bitset.hpp>
#include <vector>

namespace routingblocks::utility {
    struct insertion_move {
        routingblocks::VertexID vertex_id{};
        routingblocks::NodeLocation after_node{0, 0};
        cost_t delta_cost{};

        auto operator<=>(const insertion_move& other) const {
            return delta_cost <=> other.delta_cost;
        }

        bool operator==(const insertion_move& other) const {
            return vertex_id == other.vertex_id && after_node == other.after_node;
        }
    };

    template <class bitset_t> class bitset_iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = size_t;
        using difference_type = std::ptrdiff_t;

      private:
        const bitset_t* _bitset;
        size_t _index;

        void _advance() { _index = _bitset->find_next(_index); }

      public:
        bitset_iterator() : _index(bitset_t::npos){};
        explicit bitset_iterator(const bitset_t& bitset)
            : _bitset(&bitset), _index(_bitset->find_first()) {}
        bitset_iterator(const bitset_t& bitset, size_t index) : _bitset(&bitset), _index(index) {}

        bitset_iterator& operator++() {
            _advance();
            return *this;
        }

        bitset_iterator operator++(int) {
            auto tmp = *this;
            _advance();
            return tmp;
        }

        size_t operator*() const { return _index; }

        size_t operator->() const { return _index; }

        bool operator==(const bitset_iterator& other) const { return _index == other._index; }

        bool operator!=(const bitset_iterator& other) const { return !(*this == other); }
    };

    template <class child_iterator, class Comp = std::less<typename child_iterator::value_type>>
    class joint_sorted_iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = typename child_iterator::value_type;
        using pointer = typename child_iterator::pointer;
        using difference_type = std::ptrdiff_t;
        using reference = typename child_iterator::reference;

      private:
        std::vector<std::pair<child_iterator, child_iterator>> _child_iterators;
        child_iterator _next_cheapest;
        const Comp* _comp;

        void _remove_exhausted() {
            while (!_child_iterators.empty()
                   && _child_iterators.back().first != _child_iterators.back().second) {
                _child_iterators.pop_back();
            }
        }

        void _advance() {
            ++_next_cheapest;
            _remove_exhausted();
            if (_child_iterators.empty()) return;
            _set_cheapest();
        }

        void _set_cheapest() {
            _next_cheapest = _child_iterators.back().first;
            for (auto [child_begin, child_end] : _child_iterators) {
                if (child_begin == child_end) continue;
                if ((*_comp)(*child_begin, *_next_cheapest)) {
                    _next_cheapest = child_begin;
                }
            }
        }

      public:
        joint_sorted_iterator(
            std::vector<std::pair<child_iterator, child_iterator>> child_iterators,
            const Comp* comp)
            : _child_iterators(std::move(child_iterators)), _comp(comp) {
            _remove_exhausted();
            if (!_child_iterators.empty()) {
                _set_cheapest();
            }
        };

        joint_sorted_iterator() : _child_iterators(0), _comp(nullptr){};

        joint_sorted_iterator& operator++() {
            _advance();
            return *this;
        }

        joint_sorted_iterator operator++(int) {
            joint_sorted_iterator tmp = *this;
            _advance();
            return tmp;
        }

        reference operator*() const { return *_next_cheapest; }

        pointer operator->() const { return &*_next_cheapest; }

        bool operator==(const joint_sorted_iterator& other) const {
            if (_child_iterators.empty() && other._child_iterators.empty()) {
                return true;
            } else if (_child_iterators.empty() || other._child_iterators.empty()) {
                return false;
            } else {
                return _next_cheapest == other._next_cheapest;
            }
        }

        bool operator!=(const joint_sorted_iterator& other) const { return !(*this == other); }
    };

    template <class Comp = std::less<insertion_move>> class insertion_cache {
      public:
        using move_t = insertion_move;

      private:
        using bitset_t = sul::dynamic_bitset<>;
        using cache_t = std::vector<move_t>;
        using const_iterator = joint_sorted_iterator<typename cache_t::const_iterator, Comp>;
        using tracked_vertex_iterator = bitset_iterator<bitset_t>;

      private:
        const Instance* _instance;
        Evaluation* _evaluation{};
        // Move caches, _caches[i] contains all possible insertion moves for vertex with id i.
        std::vector<cache_t> _caches;
        // Vertices to track insertions of. Setting bit at index i indicates that moves for
        // vertex with id i should be tracked.
        bitset_t _tracked_vertices;

        Comp _comp;

      public:
        explicit insertion_cache(const Instance& instance)
            : _instance(&instance),
              _caches(_instance->NumberOfVertices(), std::vector<move_t>(0)),
              _tracked_vertices(_instance->NumberOfVertices()),
              _comp(){};

        explicit insertion_cache(const Instance& instance, Comp comp)
            : _instance(&instance),
              _caches(_instance->NumberOfVertices(), std::vector<move_t>(0)),
              _tracked_vertices(_instance->NumberOfVertices()),
              _comp(std::move(comp)){};

        void clear() {
            _tracked_vertices.reset();
            std::for_each(_caches.begin(), _caches.end(), [](auto& cache) { cache.clear(); });
            _evaluation = nullptr;
        };

        template <class InputIterator>
            requires VertexIDIterator<InputIterator>
        void rebuild(Evaluation& evaluation, const Solution& solution,
                     InputIterator tracked_vertices_begin, InputIterator tracked_vertices_end) {
            clear();
            _evaluation = &evaluation;
            auto number_of_insertion_positions = number_of_nodes(solution, true);
            // Build the initial cache
            for (; tracked_vertices_begin != tracked_vertices_end; ++tracked_vertices_begin) {
                _caches[*tracked_vertices_begin].resize(number_of_insertion_positions);
                size_t route_index = 0;
                for (auto route = solution.begin(); route != solution.end();
                     ++route, ++route_index) {
                    _update_moves_of_route(*route, route_index, _caches[*tracked_vertices_begin],
                                           *tracked_vertices_begin);
                }
                _restore_order(*tracked_vertices_begin);
                _tracked_vertices.set(*tracked_vertices_begin);
            }
        }

        void invalidate_route(const Route& route, size_t route_index) {
            // Update moves involving the invalidated route for each tracked vertex
            _tracked_vertices.iterate_bits_on([&](VertexID vertex_id) {
                _update_moves_of_route(route, route_index, _caches[vertex_id], vertex_id);
                _restore_order(vertex_id);
            });
        }

        void stop_tracking(VertexID vertex_id) { _tracked_vertices.reset(vertex_id); }

        [[nodiscard]] auto best_insertions_for_vertex_begin(VertexID vertex_id) const {
            assert(_tracked_vertices.test(vertex_id));
            assert(!_caches[vertex_id].empty());
            return _caches[vertex_id].begin();
        }

        [[nodiscard]] auto best_insertions_for_vertex_end(VertexID vertex_id) const {
            assert(_tracked_vertices.test(vertex_id));
            assert(!_caches[vertex_id].empty());
            return _caches[vertex_id].end();
        }

        [[nodiscard]] tracked_vertex_iterator tracked_vertices_begin() const {
            return tracked_vertex_iterator(_tracked_vertices);
        }
        [[nodiscard]] tracked_vertex_iterator tracked_vertices_end() const { return {}; }

        [[nodiscard]] const_iterator begin() const {
            std::vector<std::pair<cache_t::const_iterator, cache_t::const_iterator>> iters;
            iters.reserve(_tracked_vertices.count());
            for (const auto& cache : _caches) {
                if (cache.empty()) continue;
                iters.push_back(std::make_pair(cache.begin(), cache.end()));
            }
            return {std::move(iters), &_comp};
        };
        [[nodiscard]] const_iterator end() const { return {}; };

        bool tracks(VertexID vertex_id) const { return _tracked_vertices.test(vertex_id); }

      private:
        void _restore_order(VertexID vertex) {
            std::sort(_caches[vertex].begin(), _caches[vertex].end(), _comp);
        }

        auto _overwrite_sequence_with_moves_from_route(std::vector<move_t>::iterator seq_begin,
                                                       const routingblocks::Route& route,
                                                       size_t route_index, VertexID vertex_id) {
            auto succ = route.begin();
            auto pred = succ++;
            size_t pos = 0;
            auto route_cost = route.cost();
            Node n = create_node(*_evaluation, *_instance, vertex_id);
            for (; succ != route.end(); ++succ, ++pred, ++pos, ++seq_begin) {
                cost_t insertion_cost
                    = evaluate_insertion(*_evaluation, *_instance, route, pred, n);
                *seq_begin = move_t{vertex_id, routingblocks::NodeLocation(route_index, pos),
                                    insertion_cost - route_cost};
            }
            return seq_begin;
        }

        void _update_moves_of_route(const routingblocks::Route& route, size_t route_index,
                                    auto& cache, VertexID vertex_id) {
            // Partitions the vector into [route_i, route_j, ..., route_index...]
            auto first_invalid_move = std::partition(
                cache.begin(), cache.end(),
                [route_index](const move_t& move) { return move.after_node.route != route_index; });
            const size_t first_invalid_move_index
                = std::distance(cache.begin(), first_invalid_move);
            // Resize the cache to have space for all elements
            const long prev_route_size = (cache.size() - first_invalid_move_index);
            // Could be negative if the route grew.
            const long route_shrank_by = prev_route_size - (route.size() - 1);
            assert(route_shrank_by <= static_cast<long long>(cache.size()));
            cache.resize(static_cast<size_t>(cache.size() - route_shrank_by));
            // Iterator may have been invalidated
            first_invalid_move = std::next(cache.begin(), first_invalid_move_index);
            // Now just replace
            auto cache_end = _overwrite_sequence_with_moves_from_route(first_invalid_move, route,
                                                                       route_index, vertex_id);
            assert(cache_end == cache.end());
        }
    };
}  // namespace routingblocks::utility

#endif  // routingblocks_INSERTION_CACHE_H
