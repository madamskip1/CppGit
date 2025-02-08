#include "_details/Refs.hpp"

#include "Repository.hpp"

#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace CppGit::_details {

Refs::Refs(const Repository& repo)
    : repo(repo)
{
}


auto Refs::getRefHash(std::string_view refName) const -> std::string
{
    auto output = repo.executeGitCommand("rev-parse", refName);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to get hash");
    }

    return std::move(output.stdout);
}


auto Refs::getSymbolicRef(std::string_view refName) const -> std::string
{
    auto output = repo.executeGitCommand("symbolic-ref", refName);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to get symbolic ref");
    }

    return std::move(output.stdout);
}


auto Refs::refExists(std::string_view refName) const -> bool
{
    auto output = repo.executeGitCommand("show-ref", "--verify", "--quiet", refName);

    if (output.return_code > 1)
    {
        throw std::runtime_error("Failed to check if ref exists");
    }

    return output.return_code == 0;
}


auto Refs::updateRefHash(std::string_view refName, std::string_view newHash) const -> void
{
    auto output = repo.executeGitCommand("update-ref", refName, newHash);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to update ref hash");
    }
}


auto Refs::updateSymbolicRef(std::string_view refName, std::string_view newRef) const -> void
{
    auto output = repo.executeGitCommand("symbolic-ref", refName, newRef);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to update symbolic ref");
    }
}

auto Refs::deleteRef(std::string_view refName) const -> void
{
    auto output = repo.executeGitCommand("update-ref", "-d", refName);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to delete ref");
    }
}

auto Refs::createRef(std::string_view refName, std::string_view hash) const -> void
{
    auto output = repo.executeGitCommand("update-ref", refName, hash);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to create ref");
    }
}

auto Refs::detachHead(std::string_view commitHash) const -> void
{
    auto HEADFile = std::ofstream{ repo.getGitDirectoryPath() / "HEAD" };
    HEADFile << commitHash;
    HEADFile.close();
}

auto Refs::getRefWithAddedPrefixIfNeeded(std::string_view refName, bool remote) -> std::string
{
    if (refName == "HEAD")
    {
        return std::string{ "HEAD" };
    }

    const auto* prefix = remote ? REMOTE_BRANCH_PREFIX : LOCAL_BRANCH_PREFIX;

    if (refName.find(prefix) == std::string::npos)
    {
        return prefix + std::string{ refName };
    }

    return std::string{ refName };
}
} // namespace CppGit::_details
