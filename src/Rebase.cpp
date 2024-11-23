#include "Rebase.hpp"

#include "Branches.hpp"
#include "CherryPick.hpp"
#include "CommitsHistory.hpp"
#include "_details/Refs.hpp"

namespace CppGit {
Rebase::Rebase(const Repository& repo)
    : repo(repo)
{
}
auto Rebase::rebase(const std::string_view upstream) const -> std::string
{
    auto branches = repo.Branches();
    auto currentBranchName = branches.getCurrentBranch();

    auto mergeBase = repo.executeGitCommand("merge-base", "HEAD", upstream);

    if (mergeBase.return_code != 0)
    {
        throw std::runtime_error("Failed to find merge base");
    }

    const auto& mergeBaseSha = mergeBase.stdout;
    auto commitsHistory = repo.CommitsHistory();
    auto commitsToRebase = commitsHistory.getCommitsLogDetailed(mergeBaseSha, "HEAD");

    auto refs = _details::Refs(repo);
    auto upstreamCommithash = refs.getRefHash(upstream);
    branches.detachHead(upstreamCommithash);

    auto cherryPick = repo.CherryPick();
    for (const auto& commit : commitsToRebase)
    {
        cherryPick.cherryPickCommit(commit.getHash(), CherryPickEmptyCommitStrategy::KEEP);
    }

    auto headCommitHash = branches.getHashBranchRefersTo("HEAD");
    refs.updateRefHash(currentBranchName, headCommitHash);
    refs.updateSymbolicRef("HEAD", currentBranchName);

    return headCommitHash;
}

} // namespace CppGit
