#include "CppGit/IndexManager.hpp"

#include "CppGit/CommitsManager.hpp"
#include "CppGit/Repository.hpp"
#include "CppGit/_details/Parser/IndexParser.hpp"
#include "CppGit/_details/Parser/Parser.hpp"

#include <algorithm>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace CppGit {

IndexManager::IndexManager(const Repository& repository)
    : repository{ &repository }
{
}

auto IndexManager::add(const std::string_view filePattern) const -> void
{
    auto filesList = getUntrackedAndIndexFilesList(filePattern);

    if (filesList.empty())
    {
        return;
    }

    auto args = std::vector<std::string>{};
    args.reserve(2 + filesList.size()); // --add --remove + num of files

    args.emplace_back("--add");
    // we do remove also, even method name is `add`, because we want to mimic `git add` behavior
    // `git add` can also remove deleted files from index
    args.emplace_back("--remove");

    std::transform(std::make_move_iterator(filesList.begin()),
                   std::make_move_iterator(filesList.end()),
                   std::back_inserter(args),
                   [](auto&& file) {
                       return std::forward<decltype(file)>(file);
                   });

    repository->executeGitCommand("update-index", std::move(args));
}

auto IndexManager::remove(const std::string_view filePattern, const bool force) const -> void
{
    auto filesList = getFilesInIndexList(filePattern);

    if (filesList.empty())
    {
        return;
    }

    auto args = std::vector<std::string>{};
    args.reserve(1 + filesList.size()); // --remove/--force-remove + num of files

    if (force)
    {
        args.emplace_back("--force-remove");
    }
    else
    {
        args.emplace_back("--remove");
    }

    std::transform(std::make_move_iterator(filesList.begin()),
                   std::make_move_iterator(filesList.end()),
                   std::back_inserter(args),
                   [](auto&& file) {
                       return std::forward<decltype(file)>(file);
                   });

    repository->executeGitCommand("update-index", std::move(args));
}

auto IndexManager::restoreAllStaged() const -> void
{
    auto stagedFiles = getStagedFilesListWithStatus();

    if (stagedFiles.empty())
    {
        return;
    }

    auto args = std::vector<std::string>{};
    args.reserve(2 + (3 * stagedFiles.size())); // --force-remove + filesCount + --add  + [(--cache-info + fileInfo) * filesCount] = 2 + 3 * filesCount

    args.emplace_back("--force-remove");

    for (const auto& file : stagedFiles)
    {
        if (file.status == DiffIndexStatus::ADDED)
        {
            args.emplace_back("--force-remove");
            args.push_back(file.path);
        }
    }

    auto filesInfoInHEADCommit = getHeadFilesHashForGivenFiles(stagedFiles);

    args.emplace_back("--add");

    for (auto& fileInfo : filesInfoInHEADCommit)
    {
        args.emplace_back("--cacheinfo");
        args.push_back(std::move(fileInfo));
    }

    repository->executeGitCommand("update-index", std::move(args));
}

auto IndexManager::isFileStaged(const std::string_view file) const -> bool
{
    const auto stagedFiles = getStagedFilesList(file);

    return stagedFiles.size() == 1 && stagedFiles[0] == file;
}

auto IndexManager::getFilesInIndexList(const std::string_view filePattern) const -> std::vector<std::string>
{
    const auto output = repository->executeGitCommand("ls-files", "--cache", "--", filePattern);

    return IndexParser::parseCacheFilenameList(output.stdout);
}

auto IndexManager::getFilesInIndexDetailedList(const std::string_view filePattern) const -> std::vector<IndexEntry>
{
    const auto output = repository->executeGitCommand("ls-files", "--stage", "--", filePattern);

    return IndexParser::parseStageDetailedList(output.stdout);
}


auto IndexManager::getUntrackedFilesList(const std::string_view filePattern) const -> std::vector<std::string>
{
    const auto output = repository->executeGitCommand("ls-files", "--others", "--exclude-standard", "--", filePattern);

    if (output.stdout.empty())
    {
        return {};
    }

    return Parser::splitToStringsVector(output.stdout, '\n');
}

