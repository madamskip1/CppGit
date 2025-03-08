#include "BaseRepositoryFixture.hpp"
#include "CppGit/Repository.hpp"
#include "CppGit/_details/FileUtility.hpp"


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
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "HEAD"), "ref: refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "config"), "[core]\n\trepositoryformatversion = 0\n\tfilemode = true\n\tbare = true");
}

TEST_F(InitRepositoryTests, NonBareRepository)
{
    const auto gitDir = repositoryPath / ".git";


    repository = std::make_unique<CppGit::Repository>(repositoryPath);
    repository->initRepository();


    ASSERT_TRUE(std::filesystem::exists(repositoryPath));
    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git"));
    checkGitFilesExistence(gitDir);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitDir / "HEAD"), "ref: refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitDir / "config"), "[core]\n\trepositoryformatversion = 0\n\tfilemode = true\n\tbare = false\n\tlogallrefupdates = true\n");
}

TEST_F(InitRepositoryTests, BareRepository_NonDefaultMainBranchName)
{
    const auto gitDir = repositoryPath / ".git";


    repository = std::make_unique<CppGit::Repository>(repositoryPath);
    repository->initRepository(false, "master");


    ASSERT_TRUE(std::filesystem::exists(repositoryPath));
    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git"));
    checkGitFilesExistence(gitDir);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(gitDir / "HEAD"), "ref: refs/heads/master");
}

TEST_F(InitRepositoryTests, NonBareRepository_NonDefaultMainBranchName)
{
    repository = std::make_unique<CppGit::Repository>(repositoryPath);
    repository->initRepository(true, "master");


    ASSERT_TRUE(std::filesystem::exists(repositoryPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git"));
    checkGitFilesExistence(repositoryPath);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "HEAD"), "ref: refs/heads/master");
}
