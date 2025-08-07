
#include "utils.h"

int random_int(std::mt19937& gen, int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}