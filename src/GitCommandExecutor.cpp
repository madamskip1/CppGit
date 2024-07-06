#include "GitCommandExecutor.hpp"
#include <memory>

namespace CppGit
{
    GitCommandOutput GitCommandExecutor::exec(std::string_view path, std::string_view cmd) const
    {
        auto command = buildCommand(path, cmd);

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

    GitCommandOutput GitCommandExecutor::exec(std::string_view cmd) const
    {
        return exec("", cmd);
    }

    std::string GitCommandExecutor::buildCommand(std::string_view path, std::string_view cmd) const
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