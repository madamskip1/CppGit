#pragma once

#include "CherryPick.hpp"
#include "Commit.hpp"
#include "Repository.hpp"

#include <string>
#include <string_view>

namespace CppGit {

class CherryPick; // Forward declaration

class Commits
{

    friend CherryPick; // for createCommitImpl method

public:
    explicit Commits(const Repository& repo);

    auto createCommit(const std::string_view message, const std::string_view description = "") const -> std::string;
    auto amendCommit(const std::string_view message = "", const std::string_view description = "") const -> std::string;

    auto hasAnyCommits() const -> bool;

    auto getHeadCommitHash() const -> std::string;
    auto getCommitInfo(const std::string_view commitHash) const -> Commit;

private:
    const Repository& repo;

    auto createCommitImpl(const std::string_view message, const std::string_view description, const std::vector<std::string>& parents, const std::vector<std::string>& envp = {}) const -> std::string;
};

} // namespace CppGit
