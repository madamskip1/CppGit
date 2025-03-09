#include "../Repository.hpp"

namespace CppGit::_details {

class GitFilesHelper
{
public:
    GitFilesHelper(const Repository& repo);

    auto setOrigHeadFile(const std::string_view commitHash) const -> void;

private:
    const Repository* repo;
};

} // namespace CppGit::_details
