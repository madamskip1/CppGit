#include "Commits.hpp"

#include "Repository.hpp"
#include "_details/AmendCommit.hpp"
#include "_details/GitCommandExecutor/GitCommandOutput.hpp"
#include "_details/GitFilesHelper.hpp"
#include "_details/Parser/CommitParser.hpp"
#include "_details/Refs.hpp"

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace CppGit {

Commits::Commits(const Repository& repo)
    : repo(repo),
      _createCommit(repo)
{
}

auto Commits::createCommit(const std::string_view message, const std::string_view description) const -> std::string
{
    auto parent = hasAnyCommits() ? getHeadCommitHash() : std::string{};

    if (!parent.empty())
    {
        _details::GitFilesHelper{ repo }.setOrigHeadFile(parent);
    }

    return _createCommit.createCommit(message, description, { parent }, {});
}

auto Commits::amendCommit(const std::string_view message, const std::string_view description) const -> std::string
{
    auto headCommitHash = getHeadCommitHash();
    auto commitInfo = getCommitInfo(headCommitHash);

    _details::GitFilesHelper{ repo }.setOrigHeadFile(headCommitHash);
    return _details::AmendCommit{ repo }.amend(commitInfo, message, description);
}

auto Commits::hasAnyCommits() const -> bool
{
    auto output = repo.executeGitCommand("rev-parse", "--verify", "HEAD");
    if (output.return_code == 0)
    {
        return true;
    }

    if (output.stderr == "fatal: Needed a single revision")
    {
        return false;
    }

    throw std::runtime_error("Failed to check if there are any commits");
}

auto Commits::getHeadCommitHash() const -> std::string
{
    auto refs = _details::Refs{ repo };
    return refs.getRefHash("HEAD");
}

auto Commits::getCommitInfo(const std::string_view commitHash) const -> Commit
{
    auto output = repo.executeGitCommand("cat-file", "-p", commitHash);
    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to get commit info");
    }

    auto parsedCommit = CommitParser::parseCommit_CatFile(output.stdout);
    parsedCommit.hash = std::string{ commitHash };
    return parsedCommit;
}

} // namespace CppGit
