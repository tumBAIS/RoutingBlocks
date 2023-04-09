#include <fmt/format.h>
#include <fmt/ostream.h>
#include <vrpis/Solution.h>
#include <vrpis/lns_operators.h>

namespace vrpis::lns::operators {

    std::vector<vrpis::NodeLocation> sample_positions(const vrpis::Solution& solution,
                                                      utility::random& random, size_t k,
                                                      bool include_depot) {
        std::vector<vrpis::NodeLocation> sample;
        if (k == 0) return sample;
        sample.reserve(k);
        auto n = vrpis::number_of_nodes(solution, include_depot);
        if (k > n) {
            throw std::runtime_error("Cannot sample fewer nodes than are in the solution.");
        }

        // Reservior sampling
        size_t i = 0;
        for (auto route_iter = solution.begin(); route_iter != solution.end(); ++route_iter) {
            for (auto node_iter = std::next(route_iter->begin(), 1 - include_depot);
                 node_iter != route_iter->end_depot(); ++node_iter, ++i) {
                if (sample.size() < k) {  // Always accept until full
                    sample.push_back(vrpis::location_cast(solution, route_iter, node_iter));
                    continue;
                }

                auto j = random.generateInt(static_cast<decltype(i)>(0), i);
                if (j < k) {
                    sample[j] = vrpis::location_cast(solution, route_iter, node_iter);
                }
            }
        }

        return sample;
    }

    std::vector<vrpis::VertexID> RandomRemoval::apply(
        [[maybe_unused]] vrpis::Evaluation& evaluation, vrpis::Solution& sol,
        size_t numberOfRemovedCustomers) {
        std::vector<vrpis::VertexID> removed_vertices(numberOfRemovedCustomers);
        // Ensure more than numberOfremovedcustomers in solution
        if (numberOfRemovedCustomers > vrpis::number_of_nodes(sol)) {
            throw std::runtime_error("Cannot remove more nodes than are in the solution!");
        }
        auto positions_to_remove = sample_positions(sol, _random, numberOfRemovedCustomers, false);
        // Track removed vertices.
        std::transform(positions_to_remove.begin(), positions_to_remove.end(),
                       removed_vertices.begin(), [&sol](const vrpis::NodeLocation& location) {
                           return vrpis::to_ref(location, sol).second->vertex_id();
                       });

        // Actually remove the selected vertices.
        sol.remove_vertices(positions_to_remove.begin(), positions_to_remove.end());
        return removed_vertices;
    }

    std::string_view RandomRemoval::name() const { return "RandomRemoval"; }

    bool RandomRemoval::can_apply_to([[maybe_unused]] const vrpis::Solution& sol) const {
        return true;
    }

    void RandomInsertion::apply([[maybe_unused]] vrpis::Evaluation& evaluation,
                                vrpis::Solution& sol,
                                const std::vector<vrpis::VertexID>& missing_vertices) {
        size_t inserted_vertices = 0;
        std::vector<std::pair<vrpis::VertexID, vrpis::NodeLocation>> locations;
        locations.reserve(missing_vertices.size());
        auto next_missing_vertex = missing_vertices.begin();
        while (inserted_vertices < missing_vertices.size()) {
            locations.clear();
            auto batch_size = std::min(vrpis::number_of_nodes(sol, true),
                                       missing_vertices.size() - inserted_vertices);
            auto insertion_positions = sample_positions(sol, _random, batch_size, true);

            auto end_missing_vertex_batch = std::next(next_missing_vertex, batch_size);
            std::transform(next_missing_vertex, end_missing_vertex_batch,
                           insertion_positions.begin(), std::back_inserter(locations),
                           [](vrpis::VertexID vid, vrpis::NodeLocation location) {
                               return std::make_pair(vid, location);
                           });

            sol.insert_vertices_after(locations.begin(), locations.end());
            inserted_vertices += batch_size;
            next_missing_vertex = end_missing_vertex_batch;
        }
    }

    bool RandomInsertion::can_apply_to(const vrpis::Solution& sol) const { return true; }

    std::string_view RandomInsertion::name() const { return "RandomInsertion"; }
}  // namespace vrpis::lns::operators