#include "BaseRepositoryFixture.hpp"
#include "Branches.hpp"
#include "CherryPick.hpp"
#include "Commit.hpp"
#include "Commits.hpp"
#include "GitCommandExecutor/GitCommandExecutorUnix.hpp"
#include "Index.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

class CherryPickTests : public BaseRepositoryFixture
{
};

TEST_F(CherryPickTests, simpleCherryPick)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto cherryPick = repository->CherryPick();
    auto commandExecutor = CppGit::GitCommandExecutorUnix{};

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();

    index.add("file.txt");

    commits.createCommit("Initial commit");

    branches.createBranch("second-branch");

    file.open(repositoryPath / "file.txt");
    file << "Hello, World! Modified";
    file.close();
    index.add("file.txt");

    auto env = std::vector<std::string>{ "GIT_AUTHOR_NAME=TestAuthor", "GIT_AUTHOR_EMAIL=test@email.com", "GIT_AUTHOR_DATE=1730738278 +0100", "GIT_COMMITTER_NAME=TestAuthor", "GIT_COMMITTER_EMAIL=test@email.com", "GIT_COMMITTER_DATE=1730738278 +0100" };
    auto output = commandExecutor.execute(env, repositoryPath.string(), "commit", "-m", "Initial commit", "--no-gpg-sign");

    auto mainBranchCommitHash = commits.getHeadCommitHash();
    auto mainBranchCommitInfo = commits.getCommitInfo(mainBranchCommitHash);

    EXPECT_EQ(mainBranchCommitInfo.getMessage(), "Initial commit");
    EXPECT_EQ(mainBranchCommitInfo.getAuthor().name, "TestAuthor");
    EXPECT_EQ(mainBranchCommitInfo.getAuthor().email, "test@email.com");
    EXPECT_EQ(mainBranchCommitInfo.getAuthorDate(), "1730738278 +0100");
    EXPECT_EQ(mainBranchCommitInfo.getCommitter().name, "TestAuthor");
    EXPECT_EQ(mainBranchCommitInfo.getCommitter().email, "test@email.com");
    EXPECT_EQ(mainBranchCommitInfo.getCommitterDate(), "1730738278 +0100");

    branches.changeCurrentBranch("second-branch");

    cherryPick.cherryPickCommit(mainBranchCommitHash);

    auto secondBranchCommitHash = commits.getHeadCommitHash();
    auto secondBranchCommitInfo = commits.getCommitInfo(secondBranchCommitHash);

    ASSERT_NE(mainBranchCommitHash, secondBranchCommitHash);
    EXPECT_EQ(secondBranchCommitInfo.getMessage(), "Initial commit");
    EXPECT_EQ(secondBranchCommitInfo.getAuthor().name, "TestAuthor");
    EXPECT_EQ(secondBranchCommitInfo.getAuthor().email, "test@email.com");
    EXPECT_EQ(secondBranchCommitInfo.getAuthorDate(), "1730738278 +0100");
    EXPECT_NE(secondBranchCommitInfo.getCommitter().name, "TestAuthor");
    EXPECT_NE(secondBranchCommitInfo.getCommitter().email, "test@email.com");
    EXPECT_NE(secondBranchCommitInfo.getCommitterDate(), "1730738278 +0100");

    std::ifstream fileRead(repositoryPath / "file.txt");
    std::ostringstream fileContentStream;
    fileContentStream << fileRead.rdbuf();
    EXPECT_EQ(fileContentStream.str(), "Hello, World! Modified");
}
