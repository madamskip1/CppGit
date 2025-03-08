#pragma once

#include "../../Branch.hpp"
#include "Parser.hpp"

#include <string_view>

namespace CppGit {

class BranchesParser final : protected Parser
{
public:
    static constexpr const char* const BRANCHES_FORMAT = "%(refname);%(upstream);%(push)"; ///< The format to use when parsing branches

    /// @brief Parse a branch line
    /// @param line Line to parse
    /// @return Branch object
    [[nodiscard]] static auto parseBranch(const std::string_view line) -> Branch;

    /// @brief Check if a branch is a local branch
    /// @param refName Branch name
    /// @return True if the branch is a local branch, false otherwise
    [[nodiscard]] static auto isLocalBranch(const std::string_view refName) -> bool; ///< Check if a branch is a local branch
};


} // namespace CppGit
