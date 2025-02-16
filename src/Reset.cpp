#include "Reset.hpp"

#include "_details/GitFilesHelper.hpp"
#include "_details/IndexWorktree.hpp"
#include "_details/Refs.hpp"

namespace CppGit {

Reset::Reset(const Repository& repo)
    : repo(repo)
{
}

auto CppGit::Reset::resetSoft(const std::string_view commitHash) const -> void
{
    auto refs = _details::Refs{ repo };
    _details::GitFilesHelper{ repo }.setOrigHeadFile(refs.getRefHash("HEAD"));
    refs.updateRefHash("HEAD", commitHash);
}

auto CppGit::Reset::resetHard(const std::string_view commitHash) const -> void
{
    auto indexWorktree = _details::IndexWorktree{ repo };
    auto refs = _details::Refs{ repo };
    _details::GitFilesHelper{ repo }.setOrigHeadFile(refs.getRefHash("HEAD"));
    indexWorktree.resetIndexToTree(commitHash);
    indexWorktree.copyForceIndexToWorktree();
    refs.updateRefHash("HEAD", commitHash);
}


} // namespace CppGit
