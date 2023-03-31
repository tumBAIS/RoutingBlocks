#include "vrpis_bindings/Operators.h"

#include <vrpis/LocalSearch.h>
#include <vrpis/operators/InsertStationOperator.h>
#include <vrpis/operators/InterRouteTwoOptOperator.h>
#include <vrpis/operators/RemoveStationOperator.h>

#include "vrpis/operators/SwapOperator.h"
#include "vrpis_bindings/binding_helpers.hpp"

namespace vrpis::bindings {

    class PyOperator : public vrpis::Operator {
        using vrpis::Operator::Operator;

      public:
        void prepare_search(const Solution& solution) override {
            PYBIND11_OVERLOAD_PURE(void, vrpis::Operator, prepare_search, solution);
        }
        std::shared_ptr<Move> find_next_improving_move(eval_t& evaluation, const Solution& solution,
                                                       const Move* previous_move) override {
            PYBIND11_OVERLOAD_PURE(std::shared_ptr<Move>, vrpis::Operator, find_next_improving_move,
                                   evaluation, solution, previous_move);
        }
        void finalize_search() override {
            PYBIND11_OVERLOAD_PURE(void, vrpis::Operator, finalize_search);
        }
    };

    class PyMove : public vrpis::Move {
        using vrpis::Move::Move;

      public:
        cost_t get_cost_delta(Evaluation& evaluation, const Instance& instance,
                              const Solution& solution) const override {
            PYBIND11_OVERLOAD_PURE(cost_t, vrpis::Move, get_cost_delta, evaluation, instance,
                                   solution);
        }

        void apply(const Instance& instance, Solution& solution) const override {
            PYBIND11_OVERRIDE_PURE(void, vrpis::Move, apply, instance, solution);
        }
    };

    auto bind_operator_interface(pybind11::module_& m) {
        return pybind11::class_<vrpis::Operator, PyOperator, std::shared_ptr<vrpis::Operator>>(
                   m, "Operator")
            .def(pybind11::init<>())
            .def("prepare_search", &vrpis::Operator::prepare_search,
                 "Prepare the operator for "
                 "searching for a move.")
            .def("find_next_improving_move", &vrpis::Operator::find_next_improving_move,
                 "Find the next improving move.")
            .def("finalize_search", &vrpis::Operator::finalize_search, "Finalize the search.");
    }

    auto bind_move_interface(pybind11::module_& m) {
        return pybind11::class_<vrpis::Move, PyMove, std::shared_ptr<vrpis::Move>>(m, "Move")
            .def(pybind11::init<>())
            .def("get_cost_delta", &vrpis::Move::get_cost_delta, "Get the cost of the move.")
            .def("apply", &vrpis::Move::apply, "Apply the move to the solution.");
    }

    template <size_t OriginSegmentSize, size_t TargetSegmentSize>
    void bind_swap_operator(pybind11::module_& m, auto& operator_interface, auto& move_interface) {
        using operator_t = vrpis::SwapOperator<OriginSegmentSize, TargetSegmentSize>;
        using operator_move_t = vrpis::SwapMove<OriginSegmentSize, TargetSegmentSize>;

        std::stringstream base_name;
        base_name << "SwapOperator"
                  << "_" << OriginSegmentSize << "_" << TargetSegmentSize;
        auto base_name_str = base_name.str();

        pybind11::class_<operator_t, std::shared_ptr<operator_t>>(
            m, base_name_str.data(), operator_interface,
            "Swap operator. Swaps a segment of customers from a route to another route.")
            .def(pybind11::init<const Instance&, const utility::arc_set*>(),
                 pybind11::keep_alive<1, 3>())
            .def("prepare_search", &operator_t::prepare_search)
            .def("find_next_improving_move", &operator_t::find_next_improving_move)
            .def("finalize_search", &operator_t::finalize_search)
            .def("create_move", &operator_t::create_move,
                 "Create a move that represents a given generator arc.");

        std::stringstream move_name;
        move_name << "SwapOperatorMove"
                  << "_" << OriginSegmentSize << "_" << TargetSegmentSize;
        auto move_name_str = move_name.str();

        pybind11::class_<operator_move_t, std::shared_ptr<operator_move_t>>(
            m, move_name_str.data(), move_interface,
            "Relocate move. Moves a customer from a route to "
            "another route. Implemented for inter- and intra-route moves.")
            .def(pybind11::init<NodeLocation, NodeLocation>())
            .def("get_cost_delta", &operator_move_t::get_cost_delta)
            .def("apply", &operator_move_t::apply);
    }

