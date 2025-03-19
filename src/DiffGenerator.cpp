#include "CppGit/DiffGenerator.hpp"

#include "CppGit/DiffFile.hpp"
#include "CppGit/Repository.hpp"
#include "CppGit/_details/GitCommandExecutor/GitCommandOutput.hpp"
#include "CppGit/_details/Parser/DiffParser.hpp"

#include <filesystem>
#include <vector>

namespace CppGit {

DiffGenerator::DiffGenerator(const Repository& repository)
    : repository{ &repository }
{
}

auto DiffGenerator::getDiff() const -> std::vector<DiffFile>
{
    return getDiff("HEAD");
}

auto DiffGenerator::getDiff(const std::string_view commitHash) const -> std::vector<DiffFile>
{
    // git diff-tree -p --no-commit-id HEAD --full-index -- test2.txt
    const auto output = repository->executeGitCommand("diff-tree", "-p", "--no-commit-id", commitHash, "--full-index");

    return getDiffFilesFromOutput(output);
}

auto DiffGenerator::getDiff(const std::string_view commitHashA, const std::string_view commitHashB) const -> std::vector<DiffFile>
{
    const auto output = repository->executeGitCommand("diff-tree", "-p", "--no-commit-id", commitHashA, commitHashB, "--full-index");

    return getDiffFilesFromOutput(output);
}

auto DiffGenerator::getDiffFile(const std::filesystem::path& path) const -> std::vector<DiffFile>
{
    return getDiffFile("HEAD", path);
}

auto DiffGenerator::getDiffFile(const std::string_view commitHash, const std::filesystem::path& path) const -> std::vector<DiffFile>
{
    const auto output = repository->executeGitCommand("diff-tree", "-p", "--no-commit-id", commitHash, "--full-index", "--", path.string());

    return getDiffFilesFromOutput(output);
}

auto DiffGenerator::getDiffFile(const std::string_view commitHashA, const std::string_view commitHashB, const std::filesystem::path& path) const -> std::vector<DiffFile>
{
    const auto output = repository->executeGitCommand("diff-tree", "-p", "--no-commit-id", commitHashA, commitHashB, "--full-index", "--", path.string());

    return getDiffFilesFromOutput(output);
}

auto DiffGenerator::getDiffFilesFromOutput(const GitCommandOutput& output) -> std::vector<DiffFile>
{
    auto diffParser = DiffParser{};
    return diffParser.parse(output.stdout);
}
} // namespace CppGit
