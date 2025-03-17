#include "CppGit/CommitsLog.hpp"

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
CommitsLog::CommitsLog(const Repository& repo)
    : repo{ &repo }
{
}

auto CommitsLog::getCommitsLogHashesOnly(const std::string_view ref) const -> std::vector<std::string>
{
    return getCommitsLogHashesOnlyImpl("", ref);
}

auto CommitsLog::getCommitsLogHashesOnly(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<std::string>
{
    return getCommitsLogHashesOnlyImpl(fromRef, toRef);
}

auto CommitsLog::getCommitsLogDetailed(const std::string_view ref) const -> std::vector<Commit>
{
    return getCommitsLogDetailedImpl("", ref);
}

auto CommitsLog::getCommitsLogDetailed(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<Commit>
{
    return getCommitsLogDetailedImpl(fromRef, toRef);
}

auto CommitsLog::setAllBranches(const bool allBranches) -> CommitsLog&
{
    allBranches_ = allBranches;
    return *this;
}

auto CommitsLog::resetAllBranches() -> CommitsLog&
{
    allBranches_ = false;
    return *this;
}

auto CommitsLog::setMaxCount(int maxCount) -> CommitsLog&
{
    maxCount_ = maxCount;
    return *this;
}
auto CommitsLog::resetMaxCount() -> CommitsLog&
{
    maxCount_ = -1;
    return *this;
}
auto CommitsLog::setSkip(int skip) -> CommitsLog&
{
    skip_ = skip;
    return *this;
}
auto CommitsLog::resetSkip() -> CommitsLog&
{
    skip_ = -1;
    return *this;
}
auto CommitsLog::setLogMerges(LOG_MERGES logMerges) -> CommitsLog&
{
    logMerges_ = logMerges;
    return *this;
}
auto CommitsLog::resetLogMerges() -> CommitsLog&
{
    logMerges_ = LOG_MERGES::ALL;
    return *this;
}
auto CommitsLog::setOrder(Order order) -> CommitsLog&
{
    order_ = order;
    return *this;
}
auto CommitsLog::resetOrder() -> CommitsLog&
{
    order_ = Order::CHRONOLOGICAL;
    return *this;
}
auto CommitsLog::setAuthorPattern(std::string_view authorPattern) -> CommitsLog&
{
    authorPattern_ = authorPattern;
    return *this;
}
auto CommitsLog::resetAuthorPattern() -> CommitsLog&
{
    authorPattern_ = "";
    return *this;
}
auto CommitsLog::setCommitterPattern(std::string_view committerPattern) -> CommitsLog&
{
    committerPattern_ = committerPattern;
    return *this;
}
auto CommitsLog::resetCommitterPattern() -> CommitsLog&
{
    committerPattern_ = "";
    return *this;
}
auto CommitsLog::setMessagePattern(std::string_view messagePattern) -> CommitsLog&
{
    messagePattern_ = messagePattern;
    return *this;
}

auto CommitsLog::resetMessagePattern() -> CommitsLog&
{
    messagePattern_ = "";
    return *this;
}

auto CommitsLog::prepareCommandsArgument(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<std::string>
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

auto CommitsLog::getCommitsLogHashesOnlyImpl(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<std::string>
{
    auto arguments = prepareCommandsArgument(fromRef, toRef);
    const auto output = repo->executeGitCommand("rev-list", std::move(arguments));

    return Parser::splitToStringsVector(output.stdout, '\n');
}

auto CommitsLog::getCommitsLogDetailedImpl(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<Commit>
{
    auto arguments = prepareCommandsArgument(fromRef, toRef);
    auto formatString = std::string{ "--pretty=" } + CommitParser::COMMIT_LOG_DEFAULT_FORMAT + "$:>";
    arguments.push_back(std::move(formatString));
    arguments.emplace_back("--no-commit-header");
    arguments.emplace_back("--date=raw");

    auto output = repo->executeGitCommand("rev-list", std::move(arguments));

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
