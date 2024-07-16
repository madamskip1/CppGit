#include "GitCommandExecutor.hpp"
#include "GitCommandOutput.hpp"
#include <array>

namespace CppGit
{
    class GitCommandExecutorUnix : public GitCommandExecutor
    {
    public:
        GitCommandExecutorUnix() = default;
        ~GitCommandExecutorUnix() override = default;
        GitCommandOutput execute(std::string_view command, std::string_view path = "") override;

    private:
        void createPipes();
        GitCommandOutput parentProcess();
        void childProcess(std::string_view command, std::string_view path);
        
        pid_t pid;
        std::array<int, 2> stdoutPipe;
        std::array<int, 2> stderrPipe;
    };
}