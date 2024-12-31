#include "_details/RebaseFilesHelper.hpp"

#include "_details/FileUtility.hpp"
#include "_details/Parser/Parser.hpp"

namespace CppGit::_details {

RebaseFilesHelper::RebaseFilesHelper(const Repository& repo)
    : repo{ repo }
{
}

auto RebaseFilesHelper::createRebaseDir() const -> void
{
    std::filesystem::create_directory(repo.getGitDirectoryPath() / "rebase-merge");
}

auto RebaseFilesHelper::deleteAllRebaseFiles() const -> void
{
    std::filesystem::remove_all(repo.getGitDirectoryPath() / "rebase-merge");
    // TODO: other files in .git dir related to rebase
}

auto RebaseFilesHelper::createOntoFile(const std::string_view onto) const -> void
{
    // Contains the commit hash of the branch or commit onto which the current branch is being rebased
    _details::FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "rebase-merge" / "onto", onto);
}

auto RebaseFilesHelper::createHeadNameFile(const std::string_view branchName) const -> void
{
    // Indicates the branch that was checked out before the rebase started
    _details::FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "rebase-merge" / "head-name", branchName);
}

auto RebaseFilesHelper::getHeadName() const -> std::string
{
    return _details::FileUtility::readFile(repo.getGitDirectoryPath() / "rebase-merge" / "head-name");
}

auto RebaseFilesHelper::createOrigHeadFiles(const std::string_view origHead) const -> void
{
    // Contains the commit hash of the branch that was checked out before the rebase started
    _details::FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "rebase-merge" / "orig-head", origHead);
    _details::FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "ORIG_HEAD", origHead); // TODO Sprawdzić ten plik co zawiera i kiedy
}

auto RebaseFilesHelper::getOrigHead() const -> std::string
{
    return _details::FileUtility::readFile(repo.getGitDirectoryPath() / "rebase-merge" / "orig-head");
}

auto RebaseFilesHelper::createStoppedShaFile(const std::string_view hash) const -> void
{
    _details::FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "rebase-merge" / "stopped-sha", hash);
}

auto RebaseFilesHelper::getStoppedShaFile() const -> std::string
{
    return _details::FileUtility::readFile(repo.getGitDirectoryPath() / "rebase-merge" / "stopped-sha");
}


auto RebaseFilesHelper::createCommitEditMsgFile(const std::string_view message) const -> void
{
    _details::FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "COMMIT_EDITMSG", message);
}

auto RebaseFilesHelper::getCommitEditMsgFile() const -> std::string
{
    return _details::FileUtility::readFile(repo.getGitDirectoryPath() / "COMMIT_EDITMSG");
}

auto RebaseFilesHelper::createAuthorScriptFile(const std::string_view authorName, const std::string_view authorEmail, const std::string_view authorDate) const -> void
{
    auto authorScript = "GIT_AUTHOR_NAME=" + std::string{ authorName } + "\n"
                      + "GIT_AUTHOR_EMAIL=" + std::string{ authorEmail } + "\n"
                      + "GIT_AUTHOR_DATE=" + std::string{ authorDate };
    _details::FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "rebase-merge" / "author-script", authorScript);
}

auto RebaseFilesHelper::getAuthorScriptFile() const -> std::vector<std::string>
{
    auto autorScript = _details::FileUtility::readFile(repo.getGitDirectoryPath() / "rebase-merge" / "author-script");

    return Parser::splitToStringsVector(autorScript, '\n');
}

auto RebaseFilesHelper::removeAuthorScriptFile() const -> void
{
    std::filesystem::remove(repo.getGitDirectoryPath() / "rebase-merge" / "author-script");
}


auto RebaseFilesHelper::createAmendFile(const std::string_view hash) const -> void
{
    _details::FileUtility::createOrOverwriteFile(repo.getGitDirectoryPath() / "rebase-merge" / "amend", hash);
}

auto RebaseFilesHelper::getAmendFile() const -> std::string
{
    return _details::FileUtility::readFile(repo.getGitDirectoryPath() / "rebase-merge" / "amend");
}

auto RebaseFilesHelper::generateTodoFile(const std::vector<RebaseTodoCommand>& rebaseTodoCommands) const -> void
{
    auto file = std::ofstream{ repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo" };
    for (const auto& command : rebaseTodoCommands)
    {
        file << command.toString() << "\n";
    }
    file.close();

    std::filesystem::copy(repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo", repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo.backup");
}

auto RebaseFilesHelper::peekTodoFile() const -> std::optional<RebaseTodoCommand>
{
    auto todoFilePath = repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo";
    auto todoFile = std::ifstream{ todoFilePath };
    std::string todoLine;
    std::getline(todoFile, todoLine);
    todoFile.close();

    auto commandTodo = parseTodoCommandLine(todoLine);

    const auto& hash = commandTodo.has_value() ? commandTodo.value().hash : "";
    const auto& message = commandTodo.has_value() ? commandTodo.value().message : "";

    return commandTodo;
}

auto RebaseFilesHelper::popTodoFile() const -> void
{
    auto todoFilePath = repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo";
    auto tempFilePath = repo.getGitDirectoryPath() / "rebase-merge" / "git-rebase-todo.temp";

    auto todoFile = std::ifstream{ todoFilePath };
    auto tempFile = std::ofstream{ tempFilePath };

    std::string line;
    std::getline(todoFile, line); // skip first line

    while (std::getline(todoFile, line))
    {
        tempFile << line << "\n";
    }

    todoFile.close();
    tempFile.close();

    std::filesystem::remove(todoFilePath);
    std::filesystem::rename(tempFilePath, todoFilePath);
}

auto RebaseFilesHelper::appendDoneFile(const RebaseTodoCommand& rebaseTodoCommand) const -> void
{
    _details::FileUtility::createOrAppendFile(repo.getGitDirectoryPath() / "rebase-merge" / "done", rebaseTodoCommand.toString(), "\n");
}

auto RebaseFilesHelper::getLastDoneCommand() const -> std::optional<RebaseTodoCommand>
{
    auto doneFile = std::ifstream{ repo.getGitDirectoryPath() / "rebase-merge" / "done", std::ios::in | std::ios::ate };

    doneFile.seekg(-2, std::ios::end); // -2, because we always have '\n' after each command
    char c;
    while (doneFile.tellg() > 0)
    {
        doneFile.get(c);
        if (c == '\n')
        {
            break;
        }
        doneFile.seekg(-2, std::ios::cur);
    }

    std::string lastLine;
    std::getline(doneFile, lastLine);

    if (lastLine.empty())
    {
        return std::nullopt;
    }

    return parseTodoCommandLine(lastLine);
}

auto RebaseFilesHelper::parseTodoCommandLine(const std::string_view line) -> std::optional<RebaseTodoCommand>
{
    if (line.empty())
    {
        return std::nullopt;
    }

    std::size_t startPos = 0;
    std::size_t endPos = 0;

    endPos = line.find(' ', startPos);
    auto command = std::string{ line.substr(startPos, endPos - startPos) };

    if (endPos == std::string::npos)
    {
        return RebaseTodoCommand{ RebaseTodoCommandTypeWrapper::fromString(command) };
    }

    startPos = endPos + 1;
    endPos = line.find(' ', startPos);
    auto commitHash = std::string{ line.substr(startPos, endPos - startPos) };

    auto message = std::string{ line.substr(endPos + 1) };

    return RebaseTodoCommand{ RebaseTodoCommandTypeWrapper::fromString(command), commitHash, message };
}

} // namespace CppGit::_details
