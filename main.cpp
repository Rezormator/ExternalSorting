#include <iostream>
#include <chrono>
#include "ExternalSorting.h"
#include "Generator.h"
#include "FilesActions.h"

constexpr int FILE_SIZE_MB = 10;
constexpr int SORT_PART_SIZE_MB = 1;
constexpr int MIN_INT = 1;
constexpr int MAX_INT = 100;

int main() {
    const std::string inputFile = "../files/A.bin";
    const std::vector<std::string> subFilesB{"../files/B1.bin", "../files/B2.bin", "../files/B3.bin"};
    const std::vector<std::string> subFilesC{"../files/C1.bin", "../files/C2.bin", "../files/C3.bin"}; {
        std::ofstream file(inputFile, std::ios::binary | std::ios::trunc);
        file.close();
    }

    for (const auto &fileName: subFilesB) {
        std::ofstream file(fileName, std::ios::binary | std::ios::trunc);
        file.close();
    }

    for (const auto &fileName: subFilesC) {
        std::ofstream file(fileName, std::ios::binary | std::ios::trunc);
        file.close();
    }


    ExternalSorting externalSorting(inputFile, subFilesB, subFilesC);

    std::cout << "Generating..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    Generator::fileFileWithNumbers(inputFile, FILE_SIZE_MB, MIN_INT, MAX_INT);
    // std::vector numbers = {3, 7, 2, 5, 10, 6, 1, 8, 9, 4, 7, 5, 2, 6, 9, 3, 8, 1, 10, 4, 5, 7, 2, 6, 9, 3, 8, 10, 1, 4};
    // std::ofstream file(inputFile, std::ios::binary);
    // file.write(reinterpret_cast<const char *>(numbers.data()), numbers.size() * sizeof(int));
    // file.close();
    // 3 7 / 1 8 9 / 2 6 9 / 4 5 7 / 1 4
    // 2 5 10 / 4 7 / 3 8 / 2 6 9
    // 6 / 5 / 1 10 / 3 8 10

    // 2 3 5 6 7 10 / 2 3 4 5 6 7 8 9 10
    // 1 4 5 7 8 9 / 1 4
    // 1 2 3 6 8 9 10

    // 1 1 2 2 3 3 4 5 5 6 6 7 7 8 8 9 9 10 10
    // 1 2 3 4 4 5 6 7 8 9 10
    //

    // 1 1 1 2 2 2 3 3 3 4 4 4 5 5 5 6 6 6 7 7 7 8 8 8 9 9 9 10 10 10
    //
    //
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Operation took " << duration.count() << " seconds." << std::endl;

    // std::cout << "Sorting..." << std::endl;
    // start = std::chrono::high_resolution_clock::now();
    // FilesActions::sortFileBySection(inputFile, tempFile, SORT_PART_SIZE_MB);
    // end = std::chrono::high_resolution_clock::now();
    // duration = end - start;
    // std::cout << "Operation took " << duration.count() << " seconds." << std::endl;

    std::cout << "Mergin..." << std::endl;
    start = std::chrono::high_resolution_clock::now();
    externalSorting.mergin();
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Operation took " << duration.count() << " seconds." << std::endl;
}
