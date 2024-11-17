#include "Commits.hpp"

#include "Branches.hpp"
#include "_details/GitCommandExecutor/GitCommandOutput.hpp"
#include "_details/Parser/CommitParser.hpp"

namespace CppGit {

Commits::Commits(const Repository& repo)
    : repo(repo),
      _createCommit(repo)
{
}

auto Commits::createCommit(const std::string_view message, const std::string_view description) const -> std::string
{
    auto parent = hasAnyCommits() ? getHeadCommitHash() : std::string{};

    return _createCommit.createCommit(message, description, { parent }, {});
}

auto Commits::amendCommit(const std::string_view message, const std::string_view description) const -> std::string
{
    auto headCommitHash = getHeadCommitHash();
    auto commitInfo = getCommitInfo(headCommitHash);

    auto envp = std::vector<std::string>{};
    envp.emplace_back("GIT_AUTHOR_NAME=" + commitInfo.getAuthor().name);
    envp.emplace_back("GIT_AUTHOR_EMAIL=" + commitInfo.getAuthor().email);
    envp.emplace_back("GIT_AUTHOR_DATE=" + commitInfo.getAuthorDate());

    auto newCommitMessage = (message == "" ? commitInfo.getMessage() : std::string{ message });
    auto newCommitDescription = (message == "" ? commitInfo.getDescription() : std::string{ description });

    return _createCommit.createCommit(newCommitMessage, newCommitDescription, commitInfo.getParents(), envp);
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
    auto branches = repo.Branches();
    return branches.getHashBranchRefersTo("HEAD");
}

auto Commits::getCommitInfo(const std::string_view commitHash) const -> Commit
{
    auto output = repo.executeGitCommand("cat-file", "-p", commitHash);
    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to get commit info");
    }

    return CommitParser::parseCommit_CatFile(output.stdout);
}

} // namespace CppGit
