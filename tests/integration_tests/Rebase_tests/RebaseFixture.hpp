#include "../BaseRepositoryFixture.hpp"

class RebaseFixture : public BaseRepositoryFixture
{
protected:
    std::filesystem::path rebaseDirPath = repositoryPath / ".git" / "rebase-merge";
    std::string expectedAuthorScript = std::string{ "GIT_AUTHOR_NAME=" } + AUTHOR_NAME + "\n"
                                     + "GIT_AUTHOR_EMAIL=" + AUTHOR_EMAIL + "\n"
                                     + "GIT_AUTHOR_DATE=" + AUTHOR_DATE_WITH_TIMEZONE;
};
