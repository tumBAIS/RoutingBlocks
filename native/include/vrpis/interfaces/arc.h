
#ifndef _VRPIS_ARC_H
#define _VRPIS_ARC_H

#include <memory>

namespace vrpis {
    struct Arc {
        using data_t = std::shared_ptr<void>;
        // Pointer to type-erased (arbitrary) data
        data_t data;

        explicit Arc(data_t data);

        template <class T> T& get_data() { return *static_cast<T*>(data.get()); }
        template <class T> const T& get_data() const { return *static_cast<const T*>(data.get()); }
    };
}  // namespace vrpis

#endif  //_VRPIS_ARC_H
