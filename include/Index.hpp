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

    void add(const std::filesystem::path& path) const;
    void remove(const std::filesystem::path& path) const;
    void reset() const;

    std::vector<std::string> getStagedFilesList() const;
    std::vector<IndexEntry> getStagedFilesListWithDetails() const;

private:
    const Repository& repo;

    static std::string getFileMode(const std::filesystem::path& absolutePath);
    void addFileToIndex(const std::filesystem::path& relativePath, const std::filesystem::path& absolutePath) const;
    void removeFileFromIndex(const std::filesystem::path& relativePath) const;
};

} // namespace CppGit
