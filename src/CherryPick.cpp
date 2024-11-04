#include "CherryPick.hpp"

#include "Commits.hpp"

#include <fstream>

namespace CppGit {

CherryPick::CherryPick(const Repository& repo)
    : repo(repo)
{
}

auto CherryPick::cherryPickCommit(const std::string_view commitHash) const -> std::string
{
    auto diffOutput = repo.executeGitCommand("diff-tree", "-p", commitHash);
    if (diffOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to get diff");
    }

    std::ofstream diffFile(repo.getGitDirectoryPath() / "patch.diff");
    diffFile << diffOutput.stdout;
    diffFile.close();

    auto applyOutput = repo.executeGitCommand("apply", "--index", repo.getGitDirectoryPath() / "patch.diff");

    std::filesystem::remove(repo.getGitDirectoryPath() / "patch.diff");

    if (applyOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to apply diff");
    }

    auto commits = Commits{ repo };

    auto commitInfo = commits.getCommitInfo(commitHash);

    auto envp = std::vector<std::string>{
        "GIT_AUTHOR_NAME=" + commitInfo.getAuthor().name,
        "GIT_AUTHOR_EMAIL=" + commitInfo.getAuthor().email,
        "GIT_AUTHOR_DATE=" + commitInfo.getAuthorDate()
    };

    auto parent = commits.hasAnyCommits() ? commits.getHeadCommitHash() : std::string{};

    return commits.createCommitImpl(commitInfo.getMessage(), commitInfo.getDescription(), { parent }, envp);
}

} // namespace CppGit
