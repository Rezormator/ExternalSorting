#include "ExternalSorting.h"
#include <algorithm>
#include <utility>
#include "FilesActions.h"
#include <iostream>
#include <chrono>

constexpr int FILES_COUNT = 3;
constexpr int INT_SIZE = 4;
constexpr int INTS_IN_MB = 262144;

ExternalSorting::ExternalSorting(std::string inputFile, const std::vector<std::string> &subFilesB,
                                 const std::vector<std::string> &subFilesC)
    : inputFile(std::move(inputFile)), subFilesB(subFilesB), subFilesC(subFilesC) {
    filesToRead = 0;
    filesToWrite = 1;
}

void ExternalSorting::mergin() {
    std::vector subFiles{FilesActions::openAllFiles(subFilesB), FilesActions::openAllFiles(subFilesC)};
    const auto start = std::chrono::high_resolution_clock::now();
    mergingInputFile(subFiles[filesToRead]);
    std::cout << "Input file merged." << std::endl;
    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> duration = end - start;
    std::cout << "Operation took " << duration.count() << " seconds." << std::endl;

    const std::vector subFilesNames = {subFilesB, subFilesC};
    while (!isSorted(subFiles[filesToRead])) {
        mergingSubFiles(subFiles[filesToRead], subFiles[filesToWrite]);
        FilesActions::clearFiles(subFiles[filesToRead], subFilesNames[filesToRead]);
        switchFiles();
        std::cout << "iteration" << std::endl;
    }
    for (const auto &subFilesGroup: subFiles)
        FilesActions::closeAllFiles(subFilesGroup);

    std::ifstream file(subFilesNames[filesToRead][0], std::ios::binary);
    for (int i = 0; i < 30; i++) {
        int number;
        file.read(reinterpret_cast<char *>(&number), sizeof(int));
        std::cout << number << ' ';
    }
    file.close();
    std::cout << std::endl;
}

void ExternalSorting::mergingInputFile(const std::vector<std::fstream *> &subFiles) const {
    std::ifstream file(inputFile, std::ios::binary);
    std::vector previousNumber(FILES_COUNT, 0);

    bool finishedReading = false;
    int fileNumber = 0;
    while (!finishedReading) {
        std::vector<int> inputNumbers(INTS_IN_MB);
        if (!file.read(reinterpret_cast<char *>(inputNumbers.data()), INTS_IN_MB * sizeof(int))) {
            inputNumbers.resize(file.gcount() / sizeof(int));
        }

        if (inputNumbers.empty()) {
            finishedReading = true;
            continue;
        }

        std::vector<std::vector<int> > numbers(FILES_COUNT);
        int position = 0;
        bool finished = false;
        while (!finished) {
            for (; fileNumber < FILES_COUNT; fileNumber++) {
                do {
                    numbers[fileNumber].push_back(inputNumbers[position]);
                    previousNumber[fileNumber] = inputNumbers[position++];
                    if (position >= inputNumbers.size())
                        finished = true;
                } while (previousNumber[fileNumber] <= inputNumbers[position] && !finished);

                if (fileNumber == FILES_COUNT - 1)
                    fileNumber = -1;

                if (finished)
                    break;
            }
        }
        fileNumber++;

        for (int i = 0; i < FILES_COUNT; i++)
            subFiles[i]->write(reinterpret_cast<const char *>(numbers[i].data()), numbers[i].size() * sizeof(int));
    }

    // for (int i = 0; i < FILES_COUNT; i++) {
    //     subFiles[i]->seekg(0, std::ios::beg);
    //     std::cout << '#' << i + 1 << ' ';
    //     int number;
    //     while (subFiles[i]->read(reinterpret_cast<char *>(&number), sizeof(int)))
    //         std::cout << number << ' ';
    //     std::cout << std::endl;
    // }
    // for (int i = 0; i < FILES_COUNT; i++) {
    //     subFiles[i]->seekg(0, std::ios::beg);
    //     std::cout << '#' << i + 1 << ' ';
    //     int number;
    //     while (subFiles[i]->read(reinterpret_cast<char *>(&number), sizeof(int)))
    //         std::cout << number << ' ';
    //     std::cout << std::endl;
    // }
    file.close();
}

