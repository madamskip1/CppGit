#include "CommitsHistory.hpp"

#include "Commit.hpp"
#include "Repository.hpp"
#include "_details/Parser/CommitParser.hpp"
#include "_details/Parser/Parser.hpp"

#include <format>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace CppGit {
CommitsHistory::CommitsHistory(const Repository& repo)
    : repo_(repo)
{
}

auto CommitsHistory::getCommitsLogHashesOnly(const std::string_view ref) const -> std::vector<std::string>
{
    return getCommitsLogHashesOnlyImpl("", ref);
}

auto CommitsHistory::getCommitsLogHashesOnly(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<std::string>
{
    return getCommitsLogHashesOnlyImpl(fromRef, toRef);
}

auto CommitsHistory::getCommitsLogDetailed(const std::string_view ref) const -> std::vector<Commit>
{
    return getCommitsLogDetailedImpl("", ref);
}

auto CommitsHistory::getCommitsLogDetailed(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<Commit>
{
    return getCommitsLogDetailedImpl(fromRef, toRef);
}

auto CommitsHistory::setAllBranches(const bool allBranches) -> CommitsHistory&
{
    allBranches_ = allBranches;
    return *this;
}

auto CommitsHistory::resetAllBranches() -> CommitsHistory&
{
    allBranches_ = false;
    return *this;
}

auto CommitsHistory::setMaxCount(int maxCount) -> CommitsHistory&
{
    maxCount_ = maxCount;
    return *this;
}
auto CommitsHistory::resetMaxCount() -> CommitsHistory&
{
    maxCount_ = -1;
    return *this;
}
auto CommitsHistory::setSkip(int skip) -> CommitsHistory&
{
    skip_ = skip;
    return *this;
}
auto CommitsHistory::resetSkip() -> CommitsHistory&
{
    skip_ = -1;
    return *this;
}
auto CommitsHistory::setLogMerges(LOG_MERGES logMerges) -> CommitsHistory&
{
    logMerges_ = logMerges;
    return *this;
}
auto CommitsHistory::resetLogMerges() -> CommitsHistory&
{
    logMerges_ = LOG_MERGES::ALL;
    return *this;
}
auto CommitsHistory::setOrder(Order order) -> CommitsHistory&
{
    order_ = order;
    return *this;
}
auto CommitsHistory::resetOrder() -> CommitsHistory&
{
    order_ = Order::CHRONOLOGICAL;
    return *this;
}
auto CommitsHistory::setAuthorPattern(std::string_view authorPattern) -> CommitsHistory&
{
    authorPattern_ = authorPattern;
    return *this;
}
auto CommitsHistory::resetAuthorPattern() -> CommitsHistory&
{
    authorPattern_ = "";
    return *this;
}
auto CommitsHistory::setCommitterPattern(std::string_view committerPattern) -> CommitsHistory&
{
    committerPattern_ = committerPattern;
    return *this;
}
auto CommitsHistory::resetCommitterPattern() -> CommitsHistory&
{
    committerPattern_ = "";
    return *this;
}
auto CommitsHistory::setMessagePattern(std::string_view messagePattern) -> CommitsHistory&
{
    messagePattern_ = messagePattern;
    return *this;
}

auto CommitsHistory::resetMessagePattern() -> CommitsHistory&
{
    messagePattern_ = "";
    return *this;
}

auto CommitsHistory::prepareCommandsArgument(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<std::string>
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

auto CommitsHistory::getCommitsLogHashesOnlyImpl(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<std::string>
{
    const auto arguments = prepareCommandsArgument(fromRef, toRef);
    const auto output = repo_.executeGitCommand("rev-list", arguments);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Error while getting commits log hashes");
    }

    return Parser::splitToStringsVector(output.stdout, '\n');
}

auto CommitsHistory::getCommitsLogDetailedImpl(const std::string_view fromRef, const std::string_view toRef) const -> std::vector<Commit>
{
    auto arguments = prepareCommandsArgument(fromRef, toRef);
    const auto formatString = std::string{ "--pretty=" } + CommitParser::COMMIT_LOG_DEFAULT_FORMAT + "$:>";
    arguments.push_back(std::move(formatString));
    arguments.emplace_back("--no-commit-header");
    arguments.emplace_back("--date=raw");

    auto output = repo_.executeGitCommand("rev-list", arguments);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Error while getting commits log detailed");
    }

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
