#include "BaseRepositoryFixture.hpp"
#include "Branches.hpp"
#include "CherryPick.hpp"
#include "Commit.hpp"
#include "Commits.hpp"
#include "Index.hpp"

#include <filesystem>
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


    auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second-branch");

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto secondCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommitHash }, envp);

    branches.changeCurrentBranch("second-branch");

    auto cherryPickedHash = cherryPick.cherryPickCommit(secondCommitHash);


    auto cherryPickedInfo = commits.getCommitInfo(cherryPickedHash);
    ASSERT_EQ(cherryPickedHash, commits.getHeadCommitHash());
    ASSERT_NE(cherryPickedHash, secondCommitHash);
    EXPECT_EQ(cherryPickedInfo.getMessage(), "Second commit");
    checkCommitAuthorEqualTest(cherryPickedInfo);
    checkCommitCommiterNotEqualTest(cherryPickedInfo);
    EXPECT_EQ(getFileContent(repositoryPath / "file.txt"), "Hello, World!");
}

TEST_F(CherryPickTests, cherryPickEmptyCommitFromCurrentBranch_keep)
{
    auto commits = repository->Commits();
    auto cherryPick = repository->CherryPick();


    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto initialCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Initial commit", {}, envp);
    auto secondCommitHash = commits.createCommit("Second commit");


    auto cherryPickedHash = cherryPick.cherryPickCommit(initialCommitHash, CppGit::CherryPickEmptyCommitStrategy::KEEP);


    auto cherryPickedInfo = commits.getCommitInfo(cherryPickedHash);
    ASSERT_EQ(cherryPickedHash, commits.getHeadCommitHash());
    ASSERT_NE(cherryPickedHash, secondCommitHash);
    EXPECT_EQ(cherryPickedInfo.getMessage(), "Initial commit");
    checkCommitAuthorEqualTest(cherryPickedInfo);
    checkCommitCommiterNotEqualTest(cherryPickedInfo);
}

TEST_F(CherryPickTests, cherryPickEmptyCommitFromCurrentBranch_drop)
{
    auto commits = repository->Commits();
    auto cherryPick = repository->CherryPick();


    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto initialCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Initial commit", {}, envp);
    auto secondCommitHash = commits.createCommit("Second commit");

    auto cherryPickedHash = cherryPick.cherryPickCommit(initialCommitHash, CppGit::CherryPickEmptyCommitStrategy::DROP);


    ASSERT_EQ(secondCommitHash, commits.getHeadCommitHash());
    EXPECT_EQ(cherryPickedHash, std::string(40, '0'));
}

TEST_F(CherryPickTests, cherryPickEmptyCommitFromCurrentBranch_stop)
{
    auto commits = repository->Commits();
    auto cherryPick = repository->CherryPick();


    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto initialCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Initial commit", {}, envp);
    auto secondCommitHash = commits.createCommit("Second commit");


    EXPECT_THROW(cherryPick.cherryPickCommit(initialCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP), std::runtime_error);


    ASSERT_EQ(secondCommitHash, commits.getHeadCommitHash());
    ASSERT_TRUE(cherryPick.isCherryPickInProgress());
    EXPECT_EQ(getFileContent(repositoryPath / ".git" / "CHERRY_PICK_HEAD"), initialCommitHash);
}

TEST_F(CherryPickTests, cherryPickEmptyCommitFromCurrentBranch_commitAfterStop)
{
    auto commits = repository->Commits();
    auto cherryPick = repository->CherryPick();


    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto initialCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Initial commit", {}, envp);
    auto secondCommitHash = commits.createCommit("Second commit");

    EXPECT_THROW(cherryPick.cherryPickCommit(initialCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP), std::runtime_error);

    auto cherryPickedHash = cherryPick.commitEmptyCherryPickedCommit();


    auto cherryPickedInfo = commits.getCommitInfo(cherryPickedHash);
    ASSERT_EQ(cherryPickedHash, commits.getHeadCommitHash());
    ASSERT_NE(cherryPickedHash, initialCommitHash);
    ASSERT_NE(cherryPickedHash, secondCommitHash);
    checkCommitAuthorEqualTest(cherryPickedInfo);
}


TEST_F(CherryPickTests, cherryPickDiffAlreadyExistFromAnotherCommitBranch)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto cherryPick = repository->CherryPick();


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second-branch");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    index.add("file.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto secondCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommitHash }, envp);

    branches.changeCurrentBranch("second-branch");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    index.add("file.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    EXPECT_THROW(cherryPick.cherryPickCommit(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP), std::runtime_error);


    ASSERT_EQ(thirdCommitHash, commits.getHeadCommitHash());
    ASSERT_TRUE(cherryPick.isCherryPickInProgress());
    EXPECT_EQ(getFileContent(repositoryPath / ".git" / "CHERRY_PICK_HEAD"), secondCommitHash);
}

TEST_F(CherryPickTests, cherryPick_conflict_diffAlreadyExistButThenChanged)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto cherryPick = repository->CherryPick();


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second-branch");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified.");
    index.add("file.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto secondCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommitHash }, envp);

    branches.changeCurrentBranch("second-branch");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified.");
    index.add("file.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    index.add("file.txt");
    auto fourthCommitHash = commits.createCommit("Fourth commit");

    EXPECT_THROW(cherryPick.cherryPickCommit(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP), std::runtime_error);


    ASSERT_EQ(fourthCommitHash, commits.getHeadCommitHash());
    ASSERT_TRUE(cherryPick.isCherryPickInProgress());
    EXPECT_EQ(getFileContent(repositoryPath / ".git" / "CHERRY_PICK_HEAD"), secondCommitHash);
    EXPECT_EQ(getFileContent(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2.\n=======\nHello, World! Modified.\n>>>>>>> " + secondCommitHash + "\n");
}

TEST_F(CherryPickTests, cherryPick_conflict_resolve)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto cherryPick = repository->CherryPick();


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second-branch");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    index.add("file.txt");
    auto envp = prepareCommitAuthorCommiterTestEnvp();
    auto secondCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommitHash }, envp);

    branches.changeCurrentBranch("second-branch");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2");
    index.add("file.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    EXPECT_THROW(cherryPick.cherryPickCommit(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP), std::runtime_error);
    ASSERT_TRUE(cherryPick.isCherryPickInProgress());

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Conflict resolved");
    index.add("file.txt");

    auto cherryPickResolvedHash = cherryPick.cherryPickContinue();


    auto cherryPickedInfo = commits.getCommitInfo(cherryPickResolvedHash);
    ASSERT_EQ(cherryPickResolvedHash, commits.getHeadCommitHash());
    ASSERT_NE(secondCommitHash, cherryPickResolvedHash);
    ASSERT_NE(thirdCommitHash, cherryPickResolvedHash);
    EXPECT_EQ(cherryPickedInfo.getMessage(), "Second commit");
    checkCommitAuthorEqualTest(cherryPickedInfo);
    checkCommitCommiterNotEqualTest(cherryPickedInfo);
    EXPECT_EQ(getFileContent(repositoryPath / "file.txt"), "Hello, World! Conflict resolved");
}
