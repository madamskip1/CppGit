#include "../BaseRepositoryFixture.hpp"

class RebaseFixture : public BaseRepositoryFixture
{
protected:
    std::filesystem::path rebaseDirPath = repositoryPath / ".git" / "rebase-merge";
};
