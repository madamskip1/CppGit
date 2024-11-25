#pragma once

#include "Commit.hpp"
#include "Repository.hpp"
#include "_details/Refs.hpp"


namespace CppGit {

class Rebase
{

public:
    explicit Rebase(const Repository& repo);

    auto rebase(const std::string_view upstream) const -> void;

private:
    auto startRebase(const std::string_view upstream) const -> void;
    auto endRebase() const -> void;

    auto createRebaseDir() const -> void;
    auto deleteAllRebaseFiles() const -> void;
    auto createHeadNameFile(const std::string_view branchName) const -> void;
    auto getHeadName() const -> std::string;
    auto createOntoFile(const std::string_view onto) const -> void;
    auto createOrigHeadFiles(const std::string_view origHead) const -> void;

    auto generateTodoFile(const std::vector<Commit>& commits) const -> void;
    auto processNextCommitRebaseFiles() const -> void;

    static auto getMessageFromTodoLine(const std::string_view line) -> std::string;

    const Repository& repo;
    const _details::Refs refs;
};

} // namespace CppGit
