
#ifndef routingblocks_FRVCP_H
#define routingblocks_FRVCP_H

#include <routingblocks/Instance.h>
#include <routingblocks/utility/heap.h>

#include <algorithm>
#include <bitset>
#include <deque>
#include <iostream>
namespace routingblocks {

    using DPVertexID = size_t;

    class DPVertex {
        DPVertexID _vertex_id;
        const Vertex* _original_vertex;

      public:
        DPVertex(DPVertexID vertexId, const Vertex* originalVertex)
            : _vertex_id(vertexId), _original_vertex(originalVertex) {}

        DPVertexID dp_vertex_id() const { return _vertex_id; };
        const Vertex& original_vertex() const { return *_original_vertex; };

        friend std::ostream& operator<<(std::ostream& out, const DPVertex& v) {
            out << v._vertex_id << " (" << v._original_vertex->str_id << ")";
            return out;
        }
    };

    class DPGraph {
        std::deque<DPVertex> _vertices;
        std::vector<std::vector<DPVertex*>> _successors;

      public:
        [[nodiscard]] size_t size() const { return _vertices.size(); };
        [[nodiscard]] auto get_successors(DPVertexID of) const {
            return std::make_pair(_successors[of].begin(), _successors[of].end());
        }

        [[nodiscard]] const DPVertex& get_vertex(DPVertexID id) const { return _vertices[id]; }

        void clear() {
            _vertices.clear();
            for (auto& successor_list : _successors) {
                successor_list.clear();
            }
        }

        DPVertexID add_vertex(const Vertex& vertex) {
            DPVertexID next_id = _vertices.size();
            _vertices.emplace_back(next_id, &vertex);
            _successors.emplace_back();
            return next_id;
        };
        void add_edge(DPVertexID i, DPVertexID j) { _successors[i].push_back(&_vertices[j]); }
    };

    template <class Label> class Propagator {
      public:
        using label_t = Label;
    };

    template <class T, class Comparator = std::less<T>> class NodeQueue {
        Comparator _compare;

        using container_t = std::vector<T>;
        container_t _container;

      public:
        NodeQueue() = default;
        explicit NodeQueue(Comparator comp) : _compare(std::move(comp)){};

        [[nodiscard]] bool empty() const { return _container.empty(); }

        T extract_cheapest() {
#ifndef NDEBUG
            if (empty()) {
                throw std::runtime_error("Cannot extract from empty container!");
            }
#endif
            auto min_elem = std::min_element(_container.begin(), _container.end(), _compare);

            std::iter_swap(min_elem, std::prev(_container.end()));
            T elem = std::move(_container.back());
            _container.pop_back();

            return elem;
        }

        void update(const T& elem) {
            auto pos = std::find(_container.begin(), _container.end(), elem);
            if (pos == _container.end()) {
                insert(elem);
            }
        }

        void insert(T elem) {
#ifndef NDEBUG
            if (std::find(_container.begin(), _container.end(), elem) != _container.end()) {
                throw std::runtime_error(
                    "Cannot insert into NodeQueue. Element already occurs in node queue!");
            }
#endif
            _container.push_back(std::move(elem));
        }

        void clear() { _container.clear(); }
    };

