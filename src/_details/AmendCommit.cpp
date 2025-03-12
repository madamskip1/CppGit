#include "CppGit/_details/AmendCommit.hpp"

#include "CppGit/Commit.hpp"
#include "CppGit/_details/CreateCommit.hpp"

namespace CppGit::_details {
AmendCommit::AmendCommit(const Repository& repo)
    : repo{ &repo }
{
}

auto AmendCommit::amend(const Commit& commit) const -> std::string
{
    return amend(commit, "", "");
}

auto AmendCommit::amend(const Commit& commit, const std::string_view newMessage, const std::string_view newDescription) const -> std::string
{
    auto envp = std::vector<std::string>{};
    envp.reserve(3);
    envp.emplace_back("GIT_AUTHOR_NAME=" + commit.getAuthor().name);
    envp.emplace_back("GIT_AUTHOR_EMAIL=" + commit.getAuthor().email);
    envp.emplace_back("GIT_AUTHOR_DATE=" + commit.getAuthorDate());

    const auto createCommit = CreateCommit{ *repo };
    if (!newMessage.empty())
    {
        return createCommit.createCommit(newMessage, newDescription, commit.getParents(), envp);
    }

    return createCommit.createCommit(commit.getMessage(), commit.getDescription(), commit.getParents(), envp);
}

} // namespace CppGit::_details
