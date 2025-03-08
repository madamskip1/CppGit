#pragma once
#include <string>
#include <utility>

namespace CppGit {

struct Signature
{
    Signature(std::string name, std::string email)
        : name(std::move(name)),
          email(std::move(email))
    {
    }

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    std::string name;
    std::string email;
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

} // namespace CppGit
