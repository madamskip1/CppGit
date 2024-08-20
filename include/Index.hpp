#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace CppGit {

struct IndexEntry
{
    std::string fileMode;
    std::string objectHash;
    int stageNumber;
    std::filesystem::path path;
};

class Repository; // Forward declaration

class Index
{
public:
    explicit Index(const Repository& repo);
    Index() = delete;

    auto add(const std::filesystem::path& path) const -> void;
    auto remove(const std::filesystem::path& path) const -> void;
    auto reset() const -> void;

    auto isFileStaged(const std::filesystem::path& path) const -> bool;

    auto getStagedFilesList() const -> std::vector<std::string>;
    auto getStagedFilesListWithDetails() const -> std::vector<IndexEntry>;

private:
    const Repository& repo;

    static auto getFileMode(const std::filesystem::path& absolutePath) -> std::string;
    auto addFileToIndex(const std::filesystem::path& relativePath, const std::filesystem::path& absolutePath) const -> void;
    auto removeFileFromIndex(const std::filesystem::path& relativePath) const -> void;
};

} // namespace CppGit
