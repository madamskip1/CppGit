#include "CppGit/_details/ReferencesManager.hpp"

#include "CppGit/Repository.hpp"
#include "CppGit/_details/GitFilesHelper.hpp"

#include <string>
#include <string_view>
#include <utility>

namespace CppGit::_details {

ReferencesManager::ReferencesManager(const Repository& repository)
    : repository{ &repository }
{
}


auto ReferencesManager::getRefHash(const std::string_view refName) const -> std::string
{
    auto output = repository->executeGitCommand("rev-parse", refName);
    return std::move(output.stdout);
}


auto ReferencesManager::getSymbolicRef(const std::string_view refName) const -> std::string
{
    auto output = repository->executeGitCommand("symbolic-ref", refName);
    return std::move(output.stdout);
}


auto ReferencesManager::refExists(const std::string_view refName) const -> bool
{
    const auto output = repository->executeGitCommand("show-ref", "--verify", "--quiet", refName);
    return output.return_code == 0;
}


auto ReferencesManager::updateRefHash(const std::string_view refName, const std::string_view newHash) const -> void
{
    repository->executeGitCommand("update-ref", refName, newHash);
}


auto ReferencesManager::updateSymbolicRef(const std::string_view refName, const std::string_view newRef) const -> void
{
    repository->executeGitCommand("symbolic-ref", refName, newRef);
}

auto ReferencesManager::deleteRef(const std::string_view refName) const -> void
{
    repository->executeGitCommand("update-ref", "-d", refName);
}

auto ReferencesManager::createRef(const std::string_view refName, const std::string_view hash) const -> void
{
    repository->executeGitCommand("update-ref", refName, hash);
}

auto ReferencesManager::detachHead(const std::string_view commitHash) const -> void
{
    GitFilesHelper{ *repository }.setHeadFile(commitHash);
}

auto ReferencesManager::appendPrefixToRefIfNeeded(const std::string_view refName, bool remote) -> std::string
{
    if (refName == "HEAD")
    {
        return std::string{ "HEAD" };
    }

    const auto* prefix = remote ? REMOTE_BRANCH_PREFIX : LOCAL_BRANCH_PREFIX;

    if (!refName.contains(prefix))
    {
        return prefix + std::string{ refName };
    }

    return std::string{ refName };
}
} // namespace CppGit::_details
