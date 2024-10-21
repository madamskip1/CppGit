#pragma once
#include "Repository.hpp"

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

enum class DiffIndexStatus
{
    ADDED,        // A
    DELETED,      // D
    MODIFIED,     // M
    RENAMED,      // R
    COPIED,       // C
    TYPE_CHANGED, // T
    UNMERGED,     // U
    UNKNOWN       // X
};

struct DiffIndexEntry
{
    std::string path;
    DiffIndexStatus status;
};

enum class LsFilesStatus
{
    TRACKED_NOT_UNMERGED_NOT_SKIP_WORKTREE, // H
    TRACKED_SKIP_WORKTREE,                  // S
    TRACKED_UNMERGED,                       // M
    TRACKED_DELETED,                        // R
    TRACKED_MODIFIED,                       // C
    UNTRACKED_CONFLICTING,                  // K
    UNTRACKED,                              // ?
    RESOLVE_UNDO                            // U
};

struct LsFilesEntry
{
    std::string path;
    LsFilesStatus status;
};

class Index
{
public:
    explicit Index(const Repository& repo);
    Index() = delete;

    auto add(const std::string_view filePattern) const -> void;
    auto remove(const std::string_view filePattern, bool force = false) const -> void;

    template <typename... Args>
    auto restoreStaged(Args&&... args) const -> void
    {
        auto output = repo.executeGitCommand("restore", std::forward<Args>(args)...);

        if (output.return_code != 0)
        {
            throw std::runtime_error("Failed to restore file");
        }
    }
    auto restoreAllStaged() const -> void;

    auto isFileStaged(const std::string_view file) const -> bool;

    auto getFilesInIndexList(const std::string_view filePattern = "") const -> std::vector<std::string>;
    auto getFilesInIndexListWithDetails(const std::string_view filePattern = "") const -> std::vector<IndexEntry>;

    auto getUntrackedFilesList(const std::string_view filePattern = "") const -> std::vector<std::string>;
    auto getStagedFilesList(const std::string_view filePattern = "") const -> std::vector<std::string>;
    auto getStagedFilesListWithStatus(const std::string_view filePattern = "") const -> std::vector<DiffIndexEntry>;
    auto getNotStagedFilesList(const std::string_view filePattern = "") const -> std::vector<std::string>;
    auto getUnmergedFilesListWithDetails(const std::string_view filePattern = "") const -> std::vector<IndexEntry>;

    auto isDirty() const -> bool;

private:
    const Repository& repo;

    auto getHeadFilesHashForGivenFiles(std::vector<DiffIndexEntry>& files) const -> std::vector<std::string>;
    auto getUntrackedAndIndexFilesList(const std::string_view pattern = "") const -> std::vector<std::string>;
};

} // namespace CppGit
