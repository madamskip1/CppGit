#include "CppGit/_details/CommitAmender.hpp"

#include "CppGit/Commit.hpp"
#include "CppGit/Repository.hpp"
#include "CppGit/_details/CommitCreator.hpp"

#include <string>
#include <vector>

namespace CppGit::_details {
CommitAmender::CommitAmender(const Repository& repository)
    : repository{ &repository }
{
}

auto CommitAmender::amendCommit(const Commit& commit) const -> std::string
{
    return amendCommit(commit, "", "");
}

auto CommitAmender::amendCommit(const Commit& commit, const std::string_view newMessage, const std::string_view newDescription) const -> std::string
{
    auto envp = std::vector<std::string>{};
    envp.reserve(3);
    envp.emplace_back("GIT_AUTHOR_NAME=" + commit.getAuthor().name);
    envp.emplace_back("GIT_AUTHOR_EMAIL=" + commit.getAuthor().email);
    envp.emplace_back("GIT_AUTHOR_DATE=" + commit.getAuthorDate());

    if (!newMessage.empty())
    {
        return CommitCreator{ *repository }.createCommit(newMessage, newDescription, commit.getParents(), envp);
    }

    return CommitCreator{ *repository }.createCommit(commit.getMessage(), commit.getDescription(), commit.getParents(), envp);
}

} // namespace CppGit::_details
