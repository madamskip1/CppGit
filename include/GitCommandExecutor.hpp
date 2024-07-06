#pragma once

#include <string>
#include <string_view>

namespace CppGit 
{
    struct GitCommandOutput
    {
        int return_code;
        std::string output;
    };

    constexpr const char* const GIT_EXECUTABLE = "git";

    class GitCommandExecutor
    {
    public:
        static GitCommandOutput execute(std::string_view cmd, std::string_view path = "");
        static bool checkIfHasGit();

    private:
        static std::string buildCommand(std::string_view cmd, std::string_view path = "");

        struct PipeDeleter {
            void operator()(FILE* pipe) const {
                if (pipe) {
                    pclose(pipe);
                }
            }
        };
    };
} // namespace CppGit