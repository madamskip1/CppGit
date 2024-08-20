#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace CppGit {

class Repository; // forward-declaration
class Commit;     // forward-declaration

class CommitsHistory
{
public:
    enum class LOG_MERGES
    {
        NO_MERGES,
        ONLY_MERGES,
        ALL
    };

    enum class Order
    {
        CHRONOLOGICAL,
        REVERSE,
        DATE,
        AUTHOR_DATE,
        TOPOLOGICAL
    };

    explicit CommitsHistory(const Repository& repo);

    std::vector<std::string> getCommitsLogHashesOnly() const;
    std::vector<Commit> getCommitsLogDetailed() const;

    CommitsHistory& setAllBranches(bool allBranches);
    CommitsHistory& resetAllBranches();
    CommitsHistory& setMaxCount(int maxCount);
    CommitsHistory& resetMaxCount();
    CommitsHistory& setSkip(int skip);
    CommitsHistory& resetSkip();
    CommitsHistory& setLogMerges(LOG_MERGES logMerges);
    CommitsHistory& resetLogMerges();
    CommitsHistory& setOrder(Order order);
    CommitsHistory& resetOrder();
    CommitsHistory& setAuthorPattern(const std::string_view authorPattern);
    CommitsHistory& resetAuthorPattern();
    CommitsHistory& setCommitterPattern(const std::string_view committerPattern);
    CommitsHistory& resetCommitterPattern();
    CommitsHistory& setMessagePattern(const std::string_view messagePattern);
    CommitsHistory& resetMessagePattern();

private:
    const Repository& repo_;
    bool allBranches_{ false };
    int maxCount_{ -1 };
    int skip_{ -1 };
    LOG_MERGES logMerges_{ LOG_MERGES::ALL };
    Order order_{ Order::CHRONOLOGICAL };
    std::string authorPattern_;
    std::string committerPattern_;
    std::string messagePattern_;

    std::vector<std::string> prepareCommandsArgument() const;
};

} // namespace CppGit
