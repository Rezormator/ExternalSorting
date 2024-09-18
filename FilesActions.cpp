#include "FilesActions.h"
#include <iostream>
#include <algorithm>

constexpr int INTS_IN_MB = 262144;

std::vector<std::fstream *> FilesActions::openAllFiles(const std::vector<std::string> &filesNames) {
    std::vector<std::fstream *> files;
    for (const auto &filesName: filesNames) {
        auto *file = new std::fstream(filesName, std::ios::binary | std::ios::in | std::ios::out);
        files.push_back(file);
    }
    return files;
}

void FilesActions::closeAllFiles(const std::vector<std::fstream *> &files) {
    for (const auto &file: files) {
        file->close();
        delete file;
    }
}

bool FilesActions::isFileEmpty(std::fstream *file) {
    file->seekg(0, std::ios::end);
    return file->tellg() == 0;
}

void FilesActions::clearFiles(const std::vector<std::fstream *> &files, const std::vector<std::string> &filesNames) {
    for (int i = 0; i < files.size(); i++) {
        files[i]->close();
        files[i]->open(filesNames[i], std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);
    }
}

void FilesActions::sortFileBySection(const std::string &fileName, const std::string &fileCopyName,
                                     const int megabytes) {
    std::ifstream file(fileName, std::ios::binary);
    std::ofstream fileCopy(fileCopyName, std::ios::binary);

    bool finished = false;
    do {
        for (int i = 0; i < megabytes; i++) {
            std::vector<int> numbers;
            for (int j = 0; j < INTS_IN_MB; j++) {
                int number;
                if (!file.read(reinterpret_cast<char *>(&number), sizeof(int))) {
                    finished = true;
                    break;
                }
                numbers.push_back(number);
            }
            std::ranges::sort(numbers);
            fileCopy.write(reinterpret_cast<const char *>(numbers.data()), numbers.size() * sizeof(int));
        }
    } while (!finished);

    file.close();
    fileCopy.close();

    std::remove(fileName.c_str());
    std::rename(fileCopyName.c_str(), fileName.c_str());
}
