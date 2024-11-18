#include "BaseRepositoryFixture.hpp"
#include "Branches.hpp"
#include "CherryPick.hpp"
#include "Commit.hpp"
#include "Commits.hpp"
#include "Index.hpp"
#include "_details/GitCommandExecutor/GitCommandExecutorUnix.hpp"

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
    auto commandExecutor = CppGit::GitCommandExecutorUnix{};

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");

    branches.createBranch("second-branch");

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    index.add("file.txt");

    auto envp = prepareCommitAuthorCommiterTestEnvp();
    CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommitHash }, envp);

    auto mainBranchCommitHash = commits.getHeadCommitHash();
    auto mainBranchCommitInfo = commits.getCommitInfo(mainBranchCommitHash);

    EXPECT_EQ(mainBranchCommitInfo.getMessage(), "Second commit");
    checkCommitAuthorEqualTest(mainBranchCommitInfo);
    checkCommitCommiterEqualTest(mainBranchCommitInfo);

    branches.changeCurrentBranch("second-branch");

    auto cherryPickedHash = cherryPick.cherryPickCommit(mainBranchCommitHash);

    auto cherryPickedInfo = commits.getCommitInfo(cherryPickedHash);
    auto headCommitHash = commits.getHeadCommitHash();

    ASSERT_EQ(cherryPickedHash, headCommitHash);
    ASSERT_NE(mainBranchCommitHash, cherryPickedHash);
    EXPECT_EQ(cherryPickedInfo.getMessage(), "Second commit");
    checkCommitAuthorEqualTest(cherryPickedInfo);
    checkCommitCommiterNotEqualTest(cherryPickedInfo);

    auto fileContent = getFileContent(repositoryPath / "file.txt");
    EXPECT_EQ(fileContent, "Hello, World! Modified");
}

TEST_F(CherryPickTests, cherryPickEmptyCommitFromCurrentBranch_keep)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto cherryPick = repository->CherryPick();
    auto commandExecutor = CppGit::GitCommandExecutorUnix{};

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");

    auto envp = prepareCommitAuthorCommiterTestEnvp();
    CppGit::_details::CreateCommit{ *repository }.createCommit("Initial commit", {}, envp);

    auto firstCommitHash = commits.getHeadCommitHash();
    auto firstCommitInfo = commits.getCommitInfo(firstCommitHash);

    EXPECT_EQ(firstCommitInfo.getMessage(), "Initial commit");
    checkCommitAuthorEqualTest(firstCommitInfo);
    checkCommitCommiterEqualTest(firstCommitInfo);

    createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");
    index.add("file2.txt");

    auto secondCommitHash = commits.createCommit("Second commit");


    auto cherryPickedHash = cherryPick.cherryPickCommit(firstCommitHash, CppGit::CherryPickEmptyCommitStrategy::KEEP);

    auto cherryPickedInfo = commits.getCommitInfo(cherryPickedHash);
    auto headCommitHash = commits.getHeadCommitHash();

    ASSERT_EQ(cherryPickedHash, headCommitHash);
    ASSERT_NE(firstCommitHash, cherryPickedHash);
    ASSERT_NE(secondCommitHash, cherryPickedHash);
    EXPECT_EQ(cherryPickedInfo.getMessage(), "Initial commit");
    checkCommitAuthorEqualTest(cherryPickedInfo);
    checkCommitCommiterNotEqualTest(cherryPickedInfo);

    auto fileContent = getFileContent(repositoryPath / "file.txt");
    EXPECT_EQ(fileContent, "Hello, World!");
}

TEST_F(CherryPickTests, cherryPickEmptyCommitFromCurrentBranch_drop)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto cherryPick = repository->CherryPick();
    auto commandExecutor = CppGit::GitCommandExecutorUnix{};

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");

    auto envp = prepareCommitAuthorCommiterTestEnvp();
    CppGit::_details::CreateCommit{ *repository }.createCommit("Initial commit", {}, envp);

    auto firstCommitHash = commits.getHeadCommitHash();
    auto firstCommitInfo = commits.getCommitInfo(firstCommitHash);

    EXPECT_EQ(firstCommitInfo.getMessage(), "Initial commit");
    checkCommitAuthorEqualTest(firstCommitInfo);
    checkCommitCommiterEqualTest(firstCommitInfo);

    createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");
    index.add("file2.txt");

    auto secondCommithash = commits.createCommit("Second commit");

    auto cherryPickedHash = cherryPick.cherryPickCommit(firstCommitHash, CppGit::CherryPickEmptyCommitStrategy::DROP);

    auto headCommitHash = commits.getHeadCommitHash();

    ASSERT_EQ(secondCommithash, headCommitHash);

    EXPECT_EQ(cherryPickedHash, std::string(40, '0'));

    auto fileContent = getFileContent(repositoryPath / "file.txt");
    EXPECT_EQ(fileContent, "Hello, World!");
}

