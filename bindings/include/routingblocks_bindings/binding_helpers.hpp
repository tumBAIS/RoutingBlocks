
#ifndef BINDINGS_HELPERS_HPP
#define BINDINGS_HELPERS_HPP

#include <sstream>
#include <string>

// https://github.com/pybind/pybind11/issues/1546
#define BIND_LIFETIME_PYTHON(type, name)                                                        \
    namespace pybind11::detail {                                                                \
        template <> struct type_caster<std::shared_ptr<type>> {                                 \
            PYBIND11_TYPE_CASTER(std::shared_ptr<type>, _(name));                               \
            using BaseCaster = copyable_holder_caster<type, std::shared_ptr<type>>;             \
            bool load(pybind11::handle src, bool b) {                                           \
                BaseCaster bc;                                                                  \
                bool success = bc.load(src, b);                                                 \
                if (!success) {                                                                 \
                    return false;                                                               \
                }                                                                               \
                auto py_obj = pybind11::reinterpret_borrow<pybind11::object>(src);              \
                auto base_ptr = static_cast<std::shared_ptr<type>>(bc);                         \
                auto py_obj_ptr                                                                 \
                    = std::shared_ptr<object>{new object{py_obj}, [](auto* py_object_ptr) {     \
                                                  gil_scoped_acquire gil;                       \
                                                  delete py_object_ptr;                         \
                                              }};                                               \
                value = std::shared_ptr<type>(py_obj_ptr, base_ptr.get());                      \
                return true;                                                                    \
            }                                                                                   \
            static handle cast(std::shared_ptr<type> base, return_value_policy rvp, handle h) { \
                return BaseCaster::cast(base, rvp, h);                                          \
            }                                                                                   \
        };                                                                                      \
        template <> struct is_holder_type<type, std::shared_ptr<type>> : std::true_type {};     \
    }

namespace bindings::helpers {
    template <class T> std::string ostream_to_string(const T& obj) {
        std::stringstream ss;
        ss << obj;
        return ss.str();
    }

    template <class T> auto py_iterator_from_range(T&& range) {
        return pybind11::make_iterator(std::begin(range), std::end(range));
    }

    template <class data_t> routingblocks::Vertex vertex_constructor(size_t vid, std::string name,
                                                                     bool is_station, bool is_depot,
                                                                     data_t user_data) {
        return routingblocks::Vertex(vid, name, is_station, is_depot,
                                     std::make_shared<data_t>(std::move(user_data)));
    }

    template <class data_t> routingblocks::Arc arc_constructor(data_t user_data) {
        return routingblocks::Arc(std::make_shared<data_t>(std::move(user_data)));
    }

}  // namespace bindings::helpers

#endif
