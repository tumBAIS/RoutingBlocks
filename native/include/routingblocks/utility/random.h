#pragma once

#include <xoshiro/xoshiro.h>

#include <algorithm>
#include <concepts>
#include <ctime>
#include <limits>
#include <random>

namespace routingblocks::utility {
    class random {
      public:
        using result_type = size_t;

        constexpr static result_type min() { return std::numeric_limits<result_type>::min(); }

        constexpr static result_type max() { return std::numeric_limits<result_type>::max(); }

      private:
        using generator_ = XoshiroCpp::Xoshiro256PlusPlus;
        generator_ _generator;

      public:
        explicit random() : _generator(time(nullptr)) {}

        explicit random(uint64_t seed) : _generator(seed) {}

        /**
         * \brief Returns a random int equally distributed on [min,max]
         * undefined behaviour if min > max.
         */
        template <typename T>
            requires std::integral<T>
        T generateInt(T min, T max) {
            return std::uniform_int_distribution<T>(min, max)(_generator);
        }

        result_type operator()() noexcept { return _generator(); }

        /**
         * \brief Returns a random double equally distributed on [min, max)
         */
        template <class T>
            requires std::floating_point<T>
        T uniform(T min, T max) {
            return std::uniform_real_distribution<T>(min, max)(_generator);
        }

        /**
         * \brief Randomly selects an iterator from the given range
         */
        template <class ForwardIterator>
        ForwardIterator choose(ForwardIterator seq_beg, ForwardIterator seq_end) {
            assert(std::distance(seq_beg, seq_end) > 0);
            ForwardIterator ret = seq_beg;
            std::advance(ret, generateInt(0, std::distance(seq_beg, seq_end) - 1));
            return ret;
        }

        /**
         * \brief Returns
         */
        template <class ForwardIterator, class weight_type>
        ForwardIterator roulette_iter(ForwardIterator beg, ForwardIterator end,
                                      weight_type ForwardIterator::value_type::*weight_member,
                                      bool higher_better = true,
                                      weight_type reference_point = weight_type(0)) {
            // sum up all weights
            double aggregated_weights = 0.0;
            for (ForwardIterator next = beg; next != end; ++next) {
                double dbl_weight = static_cast<double>((*next).*weight_member - reference_point);
                assert(dbl_weight >= 0);
                if (dbl_weight == 0) {
                    continue;
                }
                aggregated_weights += higher_better ? dbl_weight : (1 / dbl_weight);
            }

            double picked = uniform(0., aggregated_weights);
            double current_weight = 0.0;
            for (ForwardIterator next = beg; next != end; ++next) {
                double dbl_weight = static_cast<double>((*next).*weight_member - reference_point);
                if (dbl_weight == 0) {
                    continue;
                }
                if (current_weight < picked
                    && current_weight + (higher_better ? dbl_weight : (1 / dbl_weight))
                           >= picked) {  // This is guaranteed to be true eventually
                    return next;
                }
                current_weight += higher_better ? dbl_weight : (1 / dbl_weight);
            }

            if (aggregated_weights == 0.0) {  // All options are equally good
                return (beg + generateInt(0, std::distance(beg, end) - 1));
            }

            throw std::logic_error("Reached end of roulette pick");
        }
    };
}  // namespace routingblocks::utility
