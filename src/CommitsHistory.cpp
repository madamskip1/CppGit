#include "CommitsHistory.hpp"

#include "Commit.hpp"
#include "Parser/CommitParser.hpp"
#include "Parser/Parser.hpp"
#include "Repository.hpp"

namespace CppGit {
CommitsHistory::CommitsHistory(const Repository& repo)
    : repo_(repo)
{
}

std::vector<std::string> CommitsHistory::getCommitsLogHashesOnly() const
{
    auto arguments = prepareCommandsArgument();
    auto output = repo_.executeGitCommand("rev-list", arguments);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Error while getting commits log hashes");
    }

    auto hashes = std::vector<std::string>();
    auto hasheshSplited = Parser::split(output.stdout, '\n');

    return std::vector<std::string>{ hasheshSplited.begin(), hasheshSplited.end() };
}

std::vector<Commit> CommitsHistory::getCommitsLogDetailed() const
{
    auto arguments = prepareCommandsArgument();
    auto formatString = std::string{ "--pretty=" } + CommitParser::COMMIT_LOG_DEFAULT_FORMAT + "$:>";
    arguments.push_back(formatString);
    arguments.emplace_back("--no-commit-header");

    auto output = repo_.executeGitCommand("rev-list", arguments);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Error while getting commits log detailed");
    }

    auto commits = std::vector<Commit>();
    output.stdout.erase(output.stdout.size() - 3); // remove $:> from last line
    auto commitsSplitted = Parser::split(output.stdout, "$:>\n");

    for (const auto commitLog : commitsSplitted)
    {
        if (commitLog.empty())
        {
            continue;
        }
        commits.push_back(CommitParser::parseCommit_PrettyFormat(commitLog));
    }

    return commits;
}

CommitsHistory& CommitsHistory::setAllBranches(bool allBranches)
{
    allBranches_ = allBranches;
    return *this;
}

CommitsHistory& CommitsHistory::resetAllBranches()
{
    allBranches_ = false;
    return *this;
}

CommitsHistory& CommitsHistory::setMaxCount(int maxCount)
{
    maxCount_ = maxCount;
    return *this;
}
CommitsHistory& CommitsHistory::resetMaxCount()
{
    maxCount_ = -1;
    return *this;
}
CommitsHistory& CommitsHistory::setSkip(int skip)
{
    skip_ = skip;
    return *this;
}
CommitsHistory& CommitsHistory::resetSkip()
{
    skip_ = -1;
    return *this;
}
CommitsHistory& CommitsHistory::setLogMerges(LOG_MERGES logMerges)
{
    logMerges_ = logMerges;
    return *this;
}
CommitsHistory& CommitsHistory::resetLogMerges()
{
    logMerges_ = LOG_MERGES::ALL;
    return *this;
}
CommitsHistory& CommitsHistory::setOrder(Order order)
{
    order_ = order;
    return *this;
}
CommitsHistory& CommitsHistory::resetOrder()
{
    order_ = Order::CHRONOLOGICAL;
    return *this;
}
CommitsHistory& CommitsHistory::setAuthorPattern(std::string_view authorPattern)
{
    authorPattern_ = authorPattern;
    return *this;
}
CommitsHistory& CommitsHistory::resetAuthorPattern()
{
    authorPattern_ = "";
    return *this;
}
CommitsHistory& CommitsHistory::setCommitterPattern(std::string_view committerPattern)
{
    committerPattern_ = committerPattern;
    return *this;
}
CommitsHistory& CommitsHistory::resetCommitterPattern()
{
    committerPattern_ = "";
    return *this;
}
CommitsHistory& CommitsHistory::setMessagePattern(std::string_view messagePattern)
{
    messagePattern_ = messagePattern;
    return *this;
}

CommitsHistory& CommitsHistory::resetMessagePattern()
{
    messagePattern_ = "";
    return *this;
}

std::vector<std::string> CommitsHistory::prepareCommandsArgument() const
{
    auto arguments = std::vector<std::string>();
    if (allBranches_)
    {
        arguments.emplace_back("--all");
    }

    if (maxCount_ != -1)
    {
        arguments.emplace_back("--max-count=" + std::to_string(maxCount_));
    }

    if (skip_ != -1)
    {
        arguments.emplace_back("--skip=" + std::to_string(skip_));
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

    arguments.emplace_back("HEAD");
    return arguments;
}

} // namespace CppGit
