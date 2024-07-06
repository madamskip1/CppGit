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
        // TODO: change std::string_view path to CppGit::Repository or sth like that
        // Or even store Repository object in GitCommandExecutor
        GitCommandOutput exec(std::string_view path, std::string_view cmd) const;
        GitCommandOutput exec(std::string_view cmd) const;
        
    private:
        std::string buildCommand(std::string_view path, std::string_view cmd) const;

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