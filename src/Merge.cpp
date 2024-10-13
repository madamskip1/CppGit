#include "Merge.hpp"

#include "Commits.hpp"
#include "GitCommandExecutor/GitCommandOutput.hpp"
#include "Repository.hpp"

namespace CppGit {

auto Merge::canFastForward(const std::string_view sourceBranch) const -> bool
{
    return canFastForward(sourceBranch, "HEAD");
}

auto Merge::canFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool
{
    auto output = repo.executeGitCommand("merge-base", sourceBranch, targetBranch);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to find merge base");
    }

    const auto& ancestor = output.stdout;
    auto commits = repo.Commits();

    return ancestor == commits.getHeadCommitHash();
}

} // namespace CppGit