    void bind_inter_route_two_opt(pybind11::module_& m, auto& operator_interface,
                                  auto& move_interface) {
        pybind11::class_<vrpis::InterRouteTwoOptOperator,
                         std::shared_ptr<vrpis::InterRouteTwoOptOperator>>(
            m, "InterRouteTwoOptOperator", operator_interface,
            "Considers two-opt moves between distinct routes. Tries to integrate the "
            "generator arc into the solution.")
            .def(pybind11::init<const Instance&, const utility::arc_set*>(),
                 pybind11::keep_alive<1, 3>())
            .def("prepare_search", &vrpis::InterRouteTwoOptOperator::prepare_search)
            .def("find_next_improving_move",
                 &vrpis::InterRouteTwoOptOperator::find_next_improving_move)
            .def("finalize_search", &vrpis::InterRouteTwoOptOperator::finalize_search)
            .def("create_move", &vrpis::InterRouteTwoOptOperator::create_move,
                 "Create a move that represents a given generator arc.");

        pybind11::class_<vrpis::InterRouteTwoOptMove, std::shared_ptr<vrpis::InterRouteTwoOptMove>>(
            m, "InterRouteTwoOptMove", move_interface)
            .def(pybind11::init<NodeLocation, NodeLocation>())
            .def("get_cost_delta", &vrpis::InterRouteTwoOptMove::get_cost_delta)
            .def("apply", &vrpis::InterRouteTwoOptMove::apply);
    }

    void bind_station_in_operator(pybind11::module_& m, auto& operator_interface,
                                  auto& move_interface) {
        pybind11::class_<vrpis::InsertStationOperator,
                         std::shared_ptr<vrpis::InsertStationOperator>>(
            m, "InsertStationOperator", operator_interface,
            "Considers station insertions between consecutive vertices.")
            .def(pybind11::init<const Instance&>())
            .def("prepare_search", &vrpis::InsertStationOperator::prepare_search)
            .def("find_next_improving_move",
                 &vrpis::InsertStationOperator::find_next_improving_move)
            .def("finalize_search", &vrpis::InsertStationOperator::finalize_search);

        pybind11::class_<vrpis::InsertStationMove, std::shared_ptr<vrpis::InsertStationMove>>(
            m, "StationInsertionMove", move_interface)
            .def(pybind11::init<NodeLocation, VertexID>())
            .def("get_cost_delta", &vrpis::InsertStationMove::get_cost_delta)
            .def("apply", &vrpis::InsertStationMove::apply);
    }

    void bind_station_out_operator(pybind11::module_& m, auto& operator_interface,
                                   auto& move_interface) {
        pybind11::class_<vrpis::RemoveStationOperator,
                         std::shared_ptr<vrpis::RemoveStationOperator>>(
            m, "RemoveStationOperator", operator_interface,
            "Considers station removals between consecutive vertices.")
            .def(pybind11::init<const Instance&>())
            .def("prepare_search", &vrpis::RemoveStationOperator::prepare_search)
            .def("find_next_improving_move",
                 &vrpis::RemoveStationOperator::find_next_improving_move)
            .def("finalize_search", &vrpis::RemoveStationOperator::finalize_search);

        pybind11::class_<vrpis::RemoveStationMove, std::shared_ptr<vrpis::RemoveStationMove>>(
            m, "StationRemovalMove", move_interface)
            .def(pybind11::init<NodeLocation>())
            .def("get_cost_delta", &vrpis::RemoveStationMove::get_cost_delta)
            .def("apply", &vrpis::RemoveStationMove::apply);
    }

    void bind_arc_set(pybind11::module_& m) {
        pybind11::class_<utility::arc_set>(m, "ArcSet", "A set of arcs.")
            .def(pybind11::init<VertexID>())
            .def("include_arc", &utility::arc_set::include_arc, "Include an arc in the set.")
            .def("forbid_arc", &utility::arc_set::forbid_arc, "Forbid an arc in the set.")
            .def("includes_arc", &utility::arc_set::includes_arc, "Check if an arc is allowed.");
    }

    void bind_operators(pybind11::module_& m) {
        auto operator_interface = bind_operator_interface(m);
        auto move_interface = bind_move_interface(m);

        bind_arc_set(m);

        bind_inter_route_two_opt(m, operator_interface, move_interface);
        bind_station_in_operator(m, operator_interface, move_interface);
        bind_station_out_operator(m, operator_interface, move_interface);

        bind_swap_operator<0, 1>(m, operator_interface, move_interface);
        bind_swap_operator<0, 2>(m, operator_interface, move_interface);
        bind_swap_operator<0, 3>(m, operator_interface, move_interface);
        bind_swap_operator<1, 1>(m, operator_interface, move_interface);
        bind_swap_operator<1, 2>(m, operator_interface, move_interface);
        bind_swap_operator<1, 3>(m, operator_interface, move_interface);
        bind_swap_operator<2, 1>(m, operator_interface, move_interface);
        bind_swap_operator<2, 2>(m, operator_interface, move_interface);
        bind_swap_operator<2, 3>(m, operator_interface, move_interface);
        bind_swap_operator<3, 1>(m, operator_interface, move_interface);
        bind_swap_operator<3, 2>(m, operator_interface, move_interface);
        bind_swap_operator<3, 3>(m, operator_interface, move_interface);
    }

}  // namespace vrpis::bindings
