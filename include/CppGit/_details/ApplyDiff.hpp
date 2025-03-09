#pragma once

#include "../Repository.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace CppGit::_details {

/// @brief Result of applying diff
enum class ApplyDiffResult : uint8_t
{
    APPLIED,    ///< Diff applied successfully
    EMPTY_DIFF, ///< Diff occurs to be empty
    NO_CHANGES, ///< No changes to apply
    CONFLICT    ///< Conflict occurred
};

class ApplyDiff
{
public:
    /// @param repo The repository to work with
    explicit ApplyDiff(const Repository& repo);

    /// @brief Apply commit's diff to the working directory
    /// @param commitHash Commit hash to apply diff from
    /// @return Result of applying diff
    auto apply(const std::string_view commitHash) const -> ApplyDiffResult;

private:
    const Repository* repo;

    auto getDiff(const std::string_view commitHash) const -> std::string;
    auto createMissingFilesThatOccurInPatch(const std::string_view diff) const -> void;
};
} // namespace CppGit::_details
