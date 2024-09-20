#include "ExternalSorting.h"
#include <algorithm>
#include <utility>
#include "FilesActions.h"
#include <iostream>
#include <chrono>

constexpr int FILES_COUNT = 3;
constexpr int INT_SIZE = 4;
constexpr int INTS_IN_MB = 2621440;
// constexpr int INTS_IN_MB = 3;

ExternalSorting::ExternalSorting(std::string inputFile, const std::vector<std::string> &subFilesB,
                                 const std::vector<std::string> &subFilesC)
    : inputFile(std::move(inputFile)), subFilesB(subFilesB), subFilesC(subFilesC) {
    filesToRead = 0;
    filesToWrite = 1;
}

bool checkSorted(const char *targetFile) {
    std::ifstream is{targetFile, std::ios::binary};
    unsigned int prevNum{0}, curNum;
    bool isSorted{true};
    while(is.read(reinterpret_cast<char *>(&curNum), sizeof(curNum))) {
        if(curNum < prevNum) {
            isSorted = false;
            break;
        }
        prevNum = curNum;
    }
    is.close();
    return isSorted;
}

void ExternalSorting::mergin() {
    std::vector subFiles{FilesActions::openAllFiles(subFilesB), FilesActions::openAllFiles(subFilesC)};
    const auto start = std::chrono::high_resolution_clock::now();
    impruvedMergingInputFile(subFiles[filesToRead]);
    std::cout << "Input file merged." << std::endl;
    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> duration = end - start;
    std::cout << "Operation took " << duration.count() << " seconds." << std::endl;

    const std::vector subFilesNames = {subFilesB, subFilesC};
    while (!isSorted(subFiles[filesToRead])) {
        mergingSubFiles(subFiles[filesToRead], subFiles[filesToWrite]);
        FilesActions::clearFiles(subFiles[filesToRead], subFilesNames[filesToRead]);
        switchFiles();
    }
    for (const auto &subFilesGroup: subFiles)
        FilesActions::closeAllFiles(subFilesGroup);

    std::cout << checkSorted(subFilesNames[filesToRead][0].c_str()) << std::endl;
}

void ExternalSorting::mergingInputFile(const std::vector<std::fstream *> &subFiles) const {
    std::ifstream file(inputFile, std::ios::binary);
    std::vector previous(FILES_COUNT, 0);
    bool finishedFile = false;
    int fileNumber = 0;

    do {
        std::vector<int> inputNumbers(INTS_IN_MB);
        if (!file.read(reinterpret_cast<char *>(inputNumbers.data()), INTS_IN_MB * sizeof(int))) {
            inputNumbers.resize(file.gcount() / sizeof(int));
        }

        if (inputNumbers.empty()) {
            finishedFile = true;
            continue;
        }

        std::vector<std::vector<int> > outputNumbers(FILES_COUNT);
        int position = 0;
        bool finishedVector = false;
        do {
            if (previous[fileNumber] > inputNumbers[position])
                fileNumber++;

            if (fileNumber == FILES_COUNT)
                fileNumber = 0;

            do {
                outputNumbers[fileNumber].push_back(inputNumbers[position]);
                previous[fileNumber] = inputNumbers[position++];
                if (position >= inputNumbers.size())
                    finishedVector = true;
            } while (previous[fileNumber] <= inputNumbers[position] && !finishedVector);
        } while (!finishedVector);

        for (int i = 0; i < FILES_COUNT; i++)
            subFiles[i]->write(reinterpret_cast<const char *>(outputNumbers[i].data()),
                               outputNumbers[i].size() * sizeof(int));
    } while (!finishedFile);

    file.close();
}

void ExternalSorting::impruvedMergingInputFile(const std::vector<std::fstream *> &subFiles) const {
    std::ifstream file(inputFile, std::ios::binary);
    std::vector previous(FILES_COUNT, 0);
    bool finishedFile = false;
    int fileNumber = 0;

    do {
        std::vector<int> numbers(INTS_IN_MB);
        if (!file.read(reinterpret_cast<char *>(numbers.data()), INTS_IN_MB * sizeof(int))) {
            numbers.resize(file.gcount() / sizeof(int));
        }

        if (numbers.empty()) {
            finishedFile = true;
            continue;
        }

        std::ranges::sort(numbers);

        if (fileNumber == FILES_COUNT)
            fileNumber = 0;

        subFiles[fileNumber]->write(reinterpret_cast<const char *>(numbers.data()), numbers.size() * sizeof(int));

        fileNumber++;
    } while (!finishedFile);

    file.close();
}