    template <class Label> class LabelBucket {
        using label_ref_t = const Label*;
        using propagator_t = Propagator<Label>;
        // Essentially implements operator>
        struct _deref_ptrs_comp {
            propagator_t* _propagator;

          public:
            explicit _deref_ptrs_comp(propagator_t& propagator) : _propagator(&propagator) {}
            bool operator()(const label_ref_t& lhs, const label_ref_t& rhs) const {
                // Max heap, reverse comparison!
                return _propagator->cheaper_than(*rhs, *lhs);
            }
        };

        propagator_t* _propagator;
        _deref_ptrs_comp _comp;
        std::vector<label_ref_t> _settled_labels;
        util::Heap<label_ref_t, _deref_ptrs_comp> _unsettled_labels;

        decltype(_settled_labels.begin()) _find_dominator(const Label& of_label) {
            auto dominator = _settled_labels.begin();

            for (; dominator != _settled_labels.end(); ++dominator) {
                // Abort as soon as a dominator has a higher t_min than of_label.
                // Any other label will
                if (_propagator->should_order_before(of_label, **dominator)) {
                    return _settled_labels.end();
                }
                if (_propagator->dominates(**dominator, of_label)) {
                    return dominator;
                }
            }

            return _settled_labels.end();
        }

      public:
        explicit LabelBucket(propagator_t& propagator)
            : _propagator(&propagator), _comp(propagator), _unsettled_labels(_comp) {}

        [[nodiscard]] bool empty() const { return _unsettled_labels.empty(); };

        void clear() {
            _settled_labels.clear();
            _unsettled_labels.clear();
        }

        [[nodiscard]] const label_ref_t& top() const { return _unsettled_labels.top(); }

        const Label* add(label_ref_t label) {
            // If there exist unsettled labels, it's safe to push the label on the stack without
            // checking dominance - if the pushed label is dominated that will be detected when
            // it becomes the top. If it is the new top, it replaces the old top and thus cannot
            // be dominated (it's cheaper)
            if (_unsettled_labels.empty()) {
                if (auto dominator = _find_dominator(*label); dominator != _settled_labels.end()) {
                    return nullptr;
                }
            } else {
                const auto& prev_top = _unsettled_labels.top();
                // _comp implements operator>
                if (_comp(prev_top, label)) {
                    // Label replaces prev_top
                    if (auto dominator = _find_dominator(*label);
                        dominator != _settled_labels.end()) {
                        return nullptr;
                    }
                }
            }
            _unsettled_labels.push(label);
            return label;
        }

        [[nodiscard]] const Label& extract_cheapest() {
            // Extract cheapest from _unsettled_labels
            label_ref_t extracted_label = _unsettled_labels.pop();
            // Move cheapest into _settled_labels
            auto insertion_point
                = std::upper_bound(_settled_labels.begin(), _settled_labels.end(), extracted_label,
                                   [this](const label_ref_t& lhs, const label_ref_t& rhs) {
                                       return _propagator->should_order_before(*lhs, *rhs);
                                   });
            _settled_labels.insert(insertion_point, extracted_label);

            // Restore heap invariant - the new top may be dominated by an already settled label
            while (!_unsettled_labels.empty()) {
                const label_ref_t& current_top = _unsettled_labels.top();
                if (auto dominator = _find_dominator(*current_top);
                    dominator != _settled_labels.end()) {
                    _unsettled_labels.pop();
                } else {
                    break;
                }
            }

            return *extracted_label;
        }
    };

    template <class Label> class LabelAllocator {
        std::vector<std::unique_ptr<Label>> _label_slab;

      public:
        LabelAllocator() : _label_slab(1000){};

        Label* allocate() {
            _label_slab.push_back(std::make_unique<Label>());
            return _label_slab.back().get();
        }

        void free() { _label_slab.clear(); }

        void free_last() { _label_slab.pop_back(); }
    };

