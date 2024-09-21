#include "ExternalSorting.h"
#include <algorithm>
#include <utility>
#include "FilesActions.h"
#include <iostream>
#include <chrono>
#include <cmath>

constexpr int FILES_COUNT = 3;
constexpr int INT_SIZE = 4;
constexpr int EXTERNAL_SORT = 0;
constexpr int IMPRUVED_EXTERNAL_SORT = 1;
constexpr int INTS_IN_MB = 262144;

ExternalSorting::ExternalSorting(std::string inputFile, strings subFilesB, strings subFilesC, const int sortType,
                                 const int fileSize)
    : inputFile(std::move(inputFile)), subFilesB(std::move(subFilesB)), subFilesC(std::move(subFilesC)),
      sortType(sortType) {
    filesToRead = 0;
    filesToWrite = 1;
    readStep = std::ceil(fileSize / 5 * INTS_IN_MB);
}

std::string ExternalSorting::mergin() {
    std::vector subFiles{FilesActions::openAllFiles(subFilesB), FilesActions::openAllFiles(subFilesC)};

    switch (sortType) {
        case EXTERNAL_SORT:
        default:
            mergingInputFile(subFiles[filesToRead]);
        case IMPRUVED_EXTERNAL_SORT:
            impruvedMergingInputFile(subFiles[filesToRead]);
    }

    const std::vector subFilesNames = {subFilesB, subFilesC};
    while (!isSorted(subFiles[filesToRead])) {
        mergingSubFiles(subFiles[filesToRead], subFiles[filesToWrite]);
        FilesActions::clearFiles(subFiles[filesToRead], subFilesNames[filesToRead]);
        switchFiles();
    }
    for (const auto &subFilesGroup: subFiles)
        FilesActions::closeAllFiles(subFilesGroup);

    return subFilesNames[filesToRead][0];
}

void ExternalSorting::mergingInputFile(const fstreams &subFiles) const {
    std::ifstream file(inputFile, std::ios::binary);
    ints previous(FILES_COUNT, 0);
    bool finishedFile = false;
    int fileNumber = 0;

    do {
        ints input(readStep);
        if (!file.read(reinterpret_cast<char *>(input.data()), readStep * sizeof(int)))
            input.resize(file.gcount() / sizeof(int));

        if (input.empty()) {
            finishedFile = true;
            continue;
        }

        intsVector output(FILES_COUNT);
        bool finishedVector = false;
        int position = 0;
        do {
            if (previous[fileNumber] > input[position])
                fileNumber++;

            if (fileNumber == FILES_COUNT)
                fileNumber = 0;

            do {
                output[fileNumber].push_back(input[position]);
                previous[fileNumber] = input[position++];
                if (position >= input.size())
                    finishedVector = true;
            } while (previous[fileNumber] <= input[position] && !finishedVector);
        } while (!finishedVector);

        for (int i = 0; i < FILES_COUNT; i++)
            subFiles[i]->write(reinterpret_cast<const char *>(output[i].data()), output[i].size() * sizeof(int));
    } while (!finishedFile);

    file.close();
}

void ExternalSorting::impruvedMergingInputFile(const fstreams &subFiles) const {
    std::ifstream file(inputFile, std::ios::binary);
    ints previous(FILES_COUNT, 0);
    bool finishedFile = false;
    int fileNumber = 0;

    do {
        ints numbers(readStep);
        if (!file.read(reinterpret_cast<char *>(numbers.data()), readStep * sizeof(int)))
            numbers.resize(file.gcount() / sizeof(int));

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

void ExternalSorting::mergingSubFiles(const fstreams &filesToRead, const fstreams &filesToWrite) const {
    bools finishedFile(FILES_COUNT, false);
    bools finishedVector(FILES_COUNT, true);
    intsVector input(FILES_COUNT);
    intsVector output(FILES_COUNT);
    ints timesRead(FILES_COUNT, 0);
    ints current(FILES_COUNT, 0);
    ints previous(FILES_COUNT, 0);
    ints positions(FILES_COUNT, 0);
    bools continueWrite(FILES_COUNT, true);
    bools hasValue(FILES_COUNT, false);
    int fileNumber = 0;

    do {
        for (int i = 0; i < FILES_COUNT; i++) {
            if (!finishedVector[i])
                continue;

            positions[i] = 0;
            input[i].resize(readStep);
            filesToRead[i]->seekg(timesRead[i]++ * readStep * INT_SIZE, std::ios::beg);
            if (!filesToRead[i]->read(reinterpret_cast<char *>(input[i].data()), readStep * sizeof(int)))
                input[i].resize(filesToRead[i]->gcount() / sizeof(int));

            if (input[i].empty()) {
                finishedFile[i] = true;
                continueWrite[i] = false;
            } else {
                finishedVector[i] = false;
            }
        }

        if (std::ranges::all_of(finishedFile, [](const bool value) { return value; }))
            break;

        for (int i = 0; i < FILES_COUNT; i++) {
            if (finishedFile[i] || hasValue[i])
                continue;

            hasValue[i] = true;
            current[i] = input[i][positions[i]++];
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
                output[fileNumber].push_back(current[minIndex]);
                previous[minIndex] = current[minIndex];
                current[minIndex] = input[minIndex][positions[minIndex]];

                if (positions[minIndex] == input[minIndex].size()) {
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
            filesToWrite[i]->write(reinterpret_cast<const char *>(output[i].data()), output[i].size() * sizeof(int));
            output[i].clear();
        }
    } while (std::ranges::any_of(finishedFile, [](const bool value) { return !value; }));
}

bool ExternalSorting::continueMerging(const bools &finishedVector, const bools &finishedFile) {
    for (int i = 0; i < FILES_COUNT; i++) {
        if ((finishedVector[i] && finishedFile[i]) || !finishedVector[i])
            continue;
        return false;
    }
    return true;
}

bool ExternalSorting::increasFileNumber(const ints &current, const ints &previous) {
    for (int i = 0; i < FILES_COUNT; i++)
        if (current[i] > previous[i])
            return false;
    return true;
}

void ExternalSorting::switchFiles() {
    std::swap(filesToRead, filesToWrite);
}

int ExternalSorting::minNumIndex(const ints &numbers, const bools &condition) {
    int minIndex = -1;
    for (int i = 0; i < numbers.size(); i++)
        if (condition[i] == true)
            if (minIndex == -1 || numbers[i] < numbers[minIndex])
                minIndex = i;
    return minIndex;
}

bool ExternalSorting::isSorted(const fstreams &files) {
    return !FilesActions::isFileEmpty(files[0])
           && FilesActions::isFileEmpty(files[1])
           && FilesActions::isFileEmpty(files[2]);
}

bool ExternalSorting::checkSorted(const std::string &fileName) {
    std::ifstream file(fileName, std::ios::binary);
    unsigned int prevNum{0}, curNum;
    bool isSorted{true};
    while (file.read(reinterpret_cast<char *>(&curNum), sizeof(curNum))) {
        if (curNum < prevNum) {
            isSorted = false;
            break;
        }
        prevNum = curNum;
    }
    file.close();
    return isSorted;
}
