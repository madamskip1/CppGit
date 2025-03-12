#include "CppGit/Reset.hpp"

#include "CppGit/_details/GitFilesHelper.hpp"
#include "CppGit/_details/IndexWorktree.hpp"
#include "CppGit/_details/Refs.hpp"

namespace CppGit {

Reset::Reset(const Repository& repo)
    : repo{ &repo },
      gitFilesHelper{ repo },
      refs{ repo }
{
}

auto CppGit::Reset::resetSoft(const std::string_view commitHash) const -> void
{
    gitFilesHelper.setOrigHeadFile(refs.getRefHash("HEAD"));
    refs.updateRefHash("HEAD", commitHash);
}

auto CppGit::Reset::resetHard(const std::string_view commitHash) const -> void
{
    gitFilesHelper.setOrigHeadFile(refs.getRefHash("HEAD"));
    const auto indexWorktree = _details::IndexWorktree{ *repo };
    indexWorktree.resetIndexToTree(commitHash);
    indexWorktree.copyForceIndexToWorktree();
    refs.updateRefHash("HEAD", commitHash);
}


} // namespace CppGit
