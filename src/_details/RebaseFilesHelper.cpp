#include "_details/RebaseFilesHelper.hpp"

#include "_details/FileUtility.hpp"

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

} // namespace CppGit
