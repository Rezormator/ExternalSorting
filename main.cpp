#include <iostream>
#include <chrono>
#include "ExternalSorting.h"
#include "Generator.h"
#include "FilesActions.h"
#include "Input.h"

constexpr int FILE_SIZE_MB = 1024;
constexpr int MIN_INT = 1;
constexpr int MAX_INT = 100;
constexpr int EXIT = 2;

int main() {
    const std::string inputFile = "../files/A.bin";
    const strings subFilesB{"../files/B1.bin", "../files/B2.bin", "../files/B3.bin"};
    const strings subFilesC{"../files/C1.bin", "../files/C2.bin", "../files/C3.bin"};
    FilesActions::clearFiles();

    const int fileSize = Input::inputInt("Enter file size in MB: ");
    const int minInt = Input::inputInt("Enter min int: ");
    const int maxInt = Input::inputInt("Enter max int: ");
    const int sortType = Input::indexMenu({"External sort", "Impruved extrnal sort"});
    ExternalSorting externalSorting(inputFile, subFilesB, subFilesC, sortType, fileSize);

    std::cout << FILE_SIZE_MB << " MB " << "Generating..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    Generator::fileFileWithNumbers(inputFile, fileSize, minInt, maxInt);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Operation took " << duration.count() << " seconds." << std::endl;

    std::cout << FILE_SIZE_MB << " MB " << "Mergin..." << std::endl;
    start = std::chrono::high_resolution_clock::now();
    const std::string sortedFile = externalSorting.mergin();
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Operation took " << duration.count() << " seconds." << std::endl;

    std::cout << FILE_SIZE_MB << " MB " << "Checking..." << std::endl;
    start = std::chrono::high_resolution_clock::now();
    std::cout << (ExternalSorting::checkSorted(sortedFile) ? "Sorted" : "Unsorted") << std::endl;
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Operation took " << duration.count() << " seconds." << std::endl;

    int stop;
    std::cin >> stop;

    return 0;
}