auto IndexManager::getStagedFilesList(const std::string_view filePattern) const -> std::vector<std::string>
{
    const auto output = getStagedFilesListOutput(filePattern);

    if (output.empty())
    {
        return std::vector<std::string>{};
    }

    return Parser::splitToStringsVector(output, '\n');
}

auto IndexManager::getStagedFilesListWithStatus(const std::string_view filePattern) const -> std::vector<DiffIndexEntry>
{
    const auto output = repository->executeGitCommand("diff-index", "--cached", "--name-status", "HEAD", "--", filePattern);
    return IndexParser::parseDiffIndexWithStatusList(output.stdout);
}

auto IndexManager::getNotStagedFilesList(const std::string_view filePattern) const -> std::vector<std::string>
{
    const auto output = repository->executeGitCommand("ls-files", "--modified", "--deleted", "--others", "--exclude-standard", "--deduplicate", "--", filePattern);

    if (output.stdout.empty())
    {
        return std::vector<std::string>{};
    }

    return Parser::splitToStringsVector(output.stdout, '\n');
}

auto IndexManager::getUnmergedFilesDetailedList(const std::string_view filePattern) const -> std::vector<IndexEntry>
{
    const auto output = repository->executeGitCommand("ls-files", "--unmerged", "--", filePattern);

    if (output.stdout.empty())
    {
        return std::vector<IndexEntry>{};
    }

    return IndexParser::parseStageDetailedList(output.stdout);
}

auto IndexManager::areAnyStagedFiles() const -> bool
{
    const auto output = getStagedFilesListOutput();
    return !output.empty();
}

auto IndexManager::areAnyNotStagedTrackedFiles() const -> bool
{
    const auto output = repository->executeGitCommand("ls-files", "--modified", "--deleted", "--exclude-standard", "--deduplicate");
    return !output.stdout.empty();
}

auto IndexManager::isDirty() const -> bool
{
    const auto commitsManager = repository->CommitsManager();
    const auto headCommit = commitsManager.getCommitInfo("HEAD");

    auto output = repository->executeGitCommand("diff-index", "--quiet", "--exit-code", headCommit.getTreeHash());
    return output.return_code == 1;
}


auto IndexManager::getHeadFilesHashForGivenFiles(std::vector<DiffIndexEntry>& files) const -> std::vector<std::string>
{
    auto args = std::vector<std::string>{};
    constexpr auto CMD_ARGS_SIZE = 5;

    args.reserve(CMD_ARGS_SIZE + files.size());

    args.emplace_back("-r");
    args.emplace_back("--full-name");
    args.emplace_back("--format=%(objectmode),%(objectname),%(path)");
    args.emplace_back("HEAD");
    args.emplace_back("--");

    for (auto& file : files)
    {
        if (file.status != DiffIndexStatus::ADDED)
        {
            args.emplace_back(std::move(file.path));
        }
    }

    const auto output = repository->executeGitCommand("ls-tree", std::move(args));

    if (output.stdout.empty())
    {
        return {};
    }

    return Parser::splitToStringsVector(output.stdout, '\n');
}

auto IndexManager::getUntrackedAndIndexFilesList(const std::string_view pattern) const -> std::vector<std::string>
{
    const auto output = repository->executeGitCommand("ls-files", "--others", "--cached", "--exclude-standard", "--", pattern);

    if (output.stdout.empty())
    {
        return std::vector<std::string>{};
    }

    return Parser::splitToStringsVector(output.stdout, '\n');
}

auto IndexManager::getStagedFilesListOutput(const std::string_view filePattern) const -> std::string
{
    if (!repository->CommitsManager().hasAnyCommits())
    {
        auto output = repository->executeGitCommand("ls-files", "--cached", "--", filePattern);
        return std::move(output.stdout);
    }

    auto output = repository->executeGitCommand("diff-index", "--cached", "--name-only", "HEAD", "--", filePattern);
    return std::move(output.stdout);
}

} // namespace CppGit
