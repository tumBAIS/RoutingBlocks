#pragma once

#include <algorithm>
#include <ctime>
#include <exception>
#include <iterator>
#include <limits>
#include <stdexcept>

#include "sfmt/SFMT.h"

namespace vrpis::utility {
    class random {
      public:
        using result_type = size_t;

        constexpr static result_type min() { return std::numeric_limits<result_type>::min(); }

        constexpr static result_type max() { return std::numeric_limits<result_type>::max(); }

      private:
        sfmt_t _generator{};

      public:
        explicit random() { sfmt_init_gen_rand(&_generator, std::time(nullptr)); }

        explicit random(uint32_t seed) { sfmt_init_gen_rand(&_generator, seed); }

        template <typename numeric_type> numeric_type generate(numeric_type min, numeric_type max) {
            assert(min <= max);
            return static_cast<numeric_type>(min + (max - min) * sfmt_genrand_real2(&_generator));
        }

        template <typename numeric_type> numeric_type generate(numeric_type max) {
            return static_cast<numeric_type>(max * sfmt_genrand_real2(&_generator));
        }

        /**
         * @return Random double from [0, 1)
         */
        inline auto generate() { return sfmt_genrand_real2(&_generator); }

        /**
         * \brief Returns a random int equally distributed on [min,max]
         * undefined behaviour if min > max.
         */
        size_t generateInt(size_t min, size_t max) { return this->generate(min, max + 1); }

        result_type operator()() noexcept {
            if constexpr (std::is_same_v<size_t, uint64_t>) {
                return sfmt_genrand_uint64(&_generator);
            } else {
                return sfmt_genrand_uint32(&_generator);
            }
        }

        /**
         * \brief Returns a random double equally distributed on [0,1)
         */
        double generateDouble() { return sfmt_genrand_real2(&_generator); }

        /**
         * \brief Returns a random double equally distributed on [min, max)
         */
        double uniform(double min, double max) {
            assert(min <= max);
            return min + (max - min) * sfmt_genrand_real2(&_generator);
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

            double picked = uniform(0, aggregated_weights);
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

        /**
         * \brief Returns
         */
        template <class ForwardIterator, class weight_type>
        auto roulette(ForwardIterator beg, ForwardIterator end,
                      weight_type ForwardIterator::value_type::*weight_member,
                      bool higher_better = true, weight_type reference_point = weight_type())
            -> decltype(*beg)& {
            return *roulette_iter(beg, end, weight_member, higher_better, reference_point);
        }

        /**
         * \brief Returns a reference to a random element in [beg, end)
         * \param beg A RandomAccessIterator specifing the begin of the sequence
         * \param end A RandomAccessIterator pointing to the end of the sequence
         */
        template <class RandomAccessIterator>
        typename RandomAccessIterator::value_type& pick(RandomAccessIterator beg,
                                                        RandomAccessIterator end) {
            return *(beg + generateDouble() * (end - beg));
        }
    };
}  // namespace vrpis::utility
