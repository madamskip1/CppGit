#include "CppGit/CommitsLogManager.hpp"

#include "CppGit/Commit.hpp"
#include "CppGit/Repository.hpp"
#include "CppGit/_details/Parser/CommitParser.hpp"
#include "CppGit/_details/Parser/Parser.hpp"

#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace CppGit {
CommitsLogManager::CommitsLogManager(const Repository& repository)
    : repository{ &repository }
{
}

auto CommitsLogManager::getCommitsLogHashesOnly(const std::string_view ref) const -> std::vector<std::string>
{
    return getCommitsLogHashesOnlyImpl("", ref);
}

auto CommitsLogManager::getCommitsLogHashesOnly(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<std::string>
{
    return getCommitsLogHashesOnlyImpl(fromRef, toRef);
}

auto CommitsLogManager::getCommitsLogDetailed(const std::string_view ref) const -> std::vector<Commit>
{
    return getCommitsLogDetailedImpl("", ref);
}

auto CommitsLogManager::getCommitsLogDetailed(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<Commit>
{
    return getCommitsLogDetailedImpl(fromRef, toRef);
}

auto CommitsLogManager::setAllBranches(const bool allBranches) -> CommitsLogManager&
{
    allBranches_ = allBranches;
    return *this;
}

auto CommitsLogManager::resetAllBranches() -> CommitsLogManager&
{
    allBranches_ = false;
    return *this;
}

auto CommitsLogManager::setMaxCount(int maxCount) -> CommitsLogManager&
{
    maxCount_ = maxCount;
    return *this;
}
auto CommitsLogManager::resetMaxCount() -> CommitsLogManager&
{
    maxCount_ = -1;
    return *this;
}
auto CommitsLogManager::setSkip(int skip) -> CommitsLogManager&
{
    skip_ = skip;
    return *this;
}
auto CommitsLogManager::resetSkip() -> CommitsLogManager&
{
    skip_ = -1;
    return *this;
}
auto CommitsLogManager::setLogMerges(LOG_MERGES logMerges) -> CommitsLogManager&
{
    logMerges_ = logMerges;
    return *this;
}
auto CommitsLogManager::resetLogMerges() -> CommitsLogManager&
{
    logMerges_ = LOG_MERGES::ALL;
    return *this;
}
auto CommitsLogManager::setOrder(Order order) -> CommitsLogManager&
{
    order_ = order;
    return *this;
}
auto CommitsLogManager::resetOrder() -> CommitsLogManager&
{
    order_ = Order::CHRONOLOGICAL;
    return *this;
}
auto CommitsLogManager::setAuthorPattern(std::string_view authorPattern) -> CommitsLogManager&
{
    authorPattern_ = authorPattern;
    return *this;
}
auto CommitsLogManager::resetAuthorPattern() -> CommitsLogManager&
{
    authorPattern_ = "";
    return *this;
}
auto CommitsLogManager::setCommitterPattern(std::string_view committerPattern) -> CommitsLogManager&
{
    committerPattern_ = committerPattern;
    return *this;
}
auto CommitsLogManager::resetCommitterPattern() -> CommitsLogManager&
{
    committerPattern_ = "";
    return *this;
}
auto CommitsLogManager::setMessagePattern(std::string_view messagePattern) -> CommitsLogManager&
{
    messagePattern_ = messagePattern;
    return *this;
}

auto CommitsLogManager::resetMessagePattern() -> CommitsLogManager&
{
    messagePattern_ = "";
    return *this;
}

auto CommitsLogManager::prepareCommandsArgument(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<std::string>
{
    auto arguments = std::vector<std::string>();
    if (allBranches_)
    {
        arguments.emplace_back("--all");
    }

    if (maxCount_ != -1)
    {
        arguments.emplace_back(std::format("--max-count={}", maxCount_));
    }

    if (skip_ != -1)
    {
        arguments.emplace_back(std::format("--skip={}", skip_));
    }

    if (logMerges_ == LOG_MERGES::NO_MERGES)
    {
        arguments.emplace_back("--no-merges");
    }
    else if (logMerges_ == LOG_MERGES::ONLY_MERGES)
    {
        arguments.emplace_back("--merges");
    }

    if (order_ == Order::REVERSE)
    {
        arguments.emplace_back("--reverse");
    }
    else if (order_ == Order::DATE)
    {
        arguments.emplace_back("--date-order");
    }
    else if (order_ == Order::AUTHOR_DATE)
    {
        arguments.emplace_back("--author-date");
    }
    else if (order_ == Order::TOPOLOGICAL)
    {
        arguments.emplace_back("--topo-order");
    }

    if (!authorPattern_.empty())
    {
        arguments.emplace_back("--author=" + authorPattern_);
    }

    if (!committerPattern_.empty())
    {
        arguments.emplace_back("--committer=" + committerPattern_);
    }

    if (!messagePattern_.empty())
    {
        arguments.emplace_back("--grep=" + messagePattern_);
    }

    if (fromRef.empty())
    {
        arguments.emplace_back(toRef);
    }
    else
    {
        auto range = std::string{ fromRef } + std::string{ ".." } + std::string{ toRef };
        arguments.emplace_back(std::move(range));
    }

    return arguments;
}

auto CommitsLogManager::getCommitsLogHashesOnlyImpl(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<std::string>
{
    auto arguments = prepareCommandsArgument(fromRef, toRef);
    const auto output = repository->executeGitCommand("rev-list", std::move(arguments));

    return Parser::splitToStringsVector(output.stdout, '\n');
}

auto CommitsLogManager::getCommitsLogDetailedImpl(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<Commit>
{
    auto arguments = prepareCommandsArgument(fromRef, toRef);
    auto formatString = std::string{ "--pretty=" } + CommitParser::COMMIT_LOG_DEFAULT_FORMAT + "$:>";
    arguments.push_back(std::move(formatString));
    arguments.emplace_back("--no-commit-header");
    arguments.emplace_back("--date=raw");

    auto output = repository->executeGitCommand("rev-list", std::move(arguments));

    auto commits = std::vector<Commit>();
    output.stdout.erase(output.stdout.size() - 3); // remove $:> from last line
    const auto commitsSplitted = Parser::splitToStringViewsVector(output.stdout, "$:>\n");

    for (const auto commitLog : commitsSplitted)
    {
        if (commitLog.empty())
        {
            continue;
        }
        commits.emplace_back(CommitParser::parseCommit_PrettyFormat(commitLog));
    }

    return commits;
}

} // namespace CppGit
