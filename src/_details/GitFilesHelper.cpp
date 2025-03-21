#include "CppGit/_details/GitFilesHelper.hpp"

#include "CppGit/Repository.hpp"
#include "CppGit/_details/FileUtility.hpp"

#include <string>
#include <string_view>

namespace CppGit::_details {

GitFilesHelper::GitFilesHelper(const Repository& repo)
    : repository{ &repo }
{ }

auto GitFilesHelper::setOrigHeadFile(const std::string_view commitHash) const -> void
{
    FileUtility::createOrOverwriteFile(repository->getGitDirectoryPath() / "ORIG_HEAD", commitHash);
}


auto GitFilesHelper::setHeadFile(const std::string_view refName) const -> void
{
    FileUtility::createOrOverwriteFile(repository->getGitDirectoryPath() / "HEAD", refName);
}


auto GitFilesHelper::getHeadFile() const -> std::string
{
    auto headFileContent = FileUtility::readFile(repository->getGitDirectoryPath() / "HEAD");
    if (headFileContent.ends_with('\n')) // Even we set it without \n some git commands may add it
    {
        headFileContent.pop_back();
    }
    return headFileContent;
}
} // namespace CppGit::_details
