#include "BaseRepositoryFixture.hpp"
#include "Repository.hpp"

#include <fstream>
#include <sstream>

class InitRepositoryTests : public BaseRepositoryFixture
{
protected:
    void SetUp() override { }

    void checkGitFilesExistence(const std::filesystem::path& gitDir)
    {
        EXPECT_TRUE(std::filesystem::exists(gitDir / "objects"));
        EXPECT_TRUE(std::filesystem::exists(gitDir / "refs"));
        EXPECT_TRUE(std::filesystem::exists(gitDir / "refs" / "heads"));
        EXPECT_TRUE(std::filesystem::exists(gitDir / "refs" / "tags"));
        EXPECT_TRUE(std::filesystem::exists(gitDir / "HEAD"));
        EXPECT_TRUE(std::filesystem::exists(gitDir / "config"));
    }
};

TEST_F(InitRepositoryTests, BareRepository)
{
    repository = std::make_unique<CppGit::Repository>(repositoryPath);
    repository->initRepository(true);

    ASSERT_TRUE(std::filesystem::exists(repositoryPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git"));

    checkGitFilesExistence(repositoryPath);

    auto headFileContent = getFileContent(repositoryPath / "HEAD");
    EXPECT_EQ(headFileContent, "ref: refs/heads/main");

    auto configFileContent = getFileContent(repositoryPath / "config");
    EXPECT_EQ(configFileContent, "[core]\n\trepositoryformatversion = 0\n\tfilemode = true\n\tbare = true");
}

TEST_F(InitRepositoryTests, NonBareRepository)
{
    repository = std::make_unique<CppGit::Repository>(repositoryPath);
    repository->initRepository();

    ASSERT_TRUE(std::filesystem::exists(repositoryPath));
    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git"));

    auto gitDir = repositoryPath / ".git";

    checkGitFilesExistence(gitDir);

    auto headFileContent = getFileContent(gitDir / "HEAD");
    EXPECT_EQ(headFileContent, "ref: refs/heads/main");

    auto configFileContent = getFileContent(gitDir / "config");
    EXPECT_EQ(configFileContent, "[core]\n\trepositoryformatversion = 0\n\tfilemode = true\n\tbare = false\n\tlogallrefupdates = true\n");
}

TEST_F(InitRepositoryTests, BareRepository_NonDefaultMainBranchName)
{
    repository = std::make_unique<CppGit::Repository>(repositoryPath);
    repository->initRepository(false, "master");

    ASSERT_TRUE(std::filesystem::exists(repositoryPath));
    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git"));

    auto gitDir = repositoryPath / ".git";

    checkGitFilesExistence(gitDir);

    auto headFileContent = getFileContent(gitDir / "HEAD");
    EXPECT_EQ(headFileContent, "ref: refs/heads/master");
}

TEST_F(InitRepositoryTests, NonBareRepository_NonDefaultMainBranchName)
{
    repository = std::make_unique<CppGit::Repository>(repositoryPath);
    repository->initRepository(true, "master");

    ASSERT_TRUE(std::filesystem::exists(repositoryPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git"));

    checkGitFilesExistence(repositoryPath);

    auto headFileContent = getFileContent(repositoryPath / "HEAD");
    EXPECT_EQ(headFileContent, "ref: refs/heads/master");
}
