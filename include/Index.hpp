#pragma once

#include "Repository.hpp"

#include <filesystem>

namespace CppGit {

struct IndexEntry
{
    std::string fileMode;
    std::string objectHash;
    int stageNumber;
    std::filesystem::path path;
};


class Index
{
public:
    explicit Index(const Repository& repo);
    Index() = delete;

    void add(const std::filesystem::path& path);

    std::vector<std::string> getStagedFilesList() const;
    std::vector<IndexEntry> getStagedFilesListWithDetails() const;

private:
    const Repository& repo;

    static std::string getFileMode(const std::filesystem::path& path);
    void addFileToIndex(const std::filesystem::path& path);
};

} // namespace CppGit
