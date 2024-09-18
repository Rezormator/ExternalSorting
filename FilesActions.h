#pragma once

#include <string>
#include <vector>
#include <fstream>

class FilesActions {
public:
    static std::vector<std::fstream *> openAllFiles(const std::vector<std::string> &filesNames);
    static void closeAllFiles(const std::vector<std::fstream *> &files);
    static bool isFileEmpty(std::fstream *file);
    static void clearFiles(const std::vector<std::fstream *> &files, const std::vector<std::string> &filesNames);
    static void sortFileBySection(const std::string &fileName, const std::string &fileCopyName, int megabytes);
};
