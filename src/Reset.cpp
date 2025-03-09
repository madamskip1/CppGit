#include "CppGit/Reset.hpp"

#include "CppGit/_details/GitFilesHelper.hpp"
#include "CppGit/_details/IndexWorktree.hpp"
#include "CppGit/_details/Refs.hpp"

namespace CppGit {

Reset::Reset(const Repository& repo)
    : repo(&repo)
{
}

auto CppGit::Reset::resetSoft(const std::string_view commitHash) const -> void
{
    const auto refs = _details::Refs{ *repo };
    _details::GitFilesHelper{ *repo }.setOrigHeadFile(refs.getRefHash("HEAD"));
    refs.updateRefHash("HEAD", commitHash);
}

auto CppGit::Reset::resetHard(const std::string_view commitHash) const -> void
{
    const auto indexWorktree = _details::IndexWorktree{ *repo };
    const auto refs = _details::Refs{ *repo };
    _details::GitFilesHelper{ *repo }.setOrigHeadFile(refs.getRefHash("HEAD"));
    indexWorktree.resetIndexToTree(commitHash);
    indexWorktree.copyForceIndexToWorktree();
    refs.updateRefHash("HEAD", commitHash);
}


} // namespace CppGit
