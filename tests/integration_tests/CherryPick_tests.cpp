#include "BaseRepositoryFixture.hpp"
#include "Branches.hpp"
#include "CherryPick.hpp"
#include "Commit.hpp"
#include "Commits.hpp"
#include "Index.hpp"
#include "_details/GitCommandExecutor/GitCommandExecutorUnix.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

class CherryPickTests : public BaseRepositoryFixture
{
protected:
    static constexpr auto* const AUTHOR_NAME = "TestAuthor";
    static constexpr auto* const AUTHOR_EMAIL = "test@email.com";
    static constexpr auto* const AUTHOR_DATE = "1730738278 +0100";

    static auto prepareCommitAuthorCommiterTestEnvp() -> std::vector<std::string>
    {
        auto envp = std::vector<std::string>{ std::string{ "GIT_AUTHOR_NAME=" } + AUTHOR_NAME, std::string{ "GIT_AUTHOR_EMAIL=" } + AUTHOR_EMAIL, std::string{ "GIT_AUTHOR_DATE=" } + AUTHOR_DATE, std::string{ "GIT_COMMITTER_NAME=" } + AUTHOR_NAME, std::string{ "GIT_COMMITTER_EMAIL=" } + AUTHOR_EMAIL, std::string{ "GIT_COMMITTER_DATE=" } + AUTHOR_DATE };

        return envp;
    }

    static auto checkCommitAuthorEqualTest(const CppGit::Commit& commit) -> void
    {
        EXPECT_EQ(commit.getAuthor().name, AUTHOR_NAME);
        EXPECT_EQ(commit.getAuthor().email, AUTHOR_EMAIL);
        EXPECT_EQ(commit.getAuthorDate(), AUTHOR_DATE);
    }

    static auto checkCommitCommiterEqualTest(const CppGit::Commit& commit) -> void
    {
        EXPECT_EQ(commit.getCommitter().name, AUTHOR_NAME);
        EXPECT_EQ(commit.getCommitter().email, AUTHOR_EMAIL);
        EXPECT_EQ(commit.getCommitterDate(), AUTHOR_DATE);
    }

    static auto checkCommitCommiterNotEqualTest(const CppGit::Commit& commit) -> void
    {
        EXPECT_NE(commit.getCommitter().name, AUTHOR_NAME);
        EXPECT_NE(commit.getCommitter().email, AUTHOR_EMAIL);
        EXPECT_NE(commit.getCommitterDate(), AUTHOR_DATE);
    }
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

    auto envp = prepareCommitAuthorCommiterTestEnvp();

    auto output = commandExecutor.execute(envp, repositoryPath.string(), "commit", "-m", "Second commit", "--no-gpg-sign");

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

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();

    index.add("file.txt");

    auto envp = prepareCommitAuthorCommiterTestEnvp();

    auto output = commandExecutor.execute(envp, repositoryPath.string(), "commit", "-m", "Initial commit", "--no-gpg-sign");

    auto firstCommitHash = commits.getHeadCommitHash();
    auto firstCommitInfo = commits.getCommitInfo(firstCommitHash);

    EXPECT_EQ(firstCommitInfo.getMessage(), "Initial commit");
    checkCommitAuthorEqualTest(firstCommitInfo);
    checkCommitCommiterEqualTest(firstCommitInfo);

    std::ofstream file2(repositoryPath / "file2.txt");
    file2 << "Hello, World!";
    file2.close();

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

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();

    index.add("file.txt");

    auto envp = prepareCommitAuthorCommiterTestEnvp();

    auto output = commandExecutor.execute(envp, repositoryPath.string(), "commit", "-m", "Initial commit", "--no-gpg-sign");

    auto firstCommitHash = commits.getHeadCommitHash();
    auto firstCommitInfo = commits.getCommitInfo(firstCommitHash);

    EXPECT_EQ(firstCommitInfo.getMessage(), "Initial commit");
    checkCommitAuthorEqualTest(firstCommitInfo);
    checkCommitCommiterEqualTest(firstCommitInfo);

    std::ofstream file2(repositoryPath / "file2.txt");
    file2 << "Hello, World!";
    file2.close();

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

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();

