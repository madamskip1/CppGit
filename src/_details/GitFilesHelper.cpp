#include "CppGit/_details/GitFilesHelper.hpp"

#include "CppGit/_details/FileUtility.hpp"

namespace CppGit::_details {

GitFilesHelper::GitFilesHelper(const Repository& repo)
    : repo{ &repo }
{ }

auto GitFilesHelper::setOrigHeadFile(const std::string_view commitHash) const -> void
{
    FileUtility::createOrOverwriteFile(repo->getGitDirectoryPath() / "ORIG_HEAD", commitHash);
}


auto GitFilesHelper::setHeadFile(const std::string_view refName) const -> void
{
    FileUtility::createOrOverwriteFile(repo->getGitDirectoryPath() / "HEAD", refName);
}


auto GitFilesHelper::getHeadFile() const -> std::string
{
    return FileUtility::readFile(repo->getGitDirectoryPath() / "HEAD");
}
} // namespace CppGit::_details
