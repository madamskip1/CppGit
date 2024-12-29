#pragma once

#include "RebaseTodoCommand.hpp"
#include "Repository.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace CppGit::_details {

class RebaseFilesHelper
{
public:
    explicit RebaseFilesHelper(const Repository& repo);

    auto createRebaseDir() const -> void;
    auto deleteAllRebaseFiles() const -> void;

    auto createOntoFile(const std::string_view onto) const -> void;

    auto createHeadNameFile(const std::string_view branchName) const -> void;
    auto getHeadName() const -> std::string;

    auto createOrigHeadFiles(const std::string_view origHead) const -> void;
    auto getOrigHead() const -> std::string;

    auto createStoppedShaFile(const std::string_view hash) const -> void;
    auto getStoppedShaFile() const -> std::string;

    auto generateTodoFile(const std::vector<RebaseTodoCommand>& rebaseTodoCommands) const -> void;

    auto peekTodoFile() const -> std::optional<RebaseTodoCommand>;
    auto popTodoFile() const -> void;
    auto appendDoneFile(const RebaseTodoCommand& rebaseTodoCommand) const -> void;


private:
    const Repository& repo;

    static auto parseTodoCommandLine(const std::string_view line) -> std::optional<RebaseTodoCommand>;
};

} // namespace CppGit
