/*
 * Copyright (c) 2023 Patrick S. Klein (@libklein)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


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
    class destroy_operator {
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
    class repair_operator {
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
