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

    auto createCommitEditMsgFile(const std::string_view message) const -> void;
    auto getCommitEditMsgFile() const -> std::string;

    auto createAuthorScriptFile(const std::string_view authorName, const std::string_view authorEmail, const std::string_view authorDate) const -> void;
    auto getAuthorScriptFile() const -> std::vector<std::string>;
    auto removeAuthorScriptFile() const -> void;

    auto createAmendFile(const std::string_view hash) const -> void;
    auto getAmendFile() const -> std::string;

    auto createRebaseHeadFile(const std::string_view hash) const -> void;
    auto getRebaseHeadFile() const -> std::string;
    auto removeRebaseHeadFile() const -> void;

    auto appendRewrittenListFile(const std::string_view hashBefore, const std::string_view hashAfter) const -> void;

    auto generateTodoFile(const std::vector<RebaseTodoCommand>& rebaseTodoCommands) const -> void;

    auto peekTodoFile() const -> std::optional<RebaseTodoCommand>;
    auto popTodoFile() const -> void;
    auto appendDoneFile(const RebaseTodoCommand& rebaseTodoCommand) const -> void;
    auto getLastDoneCommand() const -> std::optional<RebaseTodoCommand>;


private:
    const Repository& repo;

    static auto parseTodoCommandLine(const std::string_view line) -> std::optional<RebaseTodoCommand>;
};

} // namespace CppGit::_details