void ExternalSorting::mergingSubFiles(const std::vector<std::fstream *> &filesToRead,
                                      const std::vector<std::fstream *> &filesToWrite) {
    std::vector fileFinushed(FILES_COUNT, false);
    std::vector timesRead(FILES_COUNT, 0);
    int fileToWrite = 0;
    while (std::ranges::any_of(fileFinushed, [](const bool value) { return !value; })) {
        std::vector<std::vector<int> > inputFilesNumbers(FILES_COUNT);
        for (int i = 0; i < FILES_COUNT; i++) {
            filesToRead[i]->seekg(timesRead[i]++ * INTS_IN_MB * INT_SIZE, std::ios::beg);
            inputFilesNumbers[i].resize(INTS_IN_MB);
            if (!filesToRead[i]->read(reinterpret_cast<char *>(inputFilesNumbers[i].data()), INTS_IN_MB * sizeof(int)))
                inputFilesNumbers[i].resize(filesToRead[i]->gcount() / sizeof(int));
        }

        for (int i = 0; i < FILES_COUNT; i++)
            if (inputFilesNumbers[i].empty())
                fileFinushed[i] = true;

        if (std::ranges::all_of(fileFinushed, [](const bool value) { return value; }))
            break;

        std::vector numbers(FILES_COUNT, 0);
        std::vector previousNumbers(FILES_COUNT, 0);
        std::vector positions(FILES_COUNT, 0);
        std::vector continueProcessing(FILES_COUNT, false);
        std::vector finushed(FILES_COUNT, false);

        for (int i = 0; i < FILES_COUNT; i++) {
            if (fileFinushed[i]) {
                finushed[i] = true;
                continue;
            }

            numbers[i] = inputFilesNumbers[i][positions[i]++];
            continueProcessing[i] = true;
        }

        std::vector<std::vector<int> > outputFilesNumbers(FILES_COUNT);
        while (std::ranges::any_of(finushed, [](const bool value) { return !value; })) {
            for (; fileToWrite < FILES_COUNT; fileToWrite++) {
                while (std::ranges::any_of(continueProcessing, [](const bool value) { return value; })) {
                    const auto minIndex = minNumIndex(numbers, continueProcessing);
                    outputFilesNumbers[fileToWrite].push_back(numbers[minIndex]);

                    continueProcessing[minIndex] = false;
                    previousNumbers[minIndex] = numbers[minIndex];
                    numbers[minIndex] = inputFilesNumbers[minIndex][positions[minIndex]];

                    if (positions[minIndex] == inputFilesNumbers[minIndex].size()) {
                        finushed[minIndex] = true;
                        continue;
                    }

                    positions[minIndex]++;
                    if (previousNumbers[minIndex] > numbers[minIndex])
                        continue;

                    continueProcessing[minIndex] = true;
                }

                for (int i = 0; i < FILES_COUNT; i++)
                    if (!finushed[i] && !fileFinushed[i])
                        continueProcessing[i] = true;

                if (std::ranges::all_of(finushed, [](const bool value) { return value; }))
                    break;

                if (fileToWrite == FILES_COUNT - 1)
                    fileToWrite = -1;
            }
        }
        fileToWrite++;

        for (int i = 0; i < FILES_COUNT; i++)
            filesToWrite[i]->write(reinterpret_cast<const char *>(outputFilesNumbers[i].data()),
                                   outputFilesNumbers[i].size() * sizeof(int));
    }

    // std::ranges::for_each(filesToWrite, [](std::fstream *file){ file->seekg(0, std::ios::beg); });
    // for (int i = 0; i < FILES_COUNT; i++) {
    //     std::cout << '#' << i + 1 << ' ';
    //     int number;
    //     while (filesToWrite[i]->read(reinterpret_cast<char *>(&number), sizeof(int)))
    //         std::cout << number << ' ';
    //     std::cout << std::endl;
    // }
}

void ExternalSorting::switchFiles() {
    std::swap(filesToRead, filesToWrite);
}

int ExternalSorting::minNumIndex(const std::vector<int> &numbers, const std::vector<bool> &condition) {
    int minIndex = -1;
    for (int i = 0; i < numbers.size(); i++)
        if (condition[i] == true)
            if (minIndex == -1 || numbers[i] < numbers[minIndex])
                minIndex = i;
    return minIndex;
}

bool ExternalSorting::isSorted(const std::vector<std::fstream *> &files) {
    return !FilesActions::isFileEmpty(files[0])
           && FilesActions::isFileEmpty(files[1])
           && FilesActions::isFileEmpty(files[2]);
}
