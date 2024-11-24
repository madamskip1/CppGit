#pragma once

#include "Repository.hpp"

namespace CppGit::_details {

class Refs
{
public:
    static constexpr const char* const LOCAL_BRANCH_PREFIX = "refs/heads/";
    static constexpr const char* const REMOTE_BRANCH_PREFIX = "refs/remotes/";

    explicit Refs(const Repository& repo);

    auto getRefHash(std::string_view refName) const -> std::string;
    auto getSymbolicRef(std::string_view refName) const -> std::string;

    auto refExists(std::string_view refName) const -> bool;

    auto updateRefHash(std::string_view refName, std::string_view newHash) const -> void;
    auto updateSymbolicRef(std::string_view refName, std::string_view newRef) const -> void;

    auto deleteRef(std::string_view refName) const -> void;

    auto createRef(std::string_view refName, std::string_view hash) const -> void;

    auto detachHead(std::string_view commitHash) const -> void;

    static auto getRefWithAddedPrefixIfNeeded(std::string_view refName, bool remote) -> std::string;

private:
    const Repository& repo;
};

} // namespace CppGit::_details
