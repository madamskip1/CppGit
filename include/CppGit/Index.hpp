#pragma once
#include "Error.hpp"
#include "Repository.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace CppGit {

/// @brief Entry in the index
struct IndexEntry
{
    int fileMode;               ///< File mode (100644 - regular file, 100755 - executable file, 120000 - symbolic link)
    int stageNumber;            ///< Stage number (0 - regular, 1 - base, 2 - ours, 3 - theirs)
    std::string objectHash;     ///< Object hash
    std::filesystem::path path; ///< Path to the file
};

/// @brief Status of the file in the diff-index
enum class DiffIndexStatus : uint8_t
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

/// @brief Entry in the diff-index
struct DiffIndexEntry
{
    std::string path;       ///< Path to the file
    DiffIndexStatus status; ///< Status of the file
};

/// @brief Status of the file in the ls-files
enum class LsFilesStatus : uint8_t
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

/// @brief Entry in the ls-files
struct LsFilesEntry
{
    std::string path;     ///< Path to the file
    LsFilesStatus status; ///< Status of the file
};

/// @brief Provides functionality to work with the git index
class Index
{
public:
    /// @param repo The repository to work with
    explicit Index(const Repository& repo);
    Index() = delete;

    /// @brief Add a file to the index
    /// @param filePattern File(s) pattern
    /// @return Error code
    auto add(const std::string_view filePattern) const -> Error;

    /// @brief Remove a file from the index
    /// @param filePattern File(s) pattern
    /// @param force Whether to force remove
    /// @return Error code
    auto remove(const std::string_view filePattern, const bool force = false) const -> Error;

    /// @brief Remove file(s) from stagged state
    /// @tparam Args File(s) arguments types
    /// @param args File(s) patterns
    template <typename... Args>
    auto restoreStaged(Args&&... args) const -> void
    {
        const auto output = repo->executeGitCommand("restore", std::forward<Args>(args)...);

        if (output.return_code != 0)
        {
            throw std::runtime_error("Failed to restore file");
        }
    }

    /// @brief  Remove all files from staged state
    auto restoreAllStaged() const -> void;

    /// @brief Check whether a file is staged
    /// @param file File path
    /// @return True if the file is staged, false otherwise
    [[nodiscard]] auto isFileStaged(const std::string_view file) const -> bool;

    /// @brief Get list of files in the index
    /// @param filePattern File(s) pattern to filter
    /// @return List of files in the index
    [[nodiscard]] auto getFilesInIndexList(const std::string_view filePattern = "") const -> std::vector<std::string>;

    /// @brief Get list of files in the index with details
    /// @param filePattern File(s) pattern to filter
    /// @return List of files in the index with details
    [[nodiscard]] auto getFilesInIndexListWithDetails(const std::string_view filePattern = "") const -> std::vector<IndexEntry>;


    /// @brief Get list of untracked files
    /// @param filePattern File(s) pattern to filter
    /// @return List of untracked files
    [[nodiscard]] auto getUntrackedFilesList(const std::string_view filePattern = "") const -> std::vector<std::string>;

    /// @brief Get list of staged files
    /// @param filePattern File(s) pattern to filter
    /// @return List of staged files
    [[nodiscard]] auto getStagedFilesList(const std::string_view filePattern = "") const -> std::vector<std::string>;

    /// @brief Get list of staged files with details
    /// @param filePattern File(s) pattern to filter
    /// @return List of staged files with details
    [[nodiscard]] auto getStagedFilesListWithStatus(const std::string_view filePattern = "") const -> std::vector<DiffIndexEntry>;

    /// @brief Get list of not staged files
    /// @param filePattern File(s) pattern to filter
    /// @return List of not staged files
    [[nodiscard]] auto getNotStagedFilesList(const std::string_view filePattern = "") const -> std::vector<std::string>;

    /// @brief Get list of unmerged files (conflicted)
    /// @param filePattern File(s) pattern to filter
    /// @return List of unmerged files
    [[nodiscard]] auto getUnmergedFilesListWithDetails(const std::string_view filePattern = "") const -> std::vector<IndexEntry>;

    /// @brief Check whether there are any staged files
    /// @return True if there are any staged files, false otherwise
    [[nodiscard]] auto areAnyStagedFiles() const -> bool;

    /// @brief Check whether there are any not staged files
    /// @return True if there are any not staged files, false otherwise
    [[nodiscard]] auto areAnyNotStagedTrackedFiles() const -> bool;

    /// @brief Check whether working directory is dirty
    /// @return True if working directory is dirty, false otherwise
    [[nodiscard]] auto isDirty() const -> bool;

private:
    const Repository* repo;

    auto getHeadFilesHashForGivenFiles(std::vector<DiffIndexEntry>& files) const -> std::vector<std::string>;
    auto getUntrackedAndIndexFilesList(const std::string_view pattern = "") const -> std::vector<std::string>;

    auto getStagedFilesListOutput(const std::string_view filePattern = "") const -> std::string;
};

} // namespace CppGit
