#pragma once

#include "DiffFile.hpp"
#include "Repository.hpp"
#include "_details/GitCommandExecutor/GitCommandOutput.hpp"

#include <filesystem>
#include <string_view>
#include <vector>

namespace CppGit {

/// @brief Provides functionality to get the diff of given commits or files
class DiffGenerator
{
public:
    /// @param repo The repository to work with
    explicit DiffGenerator(const Repository& repository);

    /// @brief Get the diff of last commit
    /// @return A vector of DiffFile

    [[nodiscard]] auto getDiff() const -> std::vector<DiffFile>;

    /// @brief Get the diff of a commit
    /// @param commitHash The hash of the commit
    /// @return A vector of DiffFile
    [[nodiscard]] auto getDiff(const std::string_view commitHash) const -> std::vector<DiffFile>;

    /// @brief Get the diff between two commits
    /// @param commitHashA The hash of the first commit
    /// @param commitHashB The hash of the second commit
    /// @return A vector of DiffFile
    [[nodiscard]] auto getDiff(const std::string_view commitHashA, const std::string_view commitHashB) const -> std::vector<DiffFile>;

    /// @brief Get the diff of a file in the last commit
    /// @param path The path of the file
    /// @return A vector of DiffFile
    [[nodiscard]] auto getDiffFile(const std::filesystem::path& path) const -> std::vector<DiffFile>;

    /// @brief Get the diff of a file in a commit
    /// @param commitHash The hash of the commit
    /// @param path The path of the file
    /// @return A vector of DiffFile
    [[nodiscard]] auto getDiffFile(const std::string_view commitHash, const std::filesystem::path& path) const -> std::vector<DiffFile>;

    /// @brief Get the diff of a file between two commits
    /// @param commitHashA The hash of the first commit
    /// @param commitHashB The hash of the second commit
    /// @param path The path of the file
    /// @return A vector of DiffFile
    [[nodiscard]] auto getDiffFile(const std::string_view commitHashA, const std::string_view commitHashB, const std::filesystem::path& path) const -> std::vector<DiffFile>;

private:
    const Repository* repository;

    static auto getDiffFilesFromOutput(const GitCommandOutput& output) -> std::vector<DiffFile>;
};
} // namespace CppGit
