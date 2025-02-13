#include "Reset.hpp"

#include "_details/IndexWorktree.hpp"
#include "_details/Refs.hpp"

namespace CppGit {

Reset::Reset(const Repository& repo)
    : repo(repo)
{
}

auto CppGit::Reset::resetSoft(const std::string_view commitHash) const -> void
{
    _details::Refs{ repo }.updateRefHash("HEAD", commitHash);
}

auto CppGit::Reset::resetHard(const std::string_view commitHash) const -> void
{
    auto indexWorktree = _details::IndexWorktree{ repo };
    indexWorktree.resetIndexToTree(commitHash);
    indexWorktree.copyForceIndexToWorktree();
    _details::Refs{ repo }.updateRefHash("HEAD", commitHash);
}


} // namespace CppGit
