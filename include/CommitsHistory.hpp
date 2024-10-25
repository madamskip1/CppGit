#pragma once

#include "Commit.hpp"
#include "Repository.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace CppGit {

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

    auto getCommitsLogHashesOnly() const -> std::vector<std::string>;
    auto getCommitsLogDetailed() const -> std::vector<Commit>;

    auto setAllBranches(bool allBranches) -> CommitsHistory&;
    auto resetAllBranches() -> CommitsHistory&;
    auto setMaxCount(int maxCount) -> CommitsHistory&;
    auto resetMaxCount() -> CommitsHistory&;
    auto setSkip(int skip) -> CommitsHistory&;
    auto resetSkip() -> CommitsHistory&;
    auto setLogMerges(LOG_MERGES logMerges) -> CommitsHistory&;
    auto resetLogMerges() -> CommitsHistory&;
    auto setOrder(Order order) -> CommitsHistory&;
    auto resetOrder() -> CommitsHistory&;
    auto setAuthorPattern(const std::string_view authorPattern) -> CommitsHistory&;
    auto resetAuthorPattern() -> CommitsHistory&;
    auto setCommitterPattern(const std::string_view committerPattern) -> CommitsHistory&;
    auto resetCommitterPattern() -> CommitsHistory&;
    auto setMessagePattern(const std::string_view messagePattern) -> CommitsHistory&;
    auto resetMessagePattern() -> CommitsHistory&;

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

    auto prepareCommandsArgument() const -> std::vector<std::string>;
};

} // namespace CppGit
