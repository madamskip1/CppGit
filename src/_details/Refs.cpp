#include "CppGit/_details/Refs.hpp"

#include "CppGit/Repository.hpp"
#include "CppGit/_details/GitFilesHelper.hpp"

#include <string>
#include <string_view>
#include <utility>

namespace CppGit::_details {

Refs::Refs(const Repository& repo)
    : repo{ &repo }
{
}


auto Refs::getRefHash(const std::string_view refName) const -> std::string
{
    auto output = repo->executeGitCommand("rev-parse", refName);
    return std::move(output.stdout);
}


auto Refs::getSymbolicRef(const std::string_view refName) const -> std::string
{
    auto output = repo->executeGitCommand("symbolic-ref", refName);
    return std::move(output.stdout);
}


auto Refs::refExists(const std::string_view refName) const -> bool
{
    const auto output = repo->executeGitCommand("show-ref", "--verify", "--quiet", refName);
    return output.return_code == 0;
}


auto Refs::updateRefHash(const std::string_view refName, const std::string_view newHash) const -> void
{
    repo->executeGitCommand("update-ref", refName, newHash);
}


auto Refs::updateSymbolicRef(const std::string_view refName, const std::string_view newRef) const -> void
{
    repo->executeGitCommand("symbolic-ref", refName, newRef);
}

auto Refs::deleteRef(const std::string_view refName) const -> void
{
    repo->executeGitCommand("update-ref", "-d", refName);
}

auto Refs::createRef(const std::string_view refName, const std::string_view hash) const -> void
{
    repo->executeGitCommand("update-ref", refName, hash);
}

auto Refs::detachHead(const std::string_view commitHash) const -> void
{
    GitFilesHelper{ *repo }.setHeadFile(commitHash);
}

auto Refs::appendPrefixToRefIfNeeded(const std::string_view refName, bool remote) -> std::string
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