    index.add("file.txt");

    auto envp = prepareCommitAuthorCommiterTestEnvp();

    auto output = commandExecutor.execute(envp, repositoryPath.string(), "commit", "-m", "Initial commit", "--no-gpg-sign");

    auto firstCommitHash = commits.getHeadCommitHash();
    auto firstCommitInfo = commits.getCommitInfo(firstCommitHash);

    EXPECT_EQ(firstCommitInfo.getMessage(), "Initial commit");
    checkCommitAuthorEqualTest(firstCommitInfo);
    checkCommitCommiterEqualTest(firstCommitInfo);

    std::ofstream file2(repositoryPath / "file2.txt");
    file2 << "Hello, World!";
    file2.close();

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

    auto envp = prepareCommitAuthorCommiterTestEnvp();

    auto output = commandExecutor.execute(envp, repositoryPath.string(), "commit", "-m", "Second commit", "--no-gpg-sign");

    auto secondCommitHash = commits.getHeadCommitHash();
    auto secondCommitInfo = commits.getCommitInfo(secondCommitHash);

    EXPECT_EQ(secondCommitInfo.getMessage(), "Second commit");
    checkCommitAuthorEqualTest(secondCommitInfo);
    checkCommitCommiterEqualTest(secondCommitInfo);

    branches.changeCurrentBranch("second-branch");

    file.open(repositoryPath / "file.txt");
    file << "Hello, World! Modified";
    file.close();
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

    auto envp = prepareCommitAuthorCommiterTestEnvp();

    auto output = commandExecutor.execute(envp, repositoryPath.string(), "commit", "-m", "Second commit", "--no-gpg-sign");

    auto secondCommitHash = commits.getHeadCommitHash();
    auto secondCommitInfo = commits.getCommitInfo(secondCommitHash);

    EXPECT_EQ(secondCommitInfo.getMessage(), "Second commit");
    checkCommitAuthorEqualTest(secondCommitInfo);
    checkCommitCommiterEqualTest(secondCommitInfo);

    branches.changeCurrentBranch("second-branch");

    file.open(repositoryPath / "file.txt");
    file << "Hello, World! Modified";
    file.close();
    index.add("file.txt");

    auto thirdCommitHash = commits.createCommit("Third commit");

    file.open(repositoryPath / "file.txt");
    file << "Hello, World! Modified 2";
    file.close();
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
    EXPECT_EQ(fileContent, "<<<<<<< HEAD\nHello, World! Modified 2\n=======\nHello, World! Modified\n>>>>>>> " + secondCommitHash + "\n");
}

TEST_F(CherryPickTests, cherryPick_conflict_resolve)
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

    auto envp = prepareCommitAuthorCommiterTestEnvp();

    auto output = commandExecutor.execute(envp, repositoryPath.string(), "commit", "-m", "Second commit", "--no-gpg-sign");

    auto secondCommitHash = commits.getHeadCommitHash();
    auto secondCommitInfo = commits.getCommitInfo(secondCommitHash);

    EXPECT_EQ(secondCommitInfo.getMessage(), "Second commit");
    checkCommitAuthorEqualTest(secondCommitInfo);
    checkCommitCommiterEqualTest(secondCommitInfo);

    branches.changeCurrentBranch("second-branch");

    file.open(repositoryPath / "file.txt");
    file << "Hello, World! Modified 2";
    file.close();
    index.add("file.txt");

    auto thirdCommitHash = commits.createCommit("Third commit");

    EXPECT_THROW(cherryPick.cherryPickCommit(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP), std::runtime_error);

    auto headCommitHash = commits.getHeadCommitHash();

    ASSERT_EQ(thirdCommitHash, headCommitHash);
    ASSERT_TRUE(cherryPick.isCherryPickInProgress());
    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git" / "CHERRY_PICK_HEAD"));

    auto cherrypickHeadFileContent = getFileContent(repositoryPath / ".git" / "CHERRY_PICK_HEAD");
    EXPECT_EQ(cherrypickHeadFileContent, secondCommitHash);

    file.open(repositoryPath / "file.txt");
    file << "Hello, World! Conflict resolved";
    file.close();
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
