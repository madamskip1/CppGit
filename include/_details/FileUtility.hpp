#pragma once

#include <fstream>
#include <sstream>

namespace CppGit::_details::FileUtility {

template <typename T, typename... Args>
inline auto createOrOverwriteFile(T&& path, Args&&... args) -> void
{
    auto file = std::ofstream{ std::forward<T>(path) };
    (file << ... << std::forward<Args>(args));
    file.close();
}

template <typename T, typename... Args>
inline auto createOrAppendFile(T&& path, Args&&... args) -> void
{
    auto file = std::ofstream{ std::forward<T>(path), std::ios::app };
    (file << ... << std::forward<Args>(args));
    file.close();
}

template <typename T>
inline auto readFile(T&& path)
{
    auto file = std::ifstream{ std::forward<T>(path) };
    auto buffer = std::ostringstream{};
    buffer << file.rdbuf();
    return buffer.str();
}

} // namespace CppGit::_details::FileUtility
