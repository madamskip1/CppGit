#include "Merge.hpp"

#include "Branches.hpp"
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
    auto ancestor = getAncestor(sourceBranch, targetBranch);
    auto commits = repo.Commits();

    return ancestor == commits.getHeadCommitHash();
}

auto Merge::isAnythingToMerge(const std::string_view sourceBranch) const -> bool
{
    return isAnythingToMerge(sourceBranch, "HEAD");
}

auto Merge::isAnythingToMerge(const std::string_view sourceBranch, const std::string_view targetBranch) const -> bool
{
    auto ancestor = getAncestor(sourceBranch, targetBranch);
    auto branches = repo.Branches();
    auto sourceBranchRef = branches.getHashBranchRefersTo(sourceBranch);

    return ancestor != sourceBranchRef;
}

auto Merge::getAncestor(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::string
{
    auto output = repo.executeGitCommand("merge-base", sourceBranch, targetBranch);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to find merge base");
    }

    return output.stdout;
}

} // namespace CppGit
