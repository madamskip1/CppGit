#include "CppGit/Resetter.hpp"

#include "CppGit/Repository.hpp"
#include "CppGit/_details/GitFilesHelper.hpp"
#include "CppGit/_details/IndexWorktreeManager.hpp"
#include "CppGit/_details/ReferencesManager.hpp"

#include <string_view>

namespace CppGit {

Resetter::Resetter(const Repository& repository)
    : repository{ &repository },
      gitFilesHelper{ repository },
      referencesManager{ repository }
{
}

auto CppGit::Resetter::resetSoft(const std::string_view commitHash) const -> void
{
    gitFilesHelper.setOrigHeadFile(referencesManager.getRefHash("HEAD"));
    referencesManager.updateRefHash("HEAD", commitHash);
}

auto CppGit::Resetter::resetHard(const std::string_view commitHash) const -> void
{
    gitFilesHelper.setOrigHeadFile(referencesManager.getRefHash("HEAD"));
    const auto indexWorktreeManager = _details::IndexWorktreeManager{ *repository };
    indexWorktreeManager.resetIndexToTree(commitHash);
    indexWorktreeManager.copyForceIndexToWorktree();
    referencesManager.updateRefHash("HEAD", commitHash);
}


} // namespace CppGit
