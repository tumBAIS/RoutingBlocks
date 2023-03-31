
#ifndef UTIL_HEAP_H
#define UTIL_HEAP_H

#include <queue>

namespace util {
    template <class T, class Comp = std::greater<T>> class Heap {
      private:
        using heap_impl_t = std::priority_queue<T, std::vector<T>, Comp>;

      public:
        using size_t = typename heap_impl_t::size_type;

      private:
        Comp _comp;
        heap_impl_t _heap_impl;

      public:
        Heap() = default;
        explicit Heap(const Comp& comp) : _comp(comp), _heap_impl(_comp){};

        T pop() {
            assert(!empty());
            T moved_from_top = std::move(const_cast<T&>(_heap_impl.top()));
            _heap_impl.pop();
            return moved_from_top;
        };
        const T& top() const { return _heap_impl.top(); };

        void push(T value) { _heap_impl.push(std::move(value)); }

        [[nodiscard]] bool empty() const { return _heap_impl.empty(); }

        [[nodiscard]] size_t size() const { return _heap_impl.size(); }

        void clear() {
            decltype(_heap_impl) empty_heap(_comp);
            _heap_impl.swap(empty_heap);
        }
    };
}  // namespace util

#endif  // UTIL_HEAP_H
