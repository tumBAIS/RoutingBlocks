#include <routingblocks/Solution.h>
#include <routingblocks/lns_operators.h>

namespace routingblocks::lns::operators {

    std::vector<routingblocks::NodeLocation> sample_positions(
        const routingblocks::Solution& solution, utility::random& random, size_t k,
        bool include_depot) {
        std::vector<routingblocks::NodeLocation> sample;
        if (k == 0) return sample;
        sample.reserve(k);
        auto n = routingblocks::number_of_nodes(solution, include_depot);
        if (k > n) {
            throw std::runtime_error("Cannot sample fewer nodes than are in the solution.");
        }

        // Reservior sampling
        size_t i = 0;
        for (auto route_iter = solution.begin(); route_iter != solution.end(); ++route_iter) {
            for (auto node_iter = std::next(route_iter->begin(), 1 - include_depot);
                 node_iter != route_iter->end_depot(); ++node_iter, ++i) {
                if (sample.size() < k) {  // Always accept until full
                    sample.push_back(routingblocks::location_cast(solution, route_iter, node_iter));
                    continue;
                }

                auto j = random.generateInt(static_cast<decltype(i)>(0), i);
                if (j < k) {
                    sample[j] = routingblocks::location_cast(solution, route_iter, node_iter);
                }
            }
        }

        return sample;
    }

    std::vector<routingblocks::VertexID> RandomRemoval::apply(
        [[maybe_unused]] routingblocks::Evaluation& evaluation, routingblocks::Solution& sol,
        size_t numberOfRemovedCustomers) {
        std::vector<routingblocks::VertexID> removed_vertices(numberOfRemovedCustomers);
        // Ensure more than numberOfremovedcustomers in solution
        if (numberOfRemovedCustomers > routingblocks::number_of_nodes(sol)) {
            throw std::runtime_error("Cannot remove more nodes than are in the solution!");
        }
        auto positions_to_remove = sample_positions(sol, _random, numberOfRemovedCustomers, false);
        // Track removed vertices.
        std::transform(positions_to_remove.begin(), positions_to_remove.end(),
                       removed_vertices.begin(),
                       [&sol](const routingblocks::NodeLocation& location) {
                           return routingblocks::to_ref(location, sol).second->vertex_id();
                       });

        // Actually remove the selected vertices.
        sol.remove_vertices(positions_to_remove.begin(), positions_to_remove.end());
        return removed_vertices;
    }

    std::string_view RandomRemoval::name() const { return "RandomRemoval"; }

    bool RandomRemoval::can_apply_to([[maybe_unused]] const routingblocks::Solution& sol) const {
        return true;
    }

    void RandomInsertion::apply([[maybe_unused]] routingblocks::Evaluation& evaluation,
                                routingblocks::Solution& sol,
                                const std::vector<routingblocks::VertexID>& missing_vertices) {
        size_t inserted_vertices = 0;
        std::vector<std::pair<routingblocks::VertexID, routingblocks::NodeLocation>> locations;
        locations.reserve(missing_vertices.size());
        auto next_missing_vertex = missing_vertices.begin();
        while (inserted_vertices < missing_vertices.size()) {
            locations.clear();
            auto batch_size = std::min(routingblocks::number_of_nodes(sol, true),
                                       missing_vertices.size() - inserted_vertices);
            auto insertion_positions = sample_positions(sol, _random, batch_size, true);

            auto end_missing_vertex_batch = std::next(next_missing_vertex, batch_size);
            std::transform(next_missing_vertex, end_missing_vertex_batch,
                           insertion_positions.begin(), std::back_inserter(locations),
                           [](routingblocks::VertexID vid, routingblocks::NodeLocation location) {
                               return std::make_pair(vid, location);
                           });

            sol.insert_vertices_after(locations.begin(), locations.end());
            inserted_vertices += batch_size;
            next_missing_vertex = end_missing_vertex_batch;
        }
    }

    bool RandomInsertion::can_apply_to(const routingblocks::Solution& sol) const { return true; }

    std::string_view RandomInsertion::name() const { return "RandomInsertion"; }
}  // namespace routingblocks::lns::operators