    template <class Label> class FRVCP {
        using label_allocator_t = LabelAllocator<Label>;
        using label_bucket_t = LabelBucket<Label>;
        using propagator_t = Propagator<Label>;

        struct cheapest_queued_label_comp {
            propagator_t* _propagator;

            explicit cheapest_queued_label_comp(propagator_t& propagator)
                : _propagator(&propagator){};

            bool operator()(std::pair<VertexID, label_bucket_t*> lhs,
                            std::pair<VertexID, label_bucket_t*> rhs) {
                return _propagator->cheaper_than(*lhs.second->top(), *rhs.second->top());
            }
        };

        const Instance& _instance;
        std::shared_ptr<propagator_t> _propagator;
        std::vector<label_bucket_t> _buckets;
        NodeQueue<std::pair<DPVertexID, label_bucket_t*>, cheapest_queued_label_comp> _node_queue;

        label_allocator_t _label_slab;
        DPGraph _graph;

      public:
        FRVCP(const Instance& instance, std::shared_ptr<propagator_t> propagator)
            : _instance(instance),
              _propagator(std::move(propagator)),
              _node_queue(cheapest_queued_label_comp(*_propagator)) {}

        // Implementation relies on comparators with references to _propagator and buckets. Copying
        // the Solver with a default copy operator would result in stale references.
        FRVCP operator=(const FRVCP&) = delete;
        FRVCP(const FRVCP&) = delete;
        FRVCP& operator=(FRVCP&&) = default;
        FRVCP(FRVCP&&) = default;

        void clear() {
            _node_queue.clear();
            _buckets.clear();
            _graph.clear();
            _label_slab.free();
        }

        std::pair<const Label*, VertexID> _extract_next_label() {
            auto origin_vertex_id = _node_queue.extract_cheapest().first;
            const auto* label = &_buckets[origin_vertex_id].extract_cheapest();
            // Re-insert vertex into queue if labels remain
            if (!_buckets[origin_vertex_id].empty()) {
                _update_queue(origin_vertex_id);
            }
            return {label, origin_vertex_id};
        }

        void _build_graph(const std::vector<VertexID>& route) {
            assert(_graph.size() == 0);
            assert(route.front() == 0);
            assert(route.back() == 0);
            assert(route.size() >= 2);
            // TODO Make virtual and put in propagator

            VertexID prev_vertex_id = route.front();
            size_t prev_route_dp_id = _graph.add_vertex(_instance.getVertex(prev_vertex_id));

            for (size_t i = 1u; i < route.size(); ++i) {
                VertexID cur_vertex_id = route[i];
                const Vertex& current_vertex = _instance.getVertex(cur_vertex_id);
                if (current_vertex.station()) {
                    continue;
                }
                // Add the vertex
                auto customer_dp_id = _graph.add_vertex(current_vertex);
                // Connect the vertex
                _graph.add_edge(prev_route_dp_id, customer_dp_id);
                // Add stations
                std::vector<DPVertexID> added_stations;
                for (const Vertex& station : _instance.Stations()) {
                    added_stations.push_back(_graph.add_vertex(station));
                }

                for (DPVertexID station_i : added_stations) {
                    // Connect to customer
                    _graph.add_edge(prev_route_dp_id, station_i);
                    _graph.add_edge(station_i, customer_dp_id);
                    // Connect to other stations
                    for (DPVertexID station_j : added_stations) {
                        if (station_i != station_j) {
                            _graph.add_edge(station_i, station_j);
                        }
                    }
                }

                prev_route_dp_id = customer_dp_id;
                prev_vertex_id = cur_vertex_id;
            }
        }

        void _initialize_buckets() {
            // Create a bucket for each DPVertex
            _buckets.resize(_graph.size(), LabelBucket(*_propagator));
        }

        void _enqueue(VertexID vertex_id) { _node_queue.insert({vertex_id, &_buckets[vertex_id]}); }
        void _update_queue(VertexID vertex_id) {
            _node_queue.update({vertex_id, &_buckets[vertex_id]});
        }

        std::vector<VertexID> optimize(const std::vector<VertexID>& route) {
            assert(route.front() == 0);
            assert(route.back() == 0);
            assert(route.size() >= 2);
            _propagator->prepare(route);
            clear();

            // Build
            _build_graph(route);
            _initialize_buckets();
            // Initialize root label
            {
                auto* root_label = _label_slab.allocate();
                *root_label = _propagator->create_root_label();

                _buckets[0].add(root_label);
                _enqueue(0);
            }

            while (!_node_queue.empty()) {
                auto [extracted_label, origin_vertex_id] = _extract_next_label();
                const auto& origin_dp_vertex = _graph.get_vertex(origin_vertex_id);
                // origin_dp_vertex, nullptr);

                if (_propagator->is_final_label(*extracted_label)) {
                    // We have found a feasible solution
                    return _propagator->extract_path(*extracted_label);
                }

                // Propagate the label to all adjacent vertices
                auto [next_target_dp_vertex, target_vertices_end]
                    = _graph.get_successors(origin_vertex_id);
                for (; next_target_dp_vertex != target_vertices_end; ++next_target_dp_vertex) {
                    const auto& target_dp_vertex = **next_target_dp_vertex;

                    if (auto propagated_label = _propagator->propagate(
                            *extracted_label, origin_dp_vertex.original_vertex(),
                            target_dp_vertex.original_vertex(),
                            _instance.getArc(origin_dp_vertex.original_vertex().id,
                                             target_dp_vertex.original_vertex().id));
                        propagated_label) {
                        // Store candidate label
                        Label* next_label = _label_slab.allocate();
                        *next_label = std::move(*propagated_label);
                        if (_buckets[target_dp_vertex.dp_vertex_id()].add(next_label)) {
                            _update_queue(target_dp_vertex.dp_vertex_id());
                        }
                    }
                }
            }
            return route;
        }
    };

}  // namespace routingblocks

#endif  // routingblocks_FRVCP_H
