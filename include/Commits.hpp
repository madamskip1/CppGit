#pragma once

#include <string>
#include <string_view>

namespace CppGit {

class Repository; // Forward declaration

class Commits
{
public:
    explicit Commits(const Repository& repo);

    void createCommit(const std::string_view message, const std::string_view description = "") const;

    bool hasAnyCommits() const;

    std::string getHeadCommitHash() const;

private:
    const Repository& repo;
};

} // namespace CppGit
