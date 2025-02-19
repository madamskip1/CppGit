#include "_details/GitFilesHelper.hpp"

#include "_details/FileUtility.hpp"

namespace CppGit::_details {

GitFilesHelper::GitFilesHelper(const Repository& repo)
    : repo(repo)
{ }

auto GitFilesHelper::setOrigHeadFile(const std::string_view commitHash) const -> void
{
    FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "ORIG_HEAD", commitHash);
}

} // namespace CppGit::_details
