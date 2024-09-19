#pragma once

#include <string>
#include <vector>
#include <fstream>

class ExternalSorting {
private:
    const std::string inputFile;
    const std::vector<std::string> subFilesB;
    const std::vector<std::string> subFilesC;
    int filesToRead;
    int filesToWrite;

    void switchFiles();
    void mergingInputFile(const std::vector<std::fstream *> &subFiles) const;
    static void mergingSubFiles(const std::vector<std::fstream *> &filesToRead,
                                const std::vector<std::fstream *> &filesToWrite);
    static int minNumIndex(const std::vector<int> &numbers, const std::vector<bool> &condition);
    static bool isSorted(const std::vector<std::fstream *> &files);
public:
    ExternalSorting(std::string inputFile, const std::vector<std::string> &subFilesB,
                    const std::vector<std::string> &subFilesC);
    void mergin();
};
