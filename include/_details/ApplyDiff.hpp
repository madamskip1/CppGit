#pragma once

#include "Repository.hpp"

#include <filesystem>
#include <string>
#include <string_view>

namespace CppGit::_details {

enum class ApplyDiffResult
{
    APPLIED,
    EMPTY_DIFF,
    NO_CHANGES,
    CONFLICT
};

class ApplyDiff
{
public:
    explicit ApplyDiff(const Repository& repo);

    auto apply(const std::string_view commitHash) const -> ApplyDiffResult;

private:
    const Repository& repo;
    const std::filesystem::path patchDiffPath;

    auto getDiff(const std::string_view commitHash) const -> std::string;
    auto createMissingFilesThatOccurInPatch(const std::string_view diff) const -> void;
};
} // namespace CppGit::_details
