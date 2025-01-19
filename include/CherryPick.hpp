#pragma once

#include "Error.hpp"
#include "Repository.hpp"
#include "_details/ApplyDiff.hpp"
#include "_details/CreateCommit.hpp"

#include <expected>
#include <string>
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

    auto cherryPickCommit(const std::string_view commitHash, CherryPickEmptyCommitStrategy emptyCommitStrategy = CherryPickEmptyCommitStrategy::STOP) const -> std::expected<std::string, Error>;
    auto commitEmptyCherryPickedCommit() const -> std::expected<std::string, Error>;
    auto cherryPickContinue() const -> std::expected<std::string, Error>;

    auto isCherryPickInProgress() const -> bool;

private:
    const Repository& repo;
    const _details::CreateCommit _createCommit;
    const _details::ApplyDiff _applyDiff;


    auto commitCherryPicked(const std::string_view commitHash) const -> std::string;
    auto createCherryPickHeadFile(const std::string_view commitHash) const -> void;
    auto getCherryPickHead() const -> std::string;

    auto processEmptyDiff(const std::string_view commitHash, CherryPickEmptyCommitStrategy emptyCommitStrategy) const -> std::expected<std::string, Error>;
};

} // namespace CppGit
