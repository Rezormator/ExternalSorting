#include "Generator.h"
#include <random>
#include <fstream>

constexpr int INT_IN_MB = 262144;

void Generator::fileFileWithNumbers(const std::string &fileName, const int megabytes, const int min, const int max) {
    std::vector<int> buffer(INT_IN_MB * megabytes);
    std::random_device rd;
    std::mt19937 gen(rd());

    std::ofstream file(fileName, std::ios::binary);
    for (auto &number : buffer) {
        std::uniform_int_distribution<> distr(min, max);
        number = distr(gen);
    }
    file.write(reinterpret_cast<const char*>(buffer.data()), INT_IN_MB * megabytes * sizeof(int));
    file.close();
}