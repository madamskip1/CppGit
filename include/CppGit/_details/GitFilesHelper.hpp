#pragma once

#include "../Repository.hpp"

namespace CppGit::_details {

class GitFilesHelper
{
public:
    explicit GitFilesHelper(const Repository& repo);

    auto setOrigHeadFile(const std::string_view commitHash) const -> void;

    auto setHeadFile(const std::string_view refName) const -> void;
    auto getHeadFile() const -> std::string;

private:
    const Repository* repo;
};

} // namespace CppGit::_details
