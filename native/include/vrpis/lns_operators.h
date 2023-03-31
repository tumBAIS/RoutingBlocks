
#ifndef vrpis_LNS_OPERATORS_H
#define vrpis_LNS_OPERATORS_H

#include "vrpis/Solution.h"
#include "vrpis/operators.h"
#include "vrpis/utility/random.h"

namespace vrpis::lns::operators {
    /**
     * Randomly samples k random positions from the solution without replacement.
     * @param solution The solution to pick a position from.
     * @param random The random number generator to use.
     * @param k Sample size
     * @param include_depot True to include the start depot.
     * @return A list of node locations.
     */
    std::vector<vrpis::NodeLocation> sample_positions(const vrpis::Solution& solution,
                                                      vrpis::utility::random& random, size_t k,
                                                      bool include_depot = false);

    class RandomRemoval : public vrpis::destroy_operator {
        vrpis::utility::random _random;

      public:
        explicit RandomRemoval(vrpis::utility::random random) : _random(std::move(random)) {}

        std::vector<vrpis::VertexID> apply(vrpis::Evaluation& evaluation, vrpis::Solution& sol,
                                           size_t numberOfRemovedCustomers) override;
        [[nodiscard]] std::string_view name() const override;
        [[nodiscard]] bool can_apply_to(const vrpis::Solution& sol) const override;
    };

    class RandomInsertion : public vrpis::repair_operator {
        vrpis::utility::random _random;

      public:
        explicit RandomInsertion(vrpis::utility::random random) : _random(std::move(random)) {}
        void apply(vrpis::Evaluation& evaluation, vrpis::Solution& sol,
                   const std::vector<vrpis::VertexID>& missing_vertices) override;
        std::string_view name() const override;
        bool can_apply_to(const vrpis::Solution& sol) const override;
    };
}  // namespace vrpis::lns::operators

#endif  // vrpis_LNS_OPERATORS_H
