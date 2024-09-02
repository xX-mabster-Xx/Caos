#include <cstdlib>
#include <future>
#include <iomanip>
#include <iostream>
#include <vector>

const int POW_2_16 = 65536;
const int POW_2_15 = 32768;

int MonteCarloMethod(unsigned int seed, int iteratoins) {
    int in = 0;
    for (int i = 0; i < iteratoins; ++i) {
        int x = rand_r(&seed) % POW_2_16;
        int y = rand_r(&seed) % POW_2_16;
        if ((x - POW_2_15) * (x - POW_2_15) + (y - POW_2_15) * (y - POW_2_15) <=
            POW_2_15 * POW_2_15) {
            in++;
        }
    }
    return in;
}

int main(int argc, char *argv[]) {
    int thr_count = std::atoi(argv[1]);
    int iter_count = std::atoi(argv[2]);
    std::vector<std::future<int>> futures;
    futures.reserve(iter_count);
    for (int i = 0; i < thr_count; ++i) {
        futures.push_back(std::async(MonteCarloMethod, i, iter_count));
    }
    long long total_in = 0;
    long long total = static_cast<long long>(iter_count) * thr_count;
    for (int i = 0; i < thr_count; ++i) {
        total_in += futures[i].get();
    }
    total_in *= 4;
    std::cout << std::setprecision(5)
              << static_cast<double>(total_in) / static_cast<double>(total)
              << "\n";
}