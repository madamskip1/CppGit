#pragma once

#include "Commit.hpp"
#include "Parser.hpp"

#include <string_view>

namespace CppGit {

class CommitParser final : protected Parser
{
public:
    static constexpr const char* const COMMIT_LOG_DEFAULT_FORMAT = "%H;%P;%an;%ae;%ad;%cn;%ce;%cd;%s;%b"; ///< The format to use when parsing commits
    static constexpr const char* const COMMIT_LOG_DEFAULT_DELIMITER = ";";                                ///< The default delimiter to use when parsing commits

    /// @brief Parse a commit log from the git cat-file command
    /// @param commitLog Commit log to parse
    /// @return Commit object
    [[nodiscard]] static auto parseCommit_CatFile(const std::string_view commitLog) -> Commit;

    /// @brief Parse a commit log from the git rev-list command
    /// @param commitLog Commit log to parse
    /// @return Commit object
    [[nodiscard]] static auto parseCommit_PrettyFormat(const std::string_view commitLog) -> Commit;

    /// @brief Parse a commit log from the git rev-list command
    /// @param commitLog Commit log to parse
    /// @param format Format to use when parsing the commit log
    /// @param delimiter Delimiter to use when parsing the commit log
    /// @return Commit object
    [[nodiscard]] static auto parseCommit_PrettyFormat(const std::string_view commitLog, const std::string_view format, const std::string_view delimiter) -> Commit;

private:
    static auto isHashToken(const std::string_view token) -> bool;
    static auto isParentsToken(const std::string_view token) -> bool;
    static auto isAuthorNameToken(const std::string_view token) -> bool;
    static auto isAuthorEmailToken(const std::string_view token) -> bool;
    static auto isAuthorDateToken(const std::string_view token) -> bool;
    static auto isCommitterNameToken(const std::string_view token) -> bool;
    static auto isCommitterEmailToken(const std::string_view token) -> bool;
    static auto isCommitterDateToken(const std::string_view token) -> bool;
    static auto isMessageToken(const std::string_view token) -> bool;
    static auto isDescriptionToken(const std::string_view token) -> bool;
    static auto isTreeHashToken(const std::string_view token) -> bool;
};

} // namespace CppGit
