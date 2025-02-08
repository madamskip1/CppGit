#pragma once
#include <string>

namespace CppGit {

struct Signature
{
    Signature(const std::string& name, const std::string& email)
        : name(name),
          email(email)
    {
    }
    Signature() = default;

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    std::string name;
    std::string email;
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

} // namespace CppGit