TEST_F(CherryPickTests, cherryPickEmptyCommitFromCurrentBranch_stop)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto cherryPick = repository->CherryPick();
    auto commandExecutor = CppGit::GitCommandExecutorUnix{};

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");

    auto envp = prepareCommitAuthorCommiterTestEnvp();
    CppGit::_details::CreateCommit{ *repository }.createCommit("Initial commit", {}, envp);

    auto firstCommitHash = commits.getHeadCommitHash();
    auto firstCommitInfo = commits.getCommitInfo(firstCommitHash);

    EXPECT_EQ(firstCommitInfo.getMessage(), "Initial commit");
    checkCommitAuthorEqualTest(firstCommitInfo);
    checkCommitCommiterEqualTest(firstCommitInfo);

    createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");
    index.add("file2.txt");

    auto secondCommitHash = commits.createCommit("Second commit");

    EXPECT_THROW(cherryPick.cherryPickCommit(firstCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP), std::runtime_error);

    auto headCommitHash = commits.getHeadCommitHash();

    ASSERT_EQ(secondCommitHash, headCommitHash);
    ASSERT_TRUE(cherryPick.isCherryPickInProgress());
    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git" / "CHERRY_PICK_HEAD"));

    auto cherrypickHeadFileContent = getFileContent(repositoryPath / ".git" / "CHERRY_PICK_HEAD");
    EXPECT_EQ(cherrypickHeadFileContent, firstCommitHash);

    auto cherryPickedHash = cherryPick.commitEmptyCherryPickedCommit();

    auto cherryPickedInfo = commits.getCommitInfo(cherryPickedHash);
    headCommitHash = commits.getHeadCommitHash();

    ASSERT_EQ(cherryPickedHash, headCommitHash);
    ASSERT_NE(firstCommitHash, cherryPickedHash);
    ASSERT_NE(secondCommitHash, cherryPickedHash);
    checkCommitAuthorEqualTest(cherryPickedInfo);
    checkCommitCommiterNotEqualTest(cherryPickedInfo);

    auto fileContent = getFileContent(repositoryPath / "file.txt");
    EXPECT_EQ(fileContent, "Hello, World!");
}

TEST_F(CherryPickTests, cherryPickDiffAlreadyExistFromAnotherCommitBranch)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto cherryPick = repository->CherryPick();
    auto commandExecutor = CppGit::GitCommandExecutorUnix{};

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");

    auto initialCommitHash = commits.createCommit("Initial commit");

    branches.createBranch("second-branch");

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    index.add("file.txt");

    auto envp = prepareCommitAuthorCommiterTestEnvp();
    CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommitHash }, envp);

    auto secondCommitHash = commits.getHeadCommitHash();
    auto secondCommitInfo = commits.getCommitInfo(secondCommitHash);

    EXPECT_EQ(secondCommitInfo.getMessage(), "Second commit");
    checkCommitAuthorEqualTest(secondCommitInfo);
    checkCommitCommiterEqualTest(secondCommitInfo);

    branches.changeCurrentBranch("second-branch");

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    index.add("file.txt");

    auto thirdCommitHash = commits.createCommit("Third commit");
    auto fourthCommitHash = commits.createCommit("Fourth commit");

    EXPECT_THROW(cherryPick.cherryPickCommit(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP), std::runtime_error);

    auto headCommitHash = commits.getHeadCommitHash();

    ASSERT_EQ(fourthCommitHash, headCommitHash);
    ASSERT_TRUE(cherryPick.isCherryPickInProgress());
    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git" / "CHERRY_PICK_HEAD"));

    auto cherrypickHeadFileContent = getFileContent(repositoryPath / ".git" / "CHERRY_PICK_HEAD");
    EXPECT_EQ(cherrypickHeadFileContent, secondCommitHash);
}

