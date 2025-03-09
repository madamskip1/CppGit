#include "CppGit/Diff.hpp"

#include "CppGit/DiffFile.hpp"
#include "CppGit/_details/GitCommandExecutor/GitCommandOutput.hpp"
#include "CppGit/_details/Parser/DiffParser.hpp"

#include <filesystem>
#include <stdexcept>
#include <vector>

namespace CppGit {

Diff::Diff(const Repository& repo)
    : repo(&repo)
{
}

auto Diff::getDiff() const -> std::vector<DiffFile>
{
    return getDiff("HEAD");
}

auto Diff::getDiff(const std::string_view commitHash) const -> std::vector<DiffFile>
{
    // git diff-tree -p --no-commit-id HEAD --full-index -- test2.txt
    const auto output = repo->executeGitCommand("diff-tree", "-p", "--no-commit-id", commitHash, "--full-index");

    return getDiffFilesFromOutput(output);
}

auto Diff::getDiff(const std::string_view commitHashA, const std::string_view commitHashB) const -> std::vector<DiffFile>
{
    const auto output = repo->executeGitCommand("diff-tree", "-p", "--no-commit-id", commitHashA, commitHashB, "--full-index");

    return getDiffFilesFromOutput(output);
}

auto Diff::getDiffFile(const std::filesystem::path& path) const -> std::vector<DiffFile>
{
    return getDiffFile("HEAD", path);
}

auto Diff::getDiffFile(const std::string_view commitHash, const std::filesystem::path& path) const -> std::vector<DiffFile>
{
    const auto output = repo->executeGitCommand("diff-tree", "-p", "--no-commit-id", commitHash, "--full-index", "--", path.string());

    return getDiffFilesFromOutput(output);
}

auto Diff::getDiffFile(const std::string_view commitHashA, const std::string_view commitHashB, const std::filesystem::path& path) const -> std::vector<DiffFile>
{
    const auto output = repo->executeGitCommand("diff-tree", "-p", "--no-commit-id", commitHashA, commitHashB, "--full-index", "--", path.string());

    return getDiffFilesFromOutput(output);
}

auto Diff::getDiffFilesFromOutput(const GitCommandOutput& output) -> std::vector<DiffFile>
{
    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to get diff");
    }

    auto diffParser = DiffParser{};
    const auto diffFiles = diffParser.parse(output.stdout);

    return diffFiles;
}
} // namespace CppGit
