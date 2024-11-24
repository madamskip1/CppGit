#include "Rebase.hpp"

#include "Branches.hpp"
#include "CherryPick.hpp"
#include "CommitsHistory.hpp"

#include <filesystem>
#include <fstream>

namespace CppGit {
Rebase::Rebase(const Repository& repo)
    : repo(repo),
      refs(_details::Refs(repo))
{
}
auto Rebase::rebase(const std::string_view upstream) const -> void
{
    startRebase(upstream);

    auto mergeBase = repo.executeGitCommand("merge-base", "HEAD", upstream);

    if (mergeBase.return_code != 0)
    {
        throw std::runtime_error("Failed to find merge base");
    }

    const auto& mergeBaseSha = mergeBase.stdout;
    auto commitsHistory = repo.CommitsHistory();
    commitsHistory.setOrder(CommitsHistory::Order::REVERSE);
    auto commitsToRebase = commitsHistory.getCommitsLogDetailed(mergeBaseSha, "HEAD");

    auto upstreamCommithash = refs.getRefHash(upstream);
    auto branches = repo.Branches();
    branches.detachHead(upstreamCommithash);

    auto cherryPick = repo.CherryPick();
    for (const auto& commit : commitsToRebase)
    {
        cherryPick.cherryPickCommit(commit.getHash(), CherryPickEmptyCommitStrategy::KEEP);
    }

    endRebase();
}

auto Rebase::startRebase(const std::string_view upstream) const -> void
{
    auto currentBranchName = repo.Branches().getCurrentBranch();
    createRebaseDir();
    createHeadNameFile(currentBranchName);
    auto upstreamHash = refs.getRefHash(upstream);
    createOntoFile(upstreamHash);
    createOrigHeadFiles(refs.getRefHash("HEAD"));
}

auto Rebase::endRebase() const -> void
{
    auto currentHash = refs.getRefHash("HEAD");
    auto headName = getHeadName();
    refs.updateRefHash(headName, currentHash);
    refs.updateSymbolicRef("HEAD", headName);
}

auto Rebase::createRebaseDir() const -> void
{
    std::filesystem::create_directory(repo.getGitDirectoryPath() / "rebase-merge");
}

auto Rebase::deleteAllRebaseFiles() const -> void
{
    std::filesystem::remove_all(repo.getGitDirectoryPath() / "rebase-merge");
    std::filesystem::remove(repo.getGitDirectoryPath() / "ORIG_HEAD");
    std::filesystem::remove(repo.getGitDirectoryPath() / "REBASE_HEAD");
}

auto Rebase::createHeadNameFile(const std::string_view branchName) const -> void
{
    // Indicates the branch that was checked out before the rebase started
    auto file = std::ofstream(repo.getGitDirectoryPath() / "rebase-merge" / "head-name");
    file << branchName;
    file.close();
}

auto Rebase::getHeadName() const -> std::string
{
    auto file = std::ifstream(repo.getGitDirectoryPath() / "rebase-merge" / "head-name");
    std::string headName;
    file >> headName;
    file.close();
    return headName;
}

auto Rebase::createOntoFile(const std::string_view onto) const -> void
{
    // Contains the commit hash of the branch or commit onto which the current branch is being rebased
    auto file = std::ofstream(repo.getGitDirectoryPath() / "rebase-merge" / "onto");
    file << onto;
    file.close();
}

auto Rebase::createOrigHeadFiles(const std::string_view origHead) const -> void
{
    // Contains the commit hash of the branch that was checked out before the rebase started
    auto file = std::ofstream(repo.getGitDirectoryPath() / "rebase-merge" / "orig-head");
    file << origHead;
    file.close();

    file = std::ofstream(repo.getGitDirectoryPath() / "ORIG_HEAD");
    file << origHead;
    file.close();
}

} // namespace CppGit
