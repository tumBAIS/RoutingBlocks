
#ifndef vrpis_OPERATORS_H
#define vrpis_OPERATORS_H

#include <memory>
#include <vector>

#include "vrpis/interfaces/evaluation.h"
#include "vrpis/interfaces/types.h"
#include "vrpis/interfaces/vertex.h"

namespace vrpis {

    class Solution;

    /**
     * \brief Destory Operator interface
     */
    class destroy_operator : public std::enable_shared_from_this<destroy_operator> {
      public:
        /**
         * Applies the operator to the passed solution.
         */
        virtual std::vector<vrpis::VertexID> apply(vrpis::Evaluation& evaluation,
                                                   vrpis::Solution& sol,
                                                   size_t numberOfRemovedCustomers)
            = 0;

        [[nodiscard]] virtual std::string_view name() const = 0;

        /**
         * Returns true if the operator can be applied to the passed solution. False otherwise.
         */
        [[nodiscard]] virtual bool can_apply_to(const vrpis::Solution& sol) const = 0;

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
        virtual void apply(vrpis::Evaluation& evaluation, vrpis::Solution& sol,
                           const std::vector<vrpis::VertexID>& missing_vertices)
            = 0;

        [[nodiscard]] virtual std::string_view name() const = 0;

        /**
         * Returns true if the operator can be applied to the passed solution. False otherwise.
         */
        [[nodiscard]] virtual bool can_apply_to(const vrpis::Solution& sol) const = 0;

        virtual ~repair_operator() = default;
    };

}  // namespace vrpis

#endif  // vrpis_OPERATORS_H
