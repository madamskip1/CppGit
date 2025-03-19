#include "CppGit/CommitsManager.hpp"

#include "CppGit/Commit.hpp"
#include "CppGit/Repository.hpp"
#include "CppGit/_details/CommitAmender.hpp"
#include "CppGit/_details/CommitCreator.hpp"
#include "CppGit/_details/GitCommandExecutor/GitCommandOutput.hpp"
#include "CppGit/_details/GitFilesHelper.hpp"
#include "CppGit/_details/Parser/CommitParser.hpp"
#include "CppGit/_details/ReferencesManager.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace CppGit {

CommitsManager::CommitsManager(const Repository& repository)
    : repository{ &repository },
      referencesManager{ repository },
      gitFilesHelper{ repository }
{
}

auto CommitsManager::createCommit(const std::string_view message, const std::string_view description) const -> std::string
{
    const auto parent = hasAnyCommits() ? getHeadCommitHash() : std::string{};

    if (!parent.empty())
    {
        gitFilesHelper.setOrigHeadFile(parent);
    }

    const auto newCommitHash = _details::CommitCreator{ *repository }.createCommit(message, description, { parent }, {});
    referencesManager.updateRefHash("HEAD", newCommitHash);

    return newCommitHash;
}

auto CommitsManager::amendCommit(const std::string_view message, const std::string_view description) const -> std::string
{
    const auto headCommitHash = getHeadCommitHash();
    const auto commitInfo = getCommitInfo(headCommitHash);

    gitFilesHelper.setOrigHeadFile(headCommitHash);
    const auto newCommitHash = _details::CommitAmender{ *repository }.amendCommit(commitInfo, message, description);
    referencesManager.updateRefHash("HEAD", newCommitHash);

    return newCommitHash;
}

auto CommitsManager::hasAnyCommits() const -> bool
{
    const auto output = repository->executeGitCommand("rev-parse", "--verify", "HEAD");
    return output.return_code == 0;
}

auto CommitsManager::getHeadCommitHash() const -> std::string
{
    return referencesManager.getRefHash("HEAD");
}

auto CommitsManager::getCommitInfo(const std::string_view commitHash) const -> Commit
{
    const auto output = repository->executeGitCommand("cat-file", "-p", commitHash);
    auto parsedCommit = CommitParser::parseCommit_CatFile(output.stdout);
    parsedCommit.hash = std::string{ commitHash };

    return parsedCommit;
}

} // namespace CppGit
