#include "ExternalSorting.h"
#include <algorithm>
#include <utility>
#include "FilesActions.h"
#include <iostream>

constexpr int FILES_COUNT = 6;
constexpr int INT_SIZE = 4;

ExternalSorting::ExternalSorting(std::string inputFile, const std::vector<std::string> &subFilesB,
                                 const std::vector<std::string> &subFilesC)
    : inputFile(std::move(inputFile)), subFilesB(subFilesB), subFilesC(subFilesC) {
    filesToRead = 0;
    filesToWrite = 1;
}

void ExternalSorting::mergin() {
    mergingInputFile();
    std::cout << "Input file merged." << std::endl;

    const std::vector subFilesNames = {subFilesB, subFilesC};
    std::vector subFiles{FilesActions::openAllFiles(subFilesB), FilesActions::openAllFiles(subFilesC)};
    while (!isSorted(subFiles[filesToRead])) {
        mergingSubFiles(subFiles[filesToRead], subFiles[filesToWrite]);
        FilesActions::clearFiles(subFiles[filesToRead], subFilesNames[filesToRead]);
        switchFiles();
    }
    for (const auto &subFilesGroup: subFiles)
        FilesActions::closeAllFiles(subFilesGroup);

    // std::ifstream file(subFilesNames[filesToRead][0], std::ios::binary);
    // for (int i = 0; i < 1000; i++){
    //     int number;
    //     file.read(reinterpret_cast<char*>(&number), sizeof(int));
    //     std::cout << number << ' ';
    // }
    // file.close();
    // std::cout << std::endl;
}

void ExternalSorting::mergingInputFile() const {
    int position = 0;
    int number, previousNumber;
    std::ifstream file(inputFile, std::ios::binary);
    const std::vector<std::fstream *> subFiles = FilesActions::openAllFiles(subFilesB);

    bool finished = false;
    while (!finished) {
        for (int i = 0; i < FILES_COUNT; i++) {
            file.seekg(position * INT_SIZE, std::ios::beg);
            file.read(reinterpret_cast<char *>(&number), sizeof(int));

            do {
                subFiles[i]->write(reinterpret_cast<const char *>(&number), sizeof(int));
                position++;
                previousNumber = number;
                if (!file.read(reinterpret_cast<char *>(&number), sizeof(int)))
                    finished = true;
            } while (previousNumber <= number && !finished);

            if (finished)
                break;
        }
    }

    file.close();
    FilesActions::closeAllFiles(subFiles);
}

void ExternalSorting::mergingSubFiles(const std::vector<std::fstream *> &filesToRead,
                                      const std::vector<std::fstream *> &filesToWrite) {
    std::vector positions(FILES_COUNT, 0);
    std::vector finushed(FILES_COUNT, false);

    for (int i = 0; i < FILES_COUNT; i++)
        if (FilesActions::isFileEmpty(filesToRead[i]))
            finushed[i] = true;

    do {
        for (int i = 0; i < FILES_COUNT; i++) {
            if (std::ranges::all_of(finushed, [](const bool value) { return value; }))
                break;

            std::vector numbers(FILES_COUNT, 0);
            std::vector previousNumbers(FILES_COUNT, 0);
            std::vector continueProcessing(FILES_COUNT, false);

            for (int j = 0; j < FILES_COUNT; j++) {
                filesToRead[j]->seekg(positions[j] * INT_SIZE, std::ios::beg);
                if (finushed[j] || filesToRead[j]->eof())
                    continue;

                filesToRead[j]->read(reinterpret_cast<char *>(&numbers[j]), sizeof(int));
                positions[j]++;
                continueProcessing[j] = true;
            }

            do {
                const auto minIndex = minNumIndex(numbers, continueProcessing);
                filesToWrite[i]->write(reinterpret_cast<const char *>(&numbers[minIndex]), sizeof(int));

                continueProcessing[minIndex] = false;
                previousNumbers[minIndex] = numbers[minIndex];
                filesToRead[minIndex]->seekg(positions[minIndex] * INT_SIZE, std::ios::beg);
                if (!filesToRead[minIndex]->read(reinterpret_cast<char *>(&numbers[minIndex]), sizeof(int))) {
                    finushed[minIndex] = true;
                    continue;
                }
                if (numbers[minIndex] < previousNumbers[minIndex]) {
                    continue;
                }
                positions[minIndex]++;
                continueProcessing[minIndex] = true;
            } while (std::ranges::any_of(continueProcessing, [](const bool value) { return value; }));
        }
    } while (std::ranges::any_of(finushed, [](const bool value) { return !value; }));
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