//
// Created by patrick on 3/10/23.
//

#ifndef vrpis_ALGORITHMS_H
#define vrpis_ALGORITHMS_H

namespace vrpis::utility {

    template <typename T>
    void apply_permutation(std::vector<T>& vec, const std::vector<size_t>& permutation) {
        std::vector<bool> done(vec.size());
        for (size_t i = 0; i < vec.size(); ++i) {
            if (done[i]) {
                continue;
            }
            done[i] = true;
            size_t prev_j = i;
            size_t j = permutation[i];
            while (i != j) {
                std::swap(vec[prev_j], vec[j]);
                done[j] = true;
                prev_j = j;
                j = permutation[j];
            }
        }
    }

}  // namespace vrpis::utility

#endif  // vrpis_ALGORITHMS_H
