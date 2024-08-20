#pragma once

#include <string>
#include <string_view>

namespace CppGit {

class Repository; // Forward declaration
class Commit;     // Forward declaration

class Commits
{
public:
    explicit Commits(const Repository& repo);

    auto createCommit(const std::string_view message, const std::string_view description = "") const -> std::string;

    auto hasAnyCommits() const -> bool;

    auto getHeadCommitHash() const -> std::string;
    auto getCommitInfo(const std::string_view commitHash) const -> Commit;

private:
    const Repository& repo;
};

} // namespace CppGit
