#pragma once

#include "../Commit.hpp"
#include "Parser.hpp"

#include <string_view>

namespace CppGit {

class CommitParser : protected Parser
{
public:
    static constexpr const char* const COMMIT_LOG_DEFAULT_FORMAT = "%H;%P;%an;%ae;%at;%cn;%ce;%ct;%s;%b";
    static constexpr const char* const COMMIT_LOG_DEFAULT_DELIMITER = ";";

    static auto parseCommit_CatFile(std::string_view commitLog) -> Commit;

    static auto parseCommit_PrettyFormat(std::string_view commitLog) -> Commit;
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
