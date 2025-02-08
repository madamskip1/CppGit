#include "_details/GitCommandExecutor/GitCommandExecutor.hpp"

#include "_details/GitCommandExecutor/GitCommandOutput.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace CppGit {

auto GitCommandExecutor::execute(const std::vector<std::string>& environmentVariables, const std::string_view repoPath, const std::string_view command, const std::vector<std::string>& args) -> GitCommandOutput
{
    return executeImpl(environmentVariables, repoPath, command, args);
}

} // namespace CppGit
