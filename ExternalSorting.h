#pragma once

#include <string>
#include <vector>
#include <fstream>

typedef std::vector<std::string> strings;
typedef std::vector<bool> bools;
typedef std::vector<std::fstream *> fstreams;
typedef std::vector<int> ints;
typedef std::vector<std::vector<int>> intsVector;

class ExternalSorting {
private:
    const std::string inputFile;
    const strings subFilesB;
    const strings subFilesC;
    int filesToRead;
    int filesToWrite;
    int sortType;
    unsigned long long readStep;
    unsigned long long readStepForSubFiles;

    void switchFiles();
    void mergingInputFile(const fstreams &subFiles) const;
    void impruvedMergingInputFile(const fstreams &subFiles) const;
    void mergingSubFiles(const fstreams &filesToRead, const fstreams &filesToWrite) const;
    static bool continueMerging(const bools &finishedVector, const bools &finishedFile);
    static bool increasFileNumber(const ints &current, const ints &previous);
    static bool isSorted(const fstreams &files);
    static int minNumIndex(const ints &numbers, const bools &condition);
public:
    ExternalSorting(std::string inputFile, strings subFilesB, strings subFilesC, int sortType, int fileSize);
    std::string mergin();
    static bool checkSorted(const std::string &fileName);
    static bool limitMemory(int limit);
};
