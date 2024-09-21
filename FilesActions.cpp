#include "FilesActions.h"
#include <iostream>

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

void FilesActions::clearFiles() {
    const std::vector<std::string> allFiles = {
        "../files/A.bin", "../files/B1.bin", "../files/B2.bin",
        "../files/B3.bin", "../files/C1.bin", "../files/C2.bin", "../files/C3.bin"
    };

    for (const auto &fileName: allFiles) {
        std::ofstream file(fileName, std::ios::binary | std::ios::trunc);
        file.close();
    }
}
