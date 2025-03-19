#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>

/// @brief Contains file utility functions
namespace CppGit::_details::FileUtility {

/// @brief Create or overwrite file with given content
/// @tparam T File path type
/// @tparam Args Content types
/// @param path File path
/// @param args Content
template <typename T, typename... Args>
    requires std::convertible_to<T, std::filesystem::path>
inline auto createOrOverwriteFile(T&& path, Args&&... args) -> void
{
    auto file = std::ofstream{ std::forward<T>(path) };
    (file << ... << std::forward<Args>(args));
    file.close();
}

/// @brief Create or append file with given content
/// @tparam T File path type
/// @tparam Args Content types
/// @param path File path
/// @param args Content
template <typename T, typename... Args>
    requires std::convertible_to<T, std::filesystem::path>
inline auto createOrAppendFile(T&& path, Args&&... args) -> void
{
    auto file = std::ofstream{ std::forward<T>(path), std::ios::app };
    (file << ... << std::forward<Args>(args));
    file.close();
}

/// @brief Read file content
/// @tparam T File path type
/// @param path File path
/// @return File content
template <typename T>
    requires std::convertible_to<T, std::filesystem::path>
[[nodiscard]] inline auto readFile(T&& path)
{
    auto file = std::ifstream{ std::forward<T>(path) };
    auto buffer = std::ostringstream{};
    buffer << file.rdbuf();
    return buffer.str();
}

} // namespace CppGit::_details::FileUtility
