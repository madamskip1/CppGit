#pragma once

#include "DiffFile.hpp"
#include "Repository.hpp"
#include "_details/GitCommandExecutor/GitCommandOutput.hpp"

#include <filesystem>
#include <string_view>
#include <vector>

namespace CppGit {

class Diff
{
public:
    explicit Diff(const Repository& repo)
        : repo(repo)
    { }

    auto getDiff() const -> std::vector<DiffFile>;
    auto getDiff(const std::string_view commitHash) const -> std::vector<DiffFile>;
    auto getDiff(const std::string_view commitHashA, const std::string_view commitHashB) const -> std::vector<DiffFile>;
    auto getDiffFile(const std::filesystem::path& path) const -> std::vector<DiffFile>;
    auto getDiffFile(const std::string_view commitHash, const std::filesystem::path& path) const -> std::vector<DiffFile>;
    auto getDiffFile(const std::string_view commitHashA, const std::string_view commitHashB, const std::filesystem::path& path) const -> std::vector<DiffFile>;

private:
    const Repository& repo;

    static auto getDiffFilesFromOutput(const GitCommandOutput& output) -> std::vector<DiffFile>;
};
} // namespace CppGit
