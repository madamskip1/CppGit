#pragma once

#include "../Commit.hpp"
#include "Parser.hpp"

#include <string_view>
#include <vector>

namespace CppGit {

class CommitParser : protected Parser
{
public:
    static constexpr const char* const COMMIT_LOG_DEFAULT_FORMAT = "%H;%P;%an;%ae;%at;%cn;%ce;%ct;%s;%b";
    static constexpr char COMMIT_LOG_DEFAULT_DELIMITER = ';';

    static Commit parseCommit(std::string_view commitLog);
};

} // namespace CppGit
