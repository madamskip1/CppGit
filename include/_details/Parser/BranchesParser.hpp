#pragma once

#include "Branch.hpp"
#include "Parser.hpp"

#include <string_view>

namespace CppGit {

class BranchesParser final : protected Parser
{
public:
    static constexpr const char* const BRANCHES_FORMAT = "%(refname);%(upstream);%(push)";

    static auto parseBranch(std::string_view line) -> Branch;

    static auto isLocalBranch(std::string_view refName) -> bool;
};


} // namespace CppGit