void ExternalSorting::mergingSubFiles(const std::vector<std::fstream *> &filesToRead,
                                      const std::vector<std::fstream *> &filesToWrite) {
    std::vector finishedFile(FILES_COUNT, false);
    std::vector finishedVector(FILES_COUNT, true);
    std::vector<std::vector<int> > inputNumbers(FILES_COUNT);
    std::vector<std::vector<int> > outputNumbers(FILES_COUNT);
    std::vector timesRead(FILES_COUNT, 0);
    std::vector current(FILES_COUNT, 0);
    std::vector previous(FILES_COUNT, 0);
    std::vector positions(FILES_COUNT, 0);
    std::vector continueWrite(FILES_COUNT, true);
    std::vector hasValue(FILES_COUNT, false);
    int fileNumber = 0;

    do {
        for (int i = 0; i < FILES_COUNT; i++) {
            if (!finishedVector[i])
                continue;

            positions[i] = 0;
            inputNumbers[i].resize(INTS_IN_MB);
            filesToRead[i]->seekg(timesRead[i]++ * INTS_IN_MB * INT_SIZE, std::ios::beg);
            if (!filesToRead[i]->read(reinterpret_cast<char *>(inputNumbers[i].data()), INTS_IN_MB * sizeof(int)))
                inputNumbers[i].resize(filesToRead[i]->gcount() / sizeof(int));

            if (inputNumbers[i].empty()) {
                finishedFile[i] = true;
                continueWrite[i] = false;
            }
            else {
                finishedVector[i] = false;
            }
        }

        if (std::ranges::all_of(finishedFile, [](const bool value) { return value; }))
            break;

        for (int i = 0; i < FILES_COUNT; i++) {
            if (finishedFile[i] || hasValue[i])
                continue;

            current[i] = inputNumbers[i][positions[i]++];
            hasValue[i] = true;
            if (current[i] < previous[i])
                continueWrite[i] = false;
        }

        do {
            if (increasFileNumber(current, previous)) {
                for (int i = 0; i < FILES_COUNT; i++)
                    if (!finishedFile[i])
                        continueWrite[i] = true;
                fileNumber = fileNumber + 1 == FILES_COUNT ? 0 : fileNumber + 1;
            }

            do {
                const int minIndex = minNumIndex(current, continueWrite);
                outputNumbers[fileNumber].push_back(current[minIndex]);
                previous[minIndex] = current[minIndex];
                current[minIndex] = inputNumbers[minIndex][positions[minIndex]];

                if (positions[minIndex] == inputNumbers[minIndex].size()) {
                    finishedVector[minIndex] = true;
                    hasValue[minIndex] = false;
                    continue;
                }

                if (previous[minIndex] > current[minIndex])
                    continueWrite[minIndex] = false;

                positions[minIndex]++;
            } while (std::ranges::any_of(continueWrite, [](const bool value) { return value; })
                     && continueMerging(finishedVector, finishedFile));
        } while (continueMerging(finishedVector, finishedFile));

        for (int i = 0; i < FILES_COUNT; i++) {
            filesToWrite[i]->write(reinterpret_cast<const char *>(outputNumbers[i].data()),outputNumbers[i].size() * sizeof(int));
            outputNumbers[i].clear();
        }
    } while (std::ranges::any_of(finishedFile, [](const bool value) { return !value; }));
}

bool ExternalSorting::continueMerging(const std::vector<bool> &finishedVector, const std::vector<bool> &finishedFile) {
    for (int i = 0; i < FILES_COUNT; i++) {
        if ((finishedVector[i] && finishedFile[i]) || !finishedVector[i])
            continue;
        return false;
    }
    return true;
}

bool ExternalSorting::increasFileNumber(const std::vector<int> &current, const std::vector<int> &previous) {
    for (int i = 0; i < FILES_COUNT; i++)
        if (current[i] > previous[i])
            return false;
    return true;
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