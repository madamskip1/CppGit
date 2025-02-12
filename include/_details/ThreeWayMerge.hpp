#pragma once

#include "Index.hpp"
#include "Repository.hpp"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace CppGit::_details {

class ThreeWayMerge
{
    /// @brief Contains the three blobs of an unmerged file
    struct UnmergedFileBlobs
    {
        std::string baseBlob;   ///< Merge base, the common ancestor
        std::string targetBlob; ///< Ours, the branch we are merging onto
        std::string sourceBlob; ///< Theirs, the branch we are merging from
    };


public:
    /// @param repo The repository to work with
    explicit ThreeWayMerge(const Repository& repo);
    ThreeWayMerge() = delete;

    /// @brief Perform a three-way merge on conflicted files
    /// @param unmergedFilesEntries The entries of the unmerged files
    /// @param sourceLabel The label for the source branch
    /// @param targetLabel The label for the target branch
    auto mergeConflictedFiles(const std::vector<IndexEntry>& unmergedFilesEntries, const std::string_view sourceLabel, const std::string_view targetLabel) const -> void;

    /// @brief Create a MERGE_MSG file
    /// @param msg The message to write to the file
    /// @param description The description to write to the file (optional)
    auto createMergeMsgFile(const std::string_view msg, const std::string_view description = "") const -> void;

    /// @brief Remove the MERGE_MSG file
    auto removeMergeMsgFile() const -> void;

    /// @brief Get the content of the MERGE_MSG file
    /// @return The content of the MERGE_MSG file
    [[nodiscard]] auto getMergeMsg() const -> std::string;

private:
    const Repository& repo;

    auto unpackFile(const std::string_view fileBlob) const -> std::string;
    static auto createUnmergedFileMap(const std::vector<IndexEntry>& unmergedFilesEntries) -> std::unordered_map<std::string, UnmergedFileBlobs>;
};

} // namespace CppGit::_details
