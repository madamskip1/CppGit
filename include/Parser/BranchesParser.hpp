#pragma once

#include <string>
#include <string_view>

#include "Parser.hpp"
#include "../Branch.hpp"

namespace CppGit
{

    class BranchesParser : protected Parser
    {
    public:
        static constexpr const char *const BRANCHES_FORMAT = "%(refname);%(upstream);%(push)";

        static Branch parseBranch(std::string_view line);

        static bool isLocalBranch(std::string_view refName);
    };
}