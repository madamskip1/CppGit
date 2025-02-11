#pragma once

#include "Commit.hpp"
#include "Parser.hpp"

#include <string_view>

namespace CppGit {

class CommitParser final : protected Parser
{
public:
    static constexpr const char* const COMMIT_LOG_DEFAULT_FORMAT = "%H;%P;%an;%ae;%at;%cn;%ce;%ct;%s;%b"; ///< The format to use when parsing commits
    static constexpr const char* const COMMIT_LOG_DEFAULT_DELIMITER = ";";                                ///< The default delimiter to use when parsing commits

    /// @brief Parse a commit log from the git cat-file command
    /// @param commitLog Commit log to parse
    /// @return Commit object
    static auto parseCommit_CatFile(std::string_view commitLog) -> Commit;

    /// @brief Parse a commit log from the git rev-list command
    /// @param commitLog Commit log to parse
    /// @return Commit object
    static auto parseCommit_PrettyFormat(std::string_view commitLog) -> Commit;

    /// @brief Parse a commit log from the git rev-list command
    /// @param commitLog Commit log to parse
    /// @param format Format to use when parsing the commit log
    /// @param delimiter Delimiter to use when parsing the commit log
    /// @return Commit object
    static auto parseCommit_PrettyFormat(std::string_view commitLog, std::string_view format, std::string_view delimiter) -> Commit;

private:
    static auto isHashToken(std::string_view token) -> bool;
    static auto isParentsToken(std::string_view token) -> bool;
    static auto isAuthorNameToken(std::string_view token) -> bool;
    static auto isAuthorEmailToken(std::string_view token) -> bool;
    static auto isAuthorDateToken(std::string_view token) -> bool;
    static auto isCommitterNameToken(std::string_view token) -> bool;
    static auto isCommitterEmailToken(std::string_view token) -> bool;
    static auto isCommitterDateToken(std::string_view token) -> bool;
    static auto isMessageToken(std::string_view token) -> bool;
    static auto isDescriptionToken(std::string_view token) -> bool;
    static auto isTreeHashToken(std::string_view token) -> bool;
};

} // namespace CppGit
