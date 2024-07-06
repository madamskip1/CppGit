#include "GitCommandExecutor.hpp"
#include <memory>

namespace CppGit
{
    GitCommandOutput GitCommandExecutor::exec(std::string_view cmd, std::string_view path)
    {
        auto command = buildCommand(cmd, path);

        std::unique_ptr<FILE, PipeDeleter> pipe(popen(command.c_str(), "r"));

        if (!pipe)
        {
            return GitCommandOutput{ -1, "" };
        }

        char buffer[128];
        std::string result = "";

        while (!feof(pipe.get()))
        {
            if (fgets(buffer, 128, pipe.get()) != NULL)
            {
                result += buffer;
            }
        }
        
        int return_code = pclose(pipe.release());
        return GitCommandOutput{ return_code, result };
    }

    bool GitCommandExecutor::checkIfHasGit()
    {
        return exec("--version").return_code == 0;
    }

    std::string GitCommandExecutor::buildCommand(std::string_view cmd, std::string_view path)
    {
        std::string command = GIT_EXECUTABLE;
        if (!path.empty())
        {
            command += " -C ";
            command += path;
        }
        command += " ";
        command += cmd;
        return command;
    }

} // namespace CppGit