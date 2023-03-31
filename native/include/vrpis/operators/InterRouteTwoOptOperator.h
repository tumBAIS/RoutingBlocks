
#ifndef vrpis_INTERROUTETWOOPT_H
#define vrpis_INTERROUTETWOOPT_H

#include "vrpis/Instance.h"
#include "vrpis/LocalSearch.h"
#include "vrpis/Solution.h"
#include "vrpis/interfaces/evaluation.h"

namespace vrpis {
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
}  // namespace vrpis

#endif  // vrpis_INTERROUTETWOOPT_H
