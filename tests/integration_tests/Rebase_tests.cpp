#include "BaseRepositoryFixture.hpp"
#include "Branches.hpp"
#include "Commits.hpp"
#include "CommitsHistory.hpp"
#include "Index.hpp"
#include "Rebase.hpp"

#include <gtest/gtest.h>

class RebaseTests : public BaseRepositoryFixture
{
};


TEST_F(RebaseTests, SimpleRebase)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    auto initialCommit = commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    createOrOverwriteFile(repositoryPath / "file1.txt", "");
    index.add("file1.txt");
    auto secondCommit = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    createOrOverwriteFile(repositoryPath / "file2.txt", "");
    index.add("file2.txt");
    auto thirdCommit = commits.createCommit("Third commit");
    auto fourthCommit = commits.createCommit("Fourth commit");

    rebase.rebase("main");


    EXPECT_EQ(branches.getCurrentBranch(), "refs/heads/second_branch");
    auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 4);
    EXPECT_EQ(commitsLog[0].getMessage(), "Initial commit");
    EXPECT_EQ(commitsLog[0].getHash(), initialCommit);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getHash(), secondCommit);
    EXPECT_EQ(commitsLog[2].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[3].getMessage(), "Fourth commit");
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));
}

TEST_F(RebaseTests, rebaseConflict)
{
    auto commits = repository->Commits();
    auto branches = repository->Branches();
    auto rebase = repository->Rebase();
    auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    commits.createCommit("Initial commit");
    branches.createBranch("second_branch");
    createOrOverwriteFile(repositoryPath / "file.txt", "Main");
    index.add("file.txt");
    auto secondCommit = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    createOrOverwriteFile(repositoryPath / "file.txt", "Second");
    index.add("file.txt");
    commits.createCommit("Third commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Second2");
    index.add("file.txt");
    auto fourthCommit = commits.createCommit("Fourth commit");

    ASSERT_THROW(rebase.rebase("main"), std::runtime_error);

    auto gitRebaseDir = repositoryPath / ".git" / "rebase-merge";
    EXPECT_EQ(getFileContent(gitRebaseDir / "onto"), secondCommit);
    EXPECT_EQ(getFileContent(gitRebaseDir / "head-name"), "refs/heads/second_branch");
    EXPECT_EQ(getFileContent(gitRebaseDir / "orig-head"), fourthCommit);
    EXPECT_EQ(getFileContent(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommit);
}
