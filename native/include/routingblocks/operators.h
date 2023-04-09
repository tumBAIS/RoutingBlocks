
#ifndef routingblocks_OPERATORS_H
#define routingblocks_OPERATORS_H

#include <routingblocks/evaluation.h>
#include <routingblocks/types.h>
#include <routingblocks/vertex.h>

#include <memory>
#include <vector>

namespace routingblocks {

    class Solution;

    /**
     * \brief Destory Operator interface
     */
    class destroy_operator : public std::enable_shared_from_this<destroy_operator> {
      public:
        /**
         * Applies the operator to the passed solution.
         */
        virtual std::vector<routingblocks::VertexID> apply(routingblocks::Evaluation& evaluation,
                                                           routingblocks::Solution& sol,
                                                           size_t numberOfRemovedCustomers)
            = 0;

        [[nodiscard]] virtual std::string_view name() const = 0;

        /**
         * Returns true if the operator can be applied to the passed solution. False otherwise.
         */
        [[nodiscard]] virtual bool can_apply_to(const routingblocks::Solution& sol) const = 0;

        virtual ~destroy_operator() = default;
    };

    /**
     * \brief Destory Operator interface
     */
    class repair_operator : public std::enable_shared_from_this<repair_operator> {
      public:
        /**
         * Applies the operator to the passed solution.
         */
        virtual void apply(routingblocks::Evaluation& evaluation, routingblocks::Solution& sol,
                           const std::vector<routingblocks::VertexID>& missing_vertices)
            = 0;

        [[nodiscard]] virtual std::string_view name() const = 0;

        /**
         * Returns true if the operator can be applied to the passed solution. False otherwise.
         */
        [[nodiscard]] virtual bool can_apply_to(const routingblocks::Solution& sol) const = 0;

        virtual ~repair_operator() = default;
    };

}  // namespace routingblocks

#endif  // routingblocks_OPERATORS_H
