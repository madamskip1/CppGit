#include "Merge.hpp"

#include "Branches.hpp"
#include "Commits.hpp"
#include "GitCommandExecutor/GitCommandOutput.hpp"
#include "Repository.hpp"

namespace CppGit {

auto Merge::mergeFastForward(const std::string_view sourceBranch) const -> std::string
{
    return mergeFastForward(sourceBranch, "HEAD");
}

auto Merge::mergeFastForward(const std::string_view sourceBranch, const std::string_view targetBranch) const -> std::string
{
    auto index = repo.Index();

    if (index.isDirty())
    {
        throw std::runtime_error("Cannot merge with dirty worktree");
    }

    auto ancestor = getAncestor(sourceBranch, targetBranch);

    auto branches = repo.Branches();

    auto sourceBranchRef = branches.getHashBranchRefersTo(sourceBranch);
    auto targetBranchRef = branches.getHashBranchRefersTo(targetBranch);

    if (ancestor == sourceBranchRef)
    {
        // Nothing to merge
        return targetBranchRef;
    }

    if (ancestor != targetBranchRef)
    {
        throw std::runtime_error("Cannot fast-forward");
    }

    branches.changeCurrentBranchRef(sourceBranchRef);

    return sourceBranchRef;
}

auto Merge::mergeNoFastForward(const std::string_view sourceBranch, const std::string_view message, const std::string_view description) const -> std::string
{
    auto index = repo.Index();

    if (index.isDirty())
    {
        throw std::runtime_error("Cannot merge with dirty worktree");
    }

    auto mergeBaseOutput = repo.executeGitCommand("merge-base", "HEAD", sourceBranch);
    if (mergeBaseOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to find merge base");
    }

    auto mergeBase = mergeBaseOutput.stdout;

    auto branches = repo.Branches();
    auto sourceBranchRef = branches.getHashBranchRefersTo(sourceBranch);
    auto targetBranchRef = branches.getHashBranchRefersTo("HEAD");

    if (mergeBase == sourceBranchRef)
    {
        throw std::runtime_error("Nothing to merge");
    }

    if (mergeBase == sourceBranchRef)
    {
        throw std::runtime_error("Nothing to merge");
    }

    auto readTreeOutput = repo.executeGitCommand("read-tree", "-m", mergeBase, "HEAD", sourceBranch);

    if (readTreeOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to read tree");
    }

    auto checkoutIndexOutput = repo.executeGitCommand("checkout-index", "-a");

    if (checkoutIndexOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to checkout index");
    }

    auto gitLsFilesOutput = repo.executeGitCommand("ls-files", "-u");

    if (gitLsFilesOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to list files");
    }

    if (!gitLsFilesOutput.stdout.empty())
    {
        // TODO
        throw std::runtime_error("Conflicts detected");
    }


    return createMergeCommit(sourceBranchRef, targetBranchRef, message, description);
}


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

auto Merge::createMergeCommit(const std::string_view sourceBranchRef, const std::string_view targetBranchRef, const std::string_view message, const std::string_view description) const -> std::string
{
    auto writeTreeOutput = repo.executeGitCommand("write-tree");
    if (writeTreeOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to write tree");
    }


    const auto& treeHash = writeTreeOutput.stdout;

    auto commitOutput = repo.executeGitCommand("commit-tree", treeHash, "-p", targetBranchRef, "-p", sourceBranchRef, "-m", message, (description.empty() ? "" : "-m"), description);

    if (commitOutput.return_code != 0)
    {
        throw std::runtime_error("Failed to create commit222");
    }

    const auto& commitHash = commitOutput.stdout;

    auto branches = repo.Branches();
    branches.changeBranchRef("HEAD", commitHash);

    auto x = repo.executeGitCommand("update-index", "--refresh", "--again", "--quiet");

    if (x.return_code != 0)
    {
        throw std::runtime_error("Failed to read tree333");
    }

    return commitHash;

    return std::string();
}

} // namespace CppGit