TEST_F(CherryPickTests, cherryPick_conflict_diffAlreadyExistButThenChanged)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto cherryPick = repository->CherryPick();
    auto commandExecutor = CppGit::GitCommandExecutorUnix{};

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");

    auto initialCommitHash = commits.createCommit("Initial commit");

    branches.createBranch("second-branch");

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified.");
    index.add("file.txt");

    auto envp = prepareCommitAuthorCommiterTestEnvp();
    CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommitHash }, envp);

    auto secondCommitHash = commits.getHeadCommitHash();
    auto secondCommitInfo = commits.getCommitInfo(secondCommitHash);

    EXPECT_EQ(secondCommitInfo.getMessage(), "Second commit");
    checkCommitAuthorEqualTest(secondCommitInfo);
    checkCommitCommiterEqualTest(secondCommitInfo);

    branches.changeCurrentBranch("second-branch");

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified.");
    index.add("file.txt");

    auto thirdCommitHash = commits.createCommit("Third commit");

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    index.add("file.txt");

    auto fourthCommitHash = commits.createCommit("Fourth commit");

    EXPECT_THROW(cherryPick.cherryPickCommit(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP), std::runtime_error);

    auto headCommitHash = commits.getHeadCommitHash();

    ASSERT_EQ(fourthCommitHash, headCommitHash);
    ASSERT_TRUE(cherryPick.isCherryPickInProgress());
    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git" / "CHERRY_PICK_HEAD"));

    auto cherrypickHeadFileContent = getFileContent(repositoryPath / ".git" / "CHERRY_PICK_HEAD");
    EXPECT_EQ(cherrypickHeadFileContent, secondCommitHash);

    auto fileContent = getFileContent(repositoryPath / "file.txt");
    EXPECT_EQ(fileContent, "<<<<<<< HEAD\nHello, World! Modified 2.\n=======\nHello, World! Modified.\n>>>>>>> " + secondCommitHash + "\n");
}

TEST_F(CherryPickTests, cherryPick_conflict_resolve)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto cherryPick = repository->CherryPick();
    auto commandExecutor = CppGit::GitCommandExecutorUnix{};

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");

    auto initialCommitHash = commits.createCommit("Initial commit");

    branches.createBranch("second-branch");

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    index.add("file.txt");

    auto envp = prepareCommitAuthorCommiterTestEnvp();
    CppGit::_details::CreateCommit{ *repository }.createCommit("Second commit", { initialCommitHash }, envp);

    auto secondCommitHash = commits.getHeadCommitHash();
    auto secondCommitInfo = commits.getCommitInfo(secondCommitHash);

    EXPECT_EQ(secondCommitInfo.getMessage(), "Second commit");
    checkCommitAuthorEqualTest(secondCommitInfo);
    checkCommitCommiterEqualTest(secondCommitInfo);

    branches.changeCurrentBranch("second-branch");

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2");
    index.add("file.txt");

    auto thirdCommitHash = commits.createCommit("Third commit");

    EXPECT_THROW(cherryPick.cherryPickCommit(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP), std::runtime_error);

    auto headCommitHash = commits.getHeadCommitHash();

    ASSERT_EQ(thirdCommitHash, headCommitHash);
    ASSERT_TRUE(cherryPick.isCherryPickInProgress());
    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git" / "CHERRY_PICK_HEAD"));

    auto cherrypickHeadFileContent = getFileContent(repositoryPath / ".git" / "CHERRY_PICK_HEAD");
    EXPECT_EQ(cherrypickHeadFileContent, secondCommitHash);

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Conflict resolved");
    index.add("file.txt");

    auto cherryPickResolvedHash = cherryPick.cherryPickContinue();

    auto cherryPickedInfo = commits.getCommitInfo(cherryPickResolvedHash);
    headCommitHash = commits.getHeadCommitHash();

    ASSERT_EQ(cherryPickResolvedHash, headCommitHash);
    ASSERT_NE(secondCommitHash, cherryPickResolvedHash);
    ASSERT_NE(thirdCommitHash, cherryPickResolvedHash);
    EXPECT_EQ(cherryPickedInfo.getMessage(), "Second commit");
    checkCommitAuthorEqualTest(cherryPickedInfo);
    checkCommitCommiterNotEqualTest(cherryPickedInfo);

    auto fileContent = getFileContent(repositoryPath / "file.txt");
    EXPECT_EQ(fileContent, "Hello, World! Conflict resolved");
}
