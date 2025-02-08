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
    struct UnmergedFileBlobs
    {
        std::string baseBlob;   // merge-base
        std::string targetBlob; // ours
        std::string sourceBlob; // their
    };


public:
    explicit ThreeWayMerge(const Repository& repo);
    ThreeWayMerge() = delete;

    auto mergeConflictedFiles(const std::vector<IndexEntry>& unmergedFilesEntries, const std::string_view sourceLabel, const std::string_view targetLabel) const -> void;

    auto createMergeMsgFile(const std::string_view msg, const std::string_view description = "") const -> void;
    auto removeMergeMsgFile() const -> void;
    auto getMergeMsg() const -> std::string;

private:
    const Repository& repo;

    auto unpackFile(const std::string_view fileBlob) const -> std::string;
    static auto createUnmergedFileMap(const std::vector<IndexEntry>& unmergedFilesEntries) -> std::unordered_map<std::string, UnmergedFileBlobs>;
};

} // namespace CppGit::_details
