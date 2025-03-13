#include "CppGit/Commits.hpp"

#include "CppGit/Repository.hpp"
#include "CppGit/_details/AmendCommit.hpp"
#include "CppGit/_details/CreateCommit.hpp"
#include "CppGit/_details/GitCommandExecutor/GitCommandOutput.hpp"
#include "CppGit/_details/GitFilesHelper.hpp"
#include "CppGit/_details/Parser/CommitParser.hpp"
#include "CppGit/_details/Refs.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace CppGit {

Commits::Commits(const Repository& repo)
    : repo{ &repo },
      refs{ repo },
      gitFilesHelper{ repo }
{
}

auto Commits::createCommit(const std::string_view message, const std::string_view description) const -> std::string
{
    const auto parent = hasAnyCommits() ? getHeadCommitHash() : std::string{};

    if (!parent.empty())
    {
        gitFilesHelper.setOrigHeadFile(parent);
    }

    const auto newCommitHash = _details::CreateCommit{ *repo }.createCommit(message, description, { parent }, {});
    refs.updateRefHash("HEAD", newCommitHash);

    return newCommitHash;
}

auto Commits::amendCommit(const std::string_view message, const std::string_view description) const -> std::string
{
    const auto headCommitHash = getHeadCommitHash();
    const auto commitInfo = getCommitInfo(headCommitHash);

    gitFilesHelper.setOrigHeadFile(headCommitHash);
    const auto newCommitHash = _details::AmendCommit{ *repo }.amend(commitInfo, message, description);
    refs.updateRefHash("HEAD", newCommitHash);

    return newCommitHash;
}

auto Commits::hasAnyCommits() const -> bool
{
    const auto output = repo->executeGitCommand("rev-parse", "--verify", "HEAD");
    return output.return_code == 0;
}

auto Commits::getHeadCommitHash() const -> std::string
{
    return refs.getRefHash("HEAD");
}

auto Commits::getCommitInfo(const std::string_view commitHash) const -> Commit
{
    const auto output = repo->executeGitCommand("cat-file", "-p", commitHash);
    auto parsedCommit = CommitParser::parseCommit_CatFile(output.stdout);
    parsedCommit.hash = std::string{ commitHash };

    return parsedCommit;
}

} // namespace CppGit
