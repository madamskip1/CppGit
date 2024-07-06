#include <string>
#include <string_view>

namespace CppGit 
{
    struct GitCommandOutput
    {
        int return_code;
        std::string output;
    };
    

    class GitCommandExecutor
    {
    public:
        static GitCommandOutput exec(std::string_view cmd, std::string_view path = "");
        static bool checkIfHasGit();
        
    private:
        static std::string buildCommand(std::string_view cmd, std::string_view path = "");

        static constexpr const char* const GIT_EXECUTABLE = "git";
        
        struct PipeDeleter {
            void operator()(FILE* pipe) const {
                if (pipe) {
                    pclose(pipe);
                }
            }
        };
    };
} // namespace CppGit