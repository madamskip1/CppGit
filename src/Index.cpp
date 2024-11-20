#include "Index.hpp"

#include "Commit.hpp"
#include "Commits.hpp"
#include "_details/Parser/IndexParser.hpp"

#include <algorithm>

namespace CppGit {

Index::Index(const Repository& repo)
    : repo(repo)
{
}

auto Index::add(const std::string_view filePattern) const -> void
{
    auto filesList = getUntrackedAndIndexFilesList(filePattern);

    if (filesList.empty())
    {
        throw std::runtime_error("Given file pattern did not match any files");
    }

    auto args = std::vector<std::string>{};
    args.reserve(2 + filesList.size()); // --add --remove + num of files

    args.emplace_back("--add");
    // we do remove also, even method name is `add`, because we want to mimic `git add` behavior
    // `git add` can also remove deleted files from index
    args.emplace_back("--remove");

    for (auto&& file : filesList)
    {
        args.emplace_back(std::move(file));
    }

    auto output = repo.executeGitCommand("update-index", args);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Error while updating index.");
    }
}

auto Index::remove(const std::string_view filePattern, bool force) const -> void
{
    auto filesList = getFilesInIndexList(filePattern);

    if (filesList.empty())
    {
        throw std::runtime_error("Given file pattern did not match any files");
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

    for (auto&& file : filesList)
    {
        args.emplace_back(std::move(file));
    }

    auto output = repo.executeGitCommand("update-index", args);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Error while updating index.");
    }
}

auto Index::restoreAllStaged() const -> void
{
    // TODO: Optimize it later - to many vector copies

    auto stagedFiles = getStagedFilesListWithStatus();

    if (stagedFiles.empty())
    {
        return;
    }

    auto args = std::vector<std::string>{};
    args.reserve(2 + (3 * stagedFiles.size())); // --force-remove + filesCount + --add  + [(--cache-info + fileInfo) * filesCount] = 2 + 3 * filesCount

    args.emplace_back("--force-remove");

    for (auto& file : stagedFiles)
    {
        if (file.status == DiffIndexStatus::ADDED)
        {
            args.emplace_back("--force-remove");
            args.push_back(std::move(file.path));
        }
    }

    auto filesInfoInHEADCommit = getHeadFilesHashForGivenFiles(stagedFiles);

    args.emplace_back("--add");

    for (auto& fileInfo : filesInfoInHEADCommit)
    {
        args.emplace_back("--cacheinfo");
        args.push_back(std::move(fileInfo));
    }

    auto output = repo.executeGitCommand("update-index", args);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to restore all staged files");
    }
}

auto Index::isFileStaged(const std::string_view file) const -> bool
{
    auto stagedFiles = getStagedFilesList(file);

    return stagedFiles.size() == 1 && stagedFiles[0] == file;
}

auto Index::getFilesInIndexList(const std::string_view filePattern) const -> std::vector<std::string>
{
    auto output = repo.executeGitCommand("ls-files", "--cache", "--", filePattern);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list staged files");
    }

    return IndexParser::parseStageSimpleCacheList(output.stdout);
}

auto Index::getFilesInIndexListWithDetails(const std::string_view filePattern) const -> std::vector<IndexEntry>
{
    auto output = repo.executeGitCommand("ls-files", "--stage", "--", filePattern);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list staged files");
    }

    return IndexParser::parseStageDetailedList(output.stdout);
}


auto Index::getUntrackedFilesList(const std::string_view filePattern) const -> std::vector<std::string>
{
    auto output = repo.executeGitCommand("ls-files", "--others", "--exclude-standard", "--", filePattern);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list untracked files");
    }

    if (output.stdout.empty())
    {
        return {};
    }

    auto splittedList_SV = Parser::splitToStringViewsVector(output.stdout, '\n');
    std::vector<std::string> untrackedFilesList;
    untrackedFilesList.reserve(splittedList_SV.size());

    for (const auto file : splittedList_SV)
    {
        untrackedFilesList.emplace_back(file);
    }

    return untrackedFilesList;
}

auto Index::getStagedFilesList(const std::string_view filePattern) const -> std::vector<std::string>
{
    auto output = repo.executeGitCommand("diff-index", "--cached", "--name-only", "HEAD", "--", filePattern);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list staged files");
    }

    if (output.stdout.empty())
    {
        return std::vector<std::string>{};
    }

    return Parser::splitToStringsVector(output.stdout, '\n');
}

auto Index::getStagedFilesListWithStatus(const std::string_view filePattern) const -> std::vector<DiffIndexEntry>
{
    auto output = repo.executeGitCommand("diff-index", "--cached", "--name-status", "HEAD", "--", filePattern);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list staged files");
    }

    return IndexParser::parseDiffIndexWithStatus(output.stdout);
}

auto Index::getNotStagedFilesList(const std::string_view filePattern) const -> std::vector<std::string>
{
    auto output = repo.executeGitCommand("ls-files", "--modified", "--deleted", "--others", "--exclude-standard", "--deduplicate", "--", filePattern);

    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to list not staged files");
    }

    if (output.stdout.empty())
    {
        return std::vector<std::string>{};
    }

    auto splittedSV = Parser::splitToStringViewsVector(output.stdout, '\n');

    return std::vector<std::string>{ splittedSV.cbegin(), splittedSV.cend() };
}

auto Index::getUnmergedFilesListWithDetails(const std::string_view filePattern) const -> std::vector<IndexEntry>
{
    auto output = repo.executeGitCommand("ls-files", "--unmerged", "--", filePattern);

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

auto Index::isDirty() const -> bool
{
    auto commits = repo.Commits();
    auto headCommit = commits.getCommitInfo("HEAD");

    auto output = repo.executeGitCommand("diff-index", "--quiet", "--exit-code", headCommit.getTreeHash());
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

    auto output = repo.executeGitCommand("ls-tree", args);

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
    auto output = repo.executeGitCommand("ls-files", "--others", "--cached", "--exclude-standard", "--", pattern);

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

} // namespace CppGit
