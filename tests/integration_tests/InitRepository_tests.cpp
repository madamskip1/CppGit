#include "BaseRepositoryFixture.hpp"
#include "Repository.hpp"

#include <fstream>
#include <sstream>

class InitRepositoryTests : public BaseRepositoryFixture
{
protected:
    void SetUp() override { }
};

TEST_F(InitRepositoryTests, BareRepository)
{
    repository = std::make_unique<CppGit::Repository>(repositoryPath);
    repository->initRepository(true);

    ASSERT_TRUE(std::filesystem::exists(repositoryPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "objects"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "refs"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "refs" / "heads"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "refs" / "tags"));

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / "HEAD"));
    std::ifstream headFile(repositoryPath / "HEAD");
    std::stringstream headContent;
    headContent << headFile.rdbuf();
    EXPECT_EQ(headContent.str(), "ref: refs/heads/main");
    headFile.close();

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / "config"));
    std::ifstream config(repositoryPath / "config");
    std::stringstream configContent;
    configContent << config.rdbuf();
    EXPECT_EQ(configContent.str(), "[core]\n\trepositoryformatversion = 0\n\tfilemode = true\n\tbare = true");
    config.close();
}

TEST_F(InitRepositoryTests, NonBareRepository)
{
    repository = std::make_unique<CppGit::Repository>(repositoryPath);
    repository->initRepository();

    ASSERT_TRUE(std::filesystem::exists(repositoryPath));
    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git"));

    auto gitDir = repositoryPath / ".git";
    EXPECT_TRUE(std::filesystem::exists(gitDir / "objects"));
    EXPECT_TRUE(std::filesystem::exists(gitDir / "refs"));
    EXPECT_TRUE(std::filesystem::exists(gitDir / "refs" / "heads"));
    EXPECT_TRUE(std::filesystem::exists(gitDir / "refs" / "tags"));

    ASSERT_TRUE(std::filesystem::exists(gitDir / "HEAD"));
    std::ifstream headFile(gitDir / "HEAD");
    std::stringstream headContent;
    headContent << headFile.rdbuf();
    EXPECT_EQ(headContent.str(), "ref: refs/heads/main");
    headFile.close();

    ASSERT_TRUE(std::filesystem::exists(gitDir / "config"));
    std::ifstream config(gitDir / "config");
    std::stringstream configContent;
    configContent << config.rdbuf();
    EXPECT_EQ(configContent.str(), "[core]\n\trepositoryformatversion = 0\n\tfilemode = true\n\tbare = false\n\tlogallrefupdates = true\n");
}

TEST_F(InitRepositoryTests, BareRepository_NonDefaultMainBranchName)
{
    repository = std::make_unique<CppGit::Repository>(repositoryPath);
    repository->initRepository(false, "master");

    ASSERT_TRUE(std::filesystem::exists(repositoryPath));
    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git"));

    auto gitDir = repositoryPath / ".git";
    ASSERT_TRUE(std::filesystem::exists(gitDir / "HEAD"));
    std::ifstream headFile(gitDir / "HEAD");
    std::stringstream headContent;
    headContent << headFile.rdbuf();
    EXPECT_EQ(headContent.str(), "ref: refs/heads/master");
    headFile.close();
}

TEST_F(InitRepositoryTests, NonBareRepository_NonDefaultMainBranchName)
{
    repository = std::make_unique<CppGit::Repository>(repositoryPath);
    repository->initRepository(true, "master");

    ASSERT_TRUE(std::filesystem::exists(repositoryPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "objects"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "refs"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "refs" / "heads"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "refs" / "tags"));

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / "HEAD"));
    std::ifstream headFile(repositoryPath / "HEAD");
    std::stringstream headContent;
    headContent << headFile.rdbuf();
    EXPECT_EQ(headContent.str(), "ref: refs/heads/master");
    headFile.close();
}
