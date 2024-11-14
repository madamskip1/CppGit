#pragma once

#include "Repository.hpp"

#include <string_view>

namespace CppGit {

enum class CherryPickEmptyCommitStrategy
{
    STOP,
    DROP,
    KEEP
};

class CherryPick
{
public:
    explicit CherryPick(const Repository& repo);

    auto cherryPickCommit(const std::string_view commitHash, CherryPickEmptyCommitStrategy emptyCommitStrategy = CherryPickEmptyCommitStrategy::STOP) const -> std::string;
    auto commitEmptyCherryPickedCommit() const -> std::string;
    auto cherryPickContinue() const -> std::string;

    auto isCherryPickInProgress() const -> bool;

private:
    const Repository& repo;

    auto commitCherryPicked(const std::string_view commitHash) const -> std::string;
    auto createCherryPickHeadFile(const std::string_view commitHash) const -> void;
    auto createConflictMsgFiles(const std::string_view message, const std::string_view description) const -> void;
    auto getCherryPickHead() const -> std::string;

    auto processEmptyDiff(const std::string_view commitHash, CherryPickEmptyCommitStrategy emptyCommitStrategy) const -> std::string;
};

} // namespace CppGit
