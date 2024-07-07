#pragma once

namespace CppGit
{
    struct Signature
    {
        Signature (const std::string& name, const std::string& email)
            : name(name), email(email)
        {
        }
        Signature() = default;
        std::string name;
        std::string email;
    };
}