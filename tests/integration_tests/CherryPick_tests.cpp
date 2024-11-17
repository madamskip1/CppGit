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
    auto output = commandExecutor.execute(env, repositoryPath.string(), "commit", "-m", "Second commit", "--no-gpg-sign");

    auto mainBranchCommitHash = commits.getHeadCommitHash();
    auto mainBranchCommitInfo = commits.getCommitInfo(mainBranchCommitHash);

    EXPECT_EQ(mainBranchCommitInfo.getMessage(), "Second commit");
    EXPECT_EQ(mainBranchCommitInfo.getAuthor().name, "TestAuthor");
    EXPECT_EQ(mainBranchCommitInfo.getAuthor().email, "test@email.com");
    EXPECT_EQ(mainBranchCommitInfo.getAuthorDate(), "1730738278 +0100");
    EXPECT_EQ(mainBranchCommitInfo.getCommitter().name, "TestAuthor");
    EXPECT_EQ(mainBranchCommitInfo.getCommitter().email, "test@email.com");
    EXPECT_EQ(mainBranchCommitInfo.getCommitterDate(), "1730738278 +0100");

    branches.changeCurrentBranch("second-branch");

    auto cherryPickedHash = cherryPick.cherryPickCommit(mainBranchCommitHash);

    auto cherryPickedInfo = commits.getCommitInfo(cherryPickedHash);
    auto headCommitHash = commits.getHeadCommitHash();

    ASSERT_EQ(cherryPickedHash, headCommitHash);
    ASSERT_NE(mainBranchCommitHash, cherryPickedHash);
    EXPECT_EQ(cherryPickedInfo.getMessage(), "Second commit");
    EXPECT_EQ(cherryPickedInfo.getAuthor().name, "TestAuthor");
    EXPECT_EQ(cherryPickedInfo.getAuthor().email, "test@email.com");
    EXPECT_EQ(cherryPickedInfo.getAuthorDate(), "1730738278 +0100");
    EXPECT_NE(cherryPickedInfo.getCommitter().name, "TestAuthor");
    EXPECT_NE(cherryPickedInfo.getCommitter().email, "test@email.com");
    EXPECT_NE(cherryPickedInfo.getCommitterDate(), "1730738278 +0100");

    std::ifstream fileRead(repositoryPath / "file.txt");
    std::ostringstream fileContentStream;
    fileContentStream << fileRead.rdbuf();
    EXPECT_EQ(fileContentStream.str(), "Hello, World! Modified");
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

    auto env = std::vector<std::string>{ "GIT_AUTHOR_NAME=TestAuthor", "GIT_AUTHOR_EMAIL=test@email.com", "GIT_AUTHOR_DATE=1730738278 +0100", "GIT_COMMITTER_NAME=TestAuthor", "GIT_COMMITTER_EMAIL=test@email.com", "GIT_COMMITTER_DATE=1730738278 +0100" };
    auto output = commandExecutor.execute(env, repositoryPath.string(), "commit", "-m", "Initial commit", "--no-gpg-sign");

    auto firstCommitHash = commits.getHeadCommitHash();
    auto firstCommitInfo = commits.getCommitInfo(firstCommitHash);

    EXPECT_EQ(firstCommitInfo.getMessage(), "Initial commit");
    EXPECT_EQ(firstCommitInfo.getAuthor().name, "TestAuthor");
    EXPECT_EQ(firstCommitInfo.getAuthor().email, "test@email.com");
    EXPECT_EQ(firstCommitInfo.getAuthorDate(), "1730738278 +0100");
    EXPECT_EQ(firstCommitInfo.getCommitter().name, "TestAuthor");
    EXPECT_EQ(firstCommitInfo.getCommitter().email, "test@email.com");
    EXPECT_EQ(firstCommitInfo.getCommitterDate(), "1730738278 +0100");

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
    EXPECT_EQ(cherryPickedInfo.getAuthor().name, "TestAuthor");
    EXPECT_EQ(cherryPickedInfo.getAuthor().email, "test@email.com");
    EXPECT_EQ(cherryPickedInfo.getAuthorDate(), "1730738278 +0100");
    EXPECT_NE(cherryPickedInfo.getCommitter().name, "TestAuthor");
    EXPECT_NE(cherryPickedInfo.getCommitter().email, "test@email.com");
    EXPECT_NE(cherryPickedInfo.getCommitterDate(), "1730738278 +0100");

    auto fileRead = std::ifstream{ repositoryPath / "file.txt" };
    std::ostringstream content;
    content << fileRead.rdbuf();
    EXPECT_EQ(content.str(), "Hello, World!");
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

    auto env = std::vector<std::string>{ "GIT_AUTHOR_NAME=TestAuthor", "GIT_AUTHOR_EMAIL=test@email.com", "GIT_AUTHOR_DATE=1730738278 +0100", "GIT_COMMITTER_NAME=TestAuthor", "GIT_COMMITTER_EMAIL=test@email.com", "GIT_COMMITTER_DATE=1730738278 +0100" };
    auto output = commandExecutor.execute(env, repositoryPath.string(), "commit", "-m", "Initial commit", "--no-gpg-sign");

    auto firstCommitHash = commits.getHeadCommitHash();
    auto firstCommitInfo = commits.getCommitInfo(firstCommitHash);

    EXPECT_EQ(firstCommitInfo.getMessage(), "Initial commit");
    EXPECT_EQ(firstCommitInfo.getAuthor().name, "TestAuthor");
    EXPECT_EQ(firstCommitInfo.getAuthor().email, "test@email.com");
    EXPECT_EQ(firstCommitInfo.getAuthorDate(), "1730738278 +0100");
    EXPECT_EQ(firstCommitInfo.getCommitter().name, "TestAuthor");
    EXPECT_EQ(firstCommitInfo.getCommitter().email, "test@email.com");
    EXPECT_EQ(firstCommitInfo.getCommitterDate(), "1730738278 +0100");

    std::ofstream file2(repositoryPath / "file2.txt");
    file2 << "Hello, World!";
    file2.close();

    index.add("file2.txt");

    auto secondCommithash = commits.createCommit("Second commit");

    auto cherryPickedHash = cherryPick.cherryPickCommit(firstCommitHash, CppGit::CherryPickEmptyCommitStrategy::DROP);

    auto headCommitHash = commits.getHeadCommitHash();

    ASSERT_EQ(secondCommithash, headCommitHash);

    EXPECT_EQ(cherryPickedHash, std::string(40, '0'));

    auto fileRead = std::ifstream{ repositoryPath / "file.txt" };
    std::ostringstream content;
    content << fileRead.rdbuf();
    EXPECT_EQ(content.str(), "Hello, World!");
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

    auto env = std::vector<std::string>{ "GIT_AUTHOR_NAME=TestAuthor", "GIT_AUTHOR_EMAIL=test@email.com", "GIT_AUTHOR_DATE=1730738278 +0100", "GIT_COMMITTER_NAME=TestAuthor", "GIT_COMMITTER_EMAIL=test@email.com", "GIT_COMMITTER_DATE=1730738278 +0100" };
    auto output = commandExecutor.execute(env, repositoryPath.string(), "commit", "-m", "Initial commit", "--no-gpg-sign");

    auto firstCommitHash = commits.getHeadCommitHash();
    auto firstCommitInfo = commits.getCommitInfo(firstCommitHash);

    EXPECT_EQ(firstCommitInfo.getMessage(), "Initial commit");
    EXPECT_EQ(firstCommitInfo.getAuthor().name, "TestAuthor");
    EXPECT_EQ(firstCommitInfo.getAuthor().email, "test@email.com");
    EXPECT_EQ(firstCommitInfo.getAuthorDate(), "1730738278 +0100");
    EXPECT_EQ(firstCommitInfo.getCommitter().name, "TestAuthor");
    EXPECT_EQ(firstCommitInfo.getCommitter().email, "test@email.com");
    EXPECT_EQ(firstCommitInfo.getCommitterDate(), "1730738278 +0100");

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

    auto cherryPickHeadFile = std::ifstream{ repositoryPath / ".git" / "CHERRY_PICK_HEAD" };
    std::ostringstream cherryPickHeadFileContent;
    cherryPickHeadFileContent << cherryPickHeadFile.rdbuf();

    EXPECT_EQ(cherryPickHeadFileContent.str(), firstCommitHash);

    auto cherryPickedHash = cherryPick.commitEmptyCherryPickedCommit();

    auto cherryPickedInfo = commits.getCommitInfo(cherryPickedHash);
    headCommitHash = commits.getHeadCommitHash();

    ASSERT_EQ(cherryPickedHash, headCommitHash);
    ASSERT_NE(firstCommitHash, cherryPickedHash);
    ASSERT_NE(secondCommitHash, cherryPickedHash);
    EXPECT_EQ(cherryPickedInfo.getMessage(), "Initial commit");
    EXPECT_EQ(cherryPickedInfo.getAuthor().name, "TestAuthor");
    EXPECT_EQ(cherryPickedInfo.getAuthor().email, "test@email.com");
    EXPECT_EQ(cherryPickedInfo.getAuthorDate(), "1730738278 +0100");
    EXPECT_NE(cherryPickedInfo.getCommitter().name, "TestAuthor");
    EXPECT_NE(cherryPickedInfo.getCommitter().email, "test@email.com");
    EXPECT_NE(cherryPickedInfo.getCommitterDate(), "1730738278 +0100");

    auto fileRead = std::ifstream{ repositoryPath / "file.txt" };
    std::ostringstream content;
    content << fileRead.rdbuf();
    EXPECT_EQ(content.str(), "Hello, World!");
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

    auto env = std::vector<std::string>{ "GIT_AUTHOR_NAME=TestAuthor", "GIT_AUTHOR_EMAIL=test@email.com", "GIT_AUTHOR_DATE=1730738278 +0100", "GIT_COMMITTER_NAME=TestAuthor", "GIT_COMMITTER_EMAIL=test@email.com", "GIT_COMMITTER_DATE=1730738278 +0100" };
    auto output = commandExecutor.execute(env, repositoryPath.string(), "commit", "-m", "Second commit", "--no-gpg-sign");

    auto secondCommitHash = commits.getHeadCommitHash();
    auto secondCommitInfo = commits.getCommitInfo(secondCommitHash);

    EXPECT_EQ(secondCommitInfo.getMessage(), "Second commit");
    EXPECT_EQ(secondCommitInfo.getAuthor().name, "TestAuthor");
    EXPECT_EQ(secondCommitInfo.getAuthor().email, "test@email.com");
    EXPECT_EQ(secondCommitInfo.getAuthorDate(), "1730738278 +0100");
    EXPECT_EQ(secondCommitInfo.getCommitter().name, "TestAuthor");
    EXPECT_EQ(secondCommitInfo.getCommitter().email, "test@email.com");
    EXPECT_EQ(secondCommitInfo.getCommitterDate(), "1730738278 +0100");

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

    auto cherryPickHeadFile = std::ifstream{ repositoryPath / ".git" / "CHERRY_PICK_HEAD" };
    std::ostringstream cherryPickHeadFileContent;
    cherryPickHeadFileContent << cherryPickHeadFile.rdbuf();

    EXPECT_EQ(cherryPickHeadFileContent.str(), secondCommitHash);
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

    auto env = std::vector<std::string>{ "GIT_AUTHOR_NAME=TestAuthor", "GIT_AUTHOR_EMAIL=test@email.com", "GIT_AUTHOR_DATE=1730738278 +0100", "GIT_COMMITTER_NAME=TestAuthor", "GIT_COMMITTER_EMAIL=test@email.com", "GIT_COMMITTER_DATE=1730738278 +0100" };
    auto output = commandExecutor.execute(env, repositoryPath.string(), "commit", "-m", "Second commit", "--no-gpg-sign");

    auto secondCommitHash = commits.getHeadCommitHash();
    auto secondCommitInfo = commits.getCommitInfo(secondCommitHash);

    EXPECT_EQ(secondCommitInfo.getMessage(), "Second commit");
    EXPECT_EQ(secondCommitInfo.getAuthor().name, "TestAuthor");
    EXPECT_EQ(secondCommitInfo.getAuthor().email, "test@email.com");
    EXPECT_EQ(secondCommitInfo.getAuthorDate(), "1730738278 +0100");
    EXPECT_EQ(secondCommitInfo.getCommitter().name, "TestAuthor");
    EXPECT_EQ(secondCommitInfo.getCommitter().email, "test@email.com");
    EXPECT_EQ(secondCommitInfo.getCommitterDate(), "1730738278 +0100");

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

    auto cherryPickHeadFile = std::ifstream{ repositoryPath / ".git" / "CHERRY_PICK_HEAD" };
    std::ostringstream cherryPickHeadFileContent;
    cherryPickHeadFileContent << cherryPickHeadFile.rdbuf();

    EXPECT_EQ(cherryPickHeadFileContent.str(), secondCommitHash);

    std::ifstream fileRead(repositoryPath / "file.txt");
    std::ostringstream content;
    content << fileRead.rdbuf();
    EXPECT_EQ(content.str(), "<<<<<<< HEAD\nHello, World! Modified 2\n=======\nHello, World! Modified\n>>>>>>> " + secondCommitHash + "\n");
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

    auto env = std::vector<std::string>{ "GIT_AUTHOR_NAME=TestAuthor", "GIT_AUTHOR_EMAIL=test@email.com", "GIT_AUTHOR_DATE=1730738278 +0100", "GIT_COMMITTER_NAME=TestAuthor", "GIT_COMMITTER_EMAIL=test@email.com", "GIT_COMMITTER_DATE=1730738278 +0100" };
    auto output = commandExecutor.execute(env, repositoryPath.string(), "commit", "-m", "Second commit", "--no-gpg-sign");

    auto secondCommitHash = commits.getHeadCommitHash();
    auto secondCommitInfo = commits.getCommitInfo(secondCommitHash);

    EXPECT_EQ(secondCommitInfo.getMessage(), "Second commit");
    EXPECT_EQ(secondCommitInfo.getAuthor().name, "TestAuthor");
    EXPECT_EQ(secondCommitInfo.getAuthor().email, "test@email.com");
    EXPECT_EQ(secondCommitInfo.getAuthorDate(), "1730738278 +0100");
    EXPECT_EQ(secondCommitInfo.getCommitter().name, "TestAuthor");
    EXPECT_EQ(secondCommitInfo.getCommitter().email, "test@email.com");
    EXPECT_EQ(secondCommitInfo.getCommitterDate(), "1730738278 +0100");

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

    auto cherryPickHeadFile = std::ifstream{ repositoryPath / ".git" / "CHERRY_PICK_HEAD" };
    std::ostringstream cherryPickHeadFileContent;
    cherryPickHeadFileContent << cherryPickHeadFile.rdbuf();

    EXPECT_EQ(cherryPickHeadFileContent.str(), secondCommitHash);

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
    EXPECT_EQ(cherryPickedInfo.getAuthor().name, "TestAuthor");
    EXPECT_EQ(cherryPickedInfo.getAuthor().email, "test@email.com");
    EXPECT_EQ(cherryPickedInfo.getAuthorDate(), "1730738278 +0100");
    EXPECT_NE(cherryPickedInfo.getCommitter().name, "TestAuthor");
    EXPECT_NE(cherryPickedInfo.getCommitter().email, "test@email.com");
    EXPECT_NE(cherryPickedInfo.getCommitterDate(), "1730738278 +0100");

    auto fileRead = std::ifstream{ repositoryPath / "file.txt" };
    std::ostringstream content;
    content << fileRead.rdbuf();
    EXPECT_EQ(content.str(), "Hello, World! Conflict resolved");
}
