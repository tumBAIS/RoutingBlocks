// Copyright (c) 2023 Patrick S. Klein (@libklein)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <routingblocks/operators/InsertStationOperator.h>

namespace routingblocks {

    cost_t InsertStationMove::get_cost_delta(Evaluation& evaluation, const Instance& instance,
                                             const Solution& solution) const {
        auto [route, node] = to_iter(_after_node, solution);
        const auto& station = instance.getVertex(_station_id);

        cost_t cost = evaluate_insertion(evaluation, instance, *route, node, station);
        return cost - route->cost();
    }

    void InsertStationMove::apply([[maybe_unused]] const Instance& instance,
                                  Solution& solution) const {
        auto [route, node] = to_iter(_after_node, solution);
        solution.insert_vertex_after(route, node, instance.getStation(_station_id).id);
    }
    InsertStationMove::InsertStationMove(const NodeLocation& afterNode, VertexID stationId)
        : _after_node(afterNode), _station_id(stationId) {}

    void InsertStationOperator::prepare_search(const Solution&) {}
    void InsertStationOperator::finalize_search() {}

    std::shared_ptr<Move> InsertStationOperator::find_next_improving_move(
        Operator::eval_t& evaluation, const Solution& solution, const Move* previous_move) {
#ifndef NDEBUG
        assert(!previous_move || dynamic_cast<const InsertStationMove*>(previous_move) != nullptr);
#endif
        SolutionArcIterator arc_iterator_end;
        auto [arc_iterator, last_station_id]
            = _recover_move(solution, static_cast<const InsertStationMove*>(previous_move));

        for (; arc_iterator != arc_iterator_end; ++arc_iterator) {
            if (arc_iterator->route->feasible()) {
                arc_iterator.move_to_end_of_route();
                continue;
            }
            for (VertexID station_id = last_station_id; station_id < _instance.NumberOfStations();
                 ++station_id) {
                const auto& station = _instance.getStation(station_id);
                cost_t cost = evaluate_insertion(evaluation, _instance, *arc_iterator->route,
                                                 arc_iterator->origin_node, station)
                              - arc_iterator->route->cost();
                if (cost < 0.) {
                    return std::make_shared<InsertStationMove>(
                        location_cast(solution, arc_iterator->route, arc_iterator->origin_node),
                        station_id);
                }
            }
            last_station_id = 0;
        }

        return nullptr;
    }

    std::pair<SolutionArcIterator, VertexID> InsertStationOperator::_recover_move(
        const Solution& solution, const InsertStationMove* move) const {
        if (move) {
            auto [route, after_node] = to_iter(move->_after_node, solution);
            auto arc_iterator = SolutionArcIterator(solution, {route, after_node});
            VertexID station_id = move->_station_id + 1;
            if (station_id >= _instance.NumberOfStations()) {
                ++arc_iterator;
                station_id = 0;
            }
            return {arc_iterator, station_id};
        } else {
            return {SolutionArcIterator(solution, {solution.begin(), solution.begin()->begin()}),
                    0};
        }
    }

    InsertStationOperator::InsertStationOperator(const Instance& instance) : _instance(instance) {}

}  // namespace routingblocks