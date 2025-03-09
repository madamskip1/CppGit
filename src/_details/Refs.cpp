#include "CppGit/_details/Refs.hpp"

#include "CppGit/Repository.hpp"

#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace CppGit::_details {

Refs::Refs(const Repository& repo)
    : repo(&repo)
{
}


auto Refs::getRefHash(const std::string_view refName) const -> std::string
{
    auto output = repo->executeGitCommand("rev-parse", refName);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to get hash");
    }

    return std::move(output.stdout);
}


auto Refs::getSymbolicRef(const std::string_view refName) const -> std::string
{
    auto output = repo->executeGitCommand("symbolic-ref", refName);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to get symbolic ref");
    }

    return std::move(output.stdout);
}


auto Refs::refExists(const std::string_view refName) const -> bool
{
    const auto output = repo->executeGitCommand("show-ref", "--verify", "--quiet", refName);

    if (output.return_code > 1)
    {
        throw std::runtime_error("Failed to check if ref exists");
    }

    return output.return_code == 0;
}


auto Refs::updateRefHash(const std::string_view refName, const std::string_view newHash) const -> void
{
    const auto output = repo->executeGitCommand("update-ref", refName, newHash);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to update ref hash");
    }
}


auto Refs::updateSymbolicRef(const std::string_view refName, const std::string_view newRef) const -> void
{
    const auto output = repo->executeGitCommand("symbolic-ref", refName, newRef);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to update symbolic ref");
    }
}

auto Refs::deleteRef(const std::string_view refName) const -> void
{
    const auto output = repo->executeGitCommand("update-ref", "-d", refName);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to delete ref");
    }
}

auto Refs::createRef(const std::string_view refName, const std::string_view hash) const -> void
{
    const auto output = repo->executeGitCommand("update-ref", refName, hash);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to create ref");
    }
}

auto Refs::detachHead(const std::string_view commitHash) const -> void
{
    auto HEADFile = std::ofstream{ repo->getGitDirectoryPath() / "HEAD" };
    HEADFile << commitHash;
    HEADFile.close();
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
