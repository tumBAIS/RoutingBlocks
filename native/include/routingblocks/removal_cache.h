
#ifndef routingblocks_REMOVAL_CACHE_H
#define routingblocks_REMOVAL_CACHE_H

#include <routingblocks/Instance.h>
#include <routingblocks/Solution.h>
#include <routingblocks/evaluation.h>
#include <routingblocks/types.h>

#include <concepts>

namespace routingblocks::utility {

    struct removal_move {
        routingblocks::VertexID vertex_id{};
        routingblocks::NodeLocation node_location{0, 0};
        cost_t delta_cost{};

        auto operator<=>(const removal_move& other) const {
            return delta_cost <=> other.delta_cost;
        }

        bool operator==(const removal_move& other) const {
            return vertex_id == other.vertex_id && node_location == other.node_location;
        }
    };

    template <class Comp = std::less<removal_move>> class removal_cache {
      public:
        using move_t = removal_move;
        using iterator = std::vector<move_t>::iterator;
        using const_iterator = std::vector<move_t>::const_iterator;

      private:
        const routingblocks::Instance* _instance;
        routingblocks::Evaluation* _evaluation;

        // Cache structure. Holds all removal moves.
        std::vector<move_t> _cache;
        // Comparator
        Comp _comp;

        auto _overwrite_sequence_with_moves_from_route(iterator seq_begin,
                                                       const routingblocks::Route& route,
                                                       size_t route_index) {
            auto succ = route.begin();
            auto pred = succ++;
            auto cur = succ++;
            size_t pos = 1;
            auto route_cost = route.cost();
            for (; succ != route.end(); ++succ, ++pred, ++cur, ++pos, ++seq_begin) {
                cost_t removal_cost
                    = concatenate(*_evaluation, *_instance, route_segment{route.begin(), cur},
                                  route_segment{succ, route.end()});
                *seq_begin = move_t{cur->vertex_id(), routingblocks::NodeLocation(route_index, pos),
                                    removal_cost - route_cost};
            }
            return seq_begin;
        }

        void _update_moves_of_route(const routingblocks::Route& route, size_t route_index) {
            // Partitions the vector into [route_i, route_j, ..., route_index...]
            auto first_invalid_move
                = std::partition(_cache.begin(), _cache.end(), [route_index](const move_t& move) {
                      return move.node_location.route != route_index;
                  });
            const size_t first_invalid_move_index
                = std::distance(_cache.begin(), first_invalid_move);
            // Resize the cache to have space for all elements
            const long prev_route_size = (_cache.size() - first_invalid_move_index);
            // Could be negative if the route grew.
            const long route_shrank_by = prev_route_size - (route.size() - 2);
            assert(route_shrank_by <= static_cast<long long>(_cache.size()));
            _cache.resize(static_cast<size_t>(_cache.size() - route_shrank_by));
            // Iterator may have been invalidated
            first_invalid_move = std::next(_cache.begin(), first_invalid_move_index);
            // Now just replace
            auto cache_end
                = _overwrite_sequence_with_moves_from_route(first_invalid_move, route, route_index);
            assert(cache_end == _cache.end());
        }

        void _restore_order() { std::sort(_cache.begin(), _cache.end(), _comp); }

      public:
        explicit removal_cache(const routingblocks::Instance& instance)
            : _instance(&instance), _evaluation{}, _cache{} {};
        removal_cache(const routingblocks::Instance& instance, Comp comp)
            : _instance(&instance), _evaluation{}, _cache{}, _comp(std::move(comp)){};

        void clear() {
            _evaluation = nullptr;
            _cache.clear();
        };

        void rebuild(routingblocks::Evaluation& evaluation,
                     const routingblocks::Solution& solution) {
            clear();
            _evaluation = &evaluation;
            _cache.resize(routingblocks::number_of_nodes(solution));
            size_t route_index = 0;
            auto next_move_storage = _cache.begin();
            for (auto route_iter = solution.begin(); route_iter != solution.end();
                 ++route_index, ++route_iter) {
                next_move_storage = _overwrite_sequence_with_moves_from_route(
                    next_move_storage, *route_iter, route_index);
            }
            assert(next_move_storage == _cache.end());
            _restore_order();
        }

        void invalidate_route(const routingblocks::Route& route, size_t route_index) {
            _update_moves_of_route(route, route_index);
            _restore_order();
        }

        auto begin() const { return _cache.begin(); }
        auto end() const { return _cache.end(); }
    };

}  // namespace routingblocks::utility

#endif  // routingblocks_REMOVAL_CACHE_H
