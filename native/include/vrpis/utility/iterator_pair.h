
#ifndef _VRPIS_ITERATOR_PAIR_H
#define _VRPIS_ITERATOR_PAIR_H

namespace vrpis::utility {
    /***
     * Provides a simple iterator helper class that allows to use range-based for loops with
     * std::pair<iterator, iterator> objects.
     */
    template <typename Iterator> class iterator_pair {
        Iterator _begin;
        Iterator _end;

      public:
        iterator_pair(Iterator begin, Iterator end) : _begin(begin), _end(end) {}
        Iterator begin() const { return _begin; }
        Iterator end() const { return _end; }
    };

    template <typename Iterator>
    iterator_pair<Iterator> make_iterator_pair(Iterator begin, Iterator end) {
        return iterator_pair<Iterator>(begin, end);
    }
}  // namespace vrpis::utility

#endif  //_VRPIS_ITERATOR_PAIR_H
