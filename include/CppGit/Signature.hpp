#pragma once
#include <string>
#include <utility>

namespace CppGit {

/// @brief Represents a signature (name and email) of a commit author or committer.
struct Signature
{
    Signature(std::string name, std::string email)
        : name(std::move(name)),
          email(std::move(email))
    {
    }

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    std::string name;  ///< The name of the author or committer.
    std::string email; ///< The email of the author or committer.
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

} // namespace CppGit
