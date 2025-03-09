#include "CppGit/Index.hpp"

#include "CppGit/Commit.hpp"
#include "CppGit/Commits.hpp"
#include "CppGit/Error.hpp"
#include "CppGit/Repository.hpp"
#include "CppGit/_details/Parser/IndexParser.hpp"
#include "CppGit/_details/Parser/Parser.hpp"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace CppGit {

Index::Index(const Repository& repo)
    : repo(&repo)
{
}

auto Index::add(const std::string_view filePattern) const -> Error
{
    auto filesList = getUntrackedAndIndexFilesList(filePattern);

    if (filesList.empty())
    {
        return Error::PATTERN_NOT_MATCHING_ANY_FILES;
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

    const auto output = repo->executeGitCommand("update-index", std::move(args));

    if (output.return_code != 0)
    {
        throw std::runtime_error("Error while updating index.");
    }

    return Error::NO_ERROR;
}

auto Index::remove(const std::string_view filePattern, const bool force) const -> Error
{
    auto filesList = getFilesInIndexList(filePattern);

    if (filesList.empty())
    {
        return Error::PATTERN_NOT_MATCHING_ANY_FILES;
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

    const auto output = repo->executeGitCommand("update-index", std::move(args));

    if (output.return_code != 0)
    {
        throw std::runtime_error("Error while updating index.");
    }

    return Error::NO_ERROR;
}

auto Index::restoreAllStaged() const -> void
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

    const auto output = repo->executeGitCommand("update-index", std::move(args));

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to restore all staged files");
    }
}

auto Index::isFileStaged(const std::string_view file) const -> bool
{
    const auto stagedFiles = getStagedFilesList(file);

    return stagedFiles.size() == 1 && stagedFiles[0] == file;
}

auto Index::getFilesInIndexList(const std::string_view filePattern) const -> std::vector<std::string>
{
    const auto output = repo->executeGitCommand("ls-files", "--cache", "--", filePattern);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list staged files");
    }

    return IndexParser::parseStageSimpleCacheList(output.stdout);
}

auto Index::getFilesInIndexListWithDetails(const std::string_view filePattern) const -> std::vector<IndexEntry>
{
    const auto output = repo->executeGitCommand("ls-files", "--stage", "--", filePattern);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list staged files");
    }

    return IndexParser::parseStageDetailedList(output.stdout);
}


auto Index::getUntrackedFilesList(const std::string_view filePattern) const -> std::vector<std::string>
{
    const auto output = repo->executeGitCommand("ls-files", "--others", "--exclude-standard", "--", filePattern);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list untracked files");
    }

    if (output.stdout.empty())
    {
        return {};
    }

    return Parser::splitToStringsVector(output.stdout, '\n');
}

auto Index::getStagedFilesList(const std::string_view filePattern) const -> std::vector<std::string>
{
    const auto output = getStagedFilesListOutput(filePattern);

    if (output.empty())
    {
        return std::vector<std::string>{};
    }

    return Parser::splitToStringsVector(output, '\n');
}

auto Index::getStagedFilesListWithStatus(const std::string_view filePattern) const -> std::vector<DiffIndexEntry>
{
    const auto output = repo->executeGitCommand("diff-index", "--cached", "--name-status", "HEAD", "--", filePattern);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list staged files");
    }

    return IndexParser::parseDiffIndexWithStatus(output.stdout);
}

auto Index::getNotStagedFilesList(const std::string_view filePattern) const -> std::vector<std::string>
{
    const auto output = repo->executeGitCommand("ls-files", "--modified", "--deleted", "--others", "--exclude-standard", "--deduplicate", "--", filePattern);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list not staged files");
    }

    if (output.stdout.empty())
    {
        return std::vector<std::string>{};
    }

    return Parser::splitToStringsVector(output.stdout, '\n');
}

auto Index::getUnmergedFilesListWithDetails(const std::string_view filePattern) const -> std::vector<IndexEntry>
{
    const auto output = repo->executeGitCommand("ls-files", "--unmerged", "--", filePattern);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list unmerged files");
    }

    if (output.stdout.empty())
    {
        return std::vector<IndexEntry>{};
    }

    return IndexParser::parseStageDetailedList(output.stdout);
}

auto Index::areAnyStagedFiles() const -> bool
{
    const auto output = getStagedFilesListOutput();

    return !output.empty();
}

auto Index::areAnyNotStagedTrackedFiles() const -> bool
{
    const auto output = repo->executeGitCommand("ls-files", "--modified", "--deleted", "--exclude-standard", "--deduplicate");

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to check if there are any not staged tracked files");
    }

    return !output.stdout.empty();
}

auto Index::isDirty() const -> bool
{
    const auto commits = repo->Commits();
    const auto headCommit = commits.getCommitInfo("HEAD");

    auto output = repo->executeGitCommand("diff-index", "--quiet", "--exit-code", headCommit.getTreeHash());
    if (output.return_code > 1)
    {
        throw std::runtime_error("Failed to check if index is dirty");
    }

    return output.return_code == 1;
}


auto Index::getHeadFilesHashForGivenFiles(std::vector<DiffIndexEntry>& files) const -> std::vector<std::string>
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

    const auto output = repo->executeGitCommand("ls-tree", std::move(args));

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to get head files hash");
    }

    if (output.stdout.empty())
    {
        return {};
    }

    return Parser::splitToStringsVector(output.stdout, '\n');
}

auto Index::getUntrackedAndIndexFilesList(const std::string_view pattern) const -> std::vector<std::string>
{
    const auto output = repo->executeGitCommand("ls-files", "--others", "--cached", "--exclude-standard", "--", pattern);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list untracked and index files");
    }

    if (output.stdout.empty())
    {
        return std::vector<std::string>{};
    }

    return Parser::splitToStringsVector(output.stdout, '\n');
}

auto Index::getStagedFilesListOutput(const std::string_view filePattern) const -> std::string
{
    const auto output = repo->executeGitCommand("diff-index", "--cached", "--name-only", "HEAD", "--", filePattern);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list staged files");
    }

    return output.stdout;
}

} // namespace CppGit
