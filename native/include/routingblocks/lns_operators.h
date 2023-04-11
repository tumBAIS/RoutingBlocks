
#ifndef routingblocks_LNS_OPERATORS_H
#define routingblocks_LNS_OPERATORS_H

#include <routingblocks/Solution.h>
#include <routingblocks/operators.h>
#include <routingblocks/utility/random.h>

namespace routingblocks::lns::operators {
    /**
     * Randomly samples k random positions from the solution without replacement.
     * @param solution The solution to pick a position from.
     * @param random The random number generator to use.
     * @param k Sample size
     * @param include_depot True to include the start depot.
     * @return A list of node locations.
     */
    std::vector<routingblocks::NodeLocation> sample_positions(
        const routingblocks::Solution& solution, routingblocks::utility::random& random, size_t k,
        bool include_depot = false);

    class RandomRemoval : public routingblocks::destroy_operator {
        routingblocks::utility::random _random;

      public:
        explicit RandomRemoval(routingblocks::utility::random random)
            : _random(std::move(random)) {}

        std::vector<routingblocks::VertexID> apply(routingblocks::Evaluation& evaluation,
                                                   routingblocks::Solution& sol,
                                                   size_t numberOfRemovedCustomers) override;
        [[nodiscard]] std::string_view name() const override;
        [[nodiscard]] bool can_apply_to(const routingblocks::Solution& sol) const override;
    };

    class RandomInsertion : public routingblocks::repair_operator {
        routingblocks::utility::random _random;

      public:
        explicit RandomInsertion(routingblocks::utility::random random)
            : _random(std::move(random)) {}
        void apply(routingblocks::Evaluation& evaluation, routingblocks::Solution& sol,
                   const std::vector<routingblocks::VertexID>& missing_vertices) override;
        std::string_view name() const override;
        bool can_apply_to(const routingblocks::Solution& sol) const override;
    };
}  // namespace routingblocks::lns::operators

#endif  // routingblocks_LNS_OPERATORS_H
