#include "Diff.hpp"

#include "Parser/DiffParser.hpp"
#include "Repository.hpp"

namespace CppGit {
auto Diff::getDiff() const -> std::vector<DiffFile>
{
    return getDiff("HEAD");
}

auto Diff::getDiff(const std::string_view commitHash) const -> std::vector<DiffFile>
{
    // git diff-tree -p --no-commit-id HEAD --full-index -- test2.txt

    auto output = repo.executeGitCommand("diff-tree", "-p", "--no-commit-id", commitHash, "--full-index");

    return getDiffFilesFromOutput(output);
}

auto Diff::getDiff(const std::string_view commitHashA, const std::string_view commitHashB) const -> std::vector<DiffFile>
{
    auto output = repo.executeGitCommand("diff-tree", "-p", "--no-commit-id", commitHashA, commitHashB, "--full-index");

    return getDiffFilesFromOutput(output);
}

auto Diff::getDiffFile(const std::filesystem::path& path) const -> std::vector<DiffFile>
{
    return getDiffFile("HEAD", path);
}

auto Diff::getDiffFile(const std::string_view commitHash, const std::filesystem::path& path) const -> std::vector<DiffFile>
{
    auto output = repo.executeGitCommand("diff-tree", "-p", "--no-commit-id", commitHash, "--full-index", "--", path.string());

    return getDiffFilesFromOutput(output);
}

auto Diff::getDiffFile(const std::string_view commitHashA, const std::string_view commitHashB, const std::filesystem::path& path) const -> std::vector<DiffFile>
{
    auto output = repo.executeGitCommand("diff-tree", "-p", "--no-commit-id", commitHashA, commitHashB, "--full-index", "--", path.string());

    return getDiffFilesFromOutput(output);
}

auto Diff::getDiffFilesFromOutput(const GitCommandOutput& output) -> std::vector<DiffFile>
{
    if (output.return_code != 0)
    {
        throw std::runtime_error("Failed to get diff");
    }
    auto diffParser = DiffParser{};
    auto diffFiles = diffParser.parse(output.stdout);

    return diffFiles;
}
} // namespace CppGit
