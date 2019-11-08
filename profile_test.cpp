#include <iostream>

#include "profile.hpp"

int main() {

//    void profile(std::string name, uint64_t count, uint64_t repetitions, const ProfileFn& fn, const AugmentorFn& augFn = nullptr)

    profile("mul", 1e6, 1, []() {
        double tmp=1.0;
        for(size_t i = 1; i < 1e6; ++i) {
            tmp = tmp*i;
        }
    });

    return 0;
}
