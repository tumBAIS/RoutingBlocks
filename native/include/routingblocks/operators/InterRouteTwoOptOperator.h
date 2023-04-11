
#ifndef routingblocks_INTERROUTETWOOPT_H
#define routingblocks_INTERROUTETWOOPT_H

#include <routingblocks/Instance.h>
#include <routingblocks/LocalSearch.h>
#include <routingblocks/Solution.h>
#include <routingblocks/evaluation.h>

namespace routingblocks {
    class InterRouteTwoOptMove : public GeneratorArcMove<InterRouteTwoOptMove> {
      public:
        using GeneratorArcMove<InterRouteTwoOptMove>::GeneratorArcMove;

        void apply_to(const Instance& instance, Solution& solution) const;

        [[nodiscard]] cost_t evaluate(Evaluation& evaluation, const Instance& instance,
                                      const Solution& solution) const;
    };

    class InterRouteTwoOptOperator : public GeneratorArcOperator<InterRouteTwoOptMove> {
      public:
        using GeneratorArcOperator<InterRouteTwoOptMove>::GeneratorArcOperator;
    };
}  // namespace routingblocks

#endif  // routingblocks_INTERROUTETWOOPT_H
