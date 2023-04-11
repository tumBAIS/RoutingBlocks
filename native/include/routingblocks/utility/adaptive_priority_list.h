#pragma once

#include <forward_list>
#include <iterator>
#include <memory>
#include <numeric>
#include <utility>

#include "random.h"

namespace routingblocks::utility {
    template <class T> class adaptive_priority_list {
      public:
        using value_type = T;

      private:
        struct priority_list_entry {
            T value;
            double period_score;
            unsigned int period_invocations;
            double weight;

            template <typename B, typename... Args> explicit priority_list_entry(Args&&... args)
                : period_score(0.0),
                  period_invocations(0),
                  weight(1.0),
                  value(std::forward<Args>(args)...) {}

            explicit priority_list_entry(T&& elem)
                : value(std::move(elem)), period_score(0.0), period_invocations(0), weight(1.0){};
        };

        using priority_list_t = std::forward_list<priority_list_entry>;
        priority_list_t _priority_list;
        double _total_weight;

        // TODO Remove, should not keep track of when to update
        double _smoothing_factor;

        size_t _size;

        random rand;

      public:
        class const_iterator {
            friend adaptive_priority_list<T>;
            typename priority_list_t::const_iterator list_iter;

          public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = T;
            using difference_type = typename priority_list_t::const_iterator::difference_type;
            using pointer = T*;
            using reference = T&;

            const_iterator() = default;
            explicit const_iterator(typename priority_list_t::const_iterator base_iter) {
                this->list_iter = base_iter;
            };

            const_iterator& operator++() {
                list_iter++;
                return *this;
            }

            const_iterator operator++(int) {
                auto retval = *this;
                ++(*this);
                return retval;
            }
            bool operator==(const_iterator other) const { return list_iter == other.list_iter; }
            bool operator!=(const_iterator other) const { return list_iter != other.list_iter; }
            const T& operator*() const { return list_iter->value; }
            const T* operator->() const { return &(list_iter->value); }
        };

        class iterator {
            friend adaptive_priority_list<T>;
            typename priority_list_t::iterator list_iter;

          public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = T;
            using difference_type = typename priority_list_t::const_iterator::difference_type;
            using pointer = T*;
            using reference = T&;

            iterator() = default;
            explicit iterator(typename priority_list_t::iterator base_iter) {
                this->list_iter = base_iter;
            };

            iterator& operator++() {
                list_iter++;
                return *this;
            }

            iterator operator++(int) {
                auto retval = *this;
                ++(*this);
                return retval;
            }
            bool operator==(iterator other) const { return list_iter == other.list_iter; }
            bool operator!=(iterator other) const { return list_iter != other.list_iter; }
            T& operator*() const { return list_iter->value; }
            T* operator->() const { return &(list_iter->value); }

            operator const_iterator() const { return const_iterator(this->list_iter); };
        };

        adaptive_priority_list(random random, double smoothingFactor)
            : _total_weight(0.0),
              _size(0),
              _smoothing_factor(smoothingFactor),
              rand(std::move(random)){};  // Zero initialization is fine

        void set_smoothing_factor(double factor) { _smoothing_factor = factor; }

        iterator add(T&& elem) {
            double new_operator_weight = _avg_weight();
            _total_weight += new_operator_weight;
            _size += 1;
            _priority_list.emplace_front(std::move(elem));
            return iterator(_priority_list.begin());
        }

        template <typename... Args> iterator emplace(Args&&... args) {
            double new_operator_weight = _avg_weight();
            _total_weight += new_operator_weight;
            _size += 1;
            _priority_list.emplace_front(std::forward<Args>(args)...);
            return iterator(_priority_list.begin());
        }

        void erase(const_iterator elem) {
            _total_weight -= elem.list_iter->weight;
            _size -= 1;
            // Find predecessor
            iterator pred = iterator(_priority_list.before_begin());
            for (iterator cur = begin(); cur != elem; pred = cur++) {
            }
            _priority_list.erase_after(pred.list_iter);
        }

        void update(iterator elem, double score) {
            elem.list_iter->period_score += score;
            elem.list_iter->period_invocations++;
        }

        void adapt() {
            _total_weight = 0.0;
            for (auto& entry : _priority_list) {
                entry.weight = _smoothing_factor
                                   * (entry.period_score / std::max(1u, entry.period_invocations))
                               + (1.0 - _smoothing_factor) * entry.weight;

                assert(entry.weight >= 0.0);

                _total_weight += entry.weight;
                // Period is finished. Reset period related scores
                entry.period_score = 0.0;
                entry.period_invocations = 0;
            }

            assert(_total_weight > 0.0 || empty());
        }

        iterator pick() {
            double selected = rand.uniform(0.0, _total_weight);
            // Traverse the list to find the interval
            double current_upper = 0.0;
            for (typename priority_list_t::iterator entry = _priority_list.begin();
                 entry != _priority_list.end(); ++entry) {
                current_upper += entry->weight;
                if (current_upper >= selected) {
                    return iterator(entry);
                }
            }

            throw std::runtime_error("Cannot pick from empty priority list!");
        }

        [[nodiscard]] auto size() const { return _size; }

        [[nodiscard]] bool empty() const { return _priority_list.empty(); }

        [[nodiscard]] const_iterator begin() const {
            return const_iterator(_priority_list.begin());
        }

        [[nodiscard]] iterator begin() { return iterator(_priority_list.begin()); }

        [[nodiscard]] const_iterator end() const { return const_iterator(_priority_list.end()); }

        [[nodiscard]] iterator end() { return iterator(_priority_list.end()); }

        void reset_weights() {
            for (auto& entry : _priority_list) {
                entry.weight = 1.0;
                entry.period_score = 0.0;
                entry.period_invocations = 0.0;
            }
            _total_weight = size() * 1.0;
        }

        adaptive_priority_list& operator=(const adaptive_priority_list& other) = delete;

        adaptive_priority_list(const adaptive_priority_list& other) = delete;

        adaptive_priority_list(adaptive_priority_list&& other) noexcept = default;

        adaptive_priority_list& operator=(adaptive_priority_list&& other) = default;

        ~adaptive_priority_list() = default;

      private:
        void update_total() {
            _total_weight
                = std::accumulate(_priority_list.begin(), _priority_list.end(), 0.,
                                  [](double acc, const auto& entry) { return acc + entry.weight; });
        };

        [[nodiscard]] double _avg_weight() const {
            if (_size == 0) return 1.0;
            return _total_weight / _size;
        }
    };
}  // namespace routingblocks::utility
