#include "BaseRepositoryFixture.hpp"
#include "Branches.hpp"
#include "CherryPick.hpp"
#include "Commit.hpp"
#include "Commits.hpp"
#include "Index.hpp"
#include "_details/FileUtility.hpp"

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

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);

    branches.changeCurrentBranch("second-branch");

    auto cherryPickedHash = cherryPick.cherryPickCommit(secondCommitHash);

    ASSERT_TRUE(cherryPickedHash.has_value());
    auto cherryPickedInfo = commits.getCommitInfo(cherryPickedHash.value());
    EXPECT_EQ(cherryPickedHash, commits.getHeadCommitHash());
    EXPECT_NE(cherryPickedHash, secondCommitHash);
    EXPECT_EQ(cherryPickedInfo.getMessage(), "Second commit");
    checkCommitAuthorEqualTest(cherryPickedInfo);
    checkCommitCommiterNotEqualTest(cherryPickedInfo);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), initialCommitHash);
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "CHERRY_PICK_HEAD"));
}

TEST_F(CherryPickTests, cherryPickEmptyCommitFromCurrentBranch_keep)
{
    auto commits = repository->Commits();
    auto cherryPick = repository->CherryPick();


    auto initialCommitHash = createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    auto secondCommitHash = commits.createCommit("Second commit");


    auto cherryPickedHash = cherryPick.cherryPickCommit(initialCommitHash, CppGit::CherryPickEmptyCommitStrategy::KEEP);


    ASSERT_TRUE(cherryPickedHash.has_value());
    auto cherryPickedInfo = commits.getCommitInfo(cherryPickedHash.value());
    EXPECT_EQ(cherryPickedHash, commits.getHeadCommitHash());
    EXPECT_NE(cherryPickedHash, secondCommitHash);
    EXPECT_EQ(cherryPickedInfo.getMessage(), "Initial commit");
    checkCommitAuthorEqualTest(cherryPickedInfo);
    checkCommitCommiterNotEqualTest(cherryPickedInfo);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "CHERRY_PICK_HEAD"));
}

TEST_F(CherryPickTests, cherryPickEmptyCommitFromCurrentBranch_drop)
{
    auto commits = repository->Commits();
    auto cherryPick = repository->CherryPick();


    auto initialCommitHash = createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    auto secondCommitHash = commits.createCommit("Second commit");

    auto cherryPickedHash = cherryPick.cherryPickCommit(initialCommitHash, CppGit::CherryPickEmptyCommitStrategy::DROP);


    ASSERT_TRUE(cherryPickedHash.has_value());
    EXPECT_EQ(cherryPickedHash, std::string(40, '0'));
    EXPECT_EQ(secondCommitHash, commits.getHeadCommitHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "CHERRY_PICK_HEAD"));
}

TEST_F(CherryPickTests, cherryPickEmptyCommitFromCurrentBranch_stop)
{
    auto commits = repository->Commits();
    auto cherryPick = repository->CherryPick();


    auto initialCommitHash = createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    auto secondCommitHash = commits.createCommit("Second commit");

    auto cherryPickedHash = cherryPick.cherryPickCommit(initialCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP);


    ASSERT_FALSE(cherryPickedHash.has_value());
    EXPECT_EQ(cherryPickedHash.error(), CppGit::Error::CHERRY_PICK_EMPTY_COMMIT);
    EXPECT_EQ(secondCommitHash, commits.getHeadCommitHash());
    EXPECT_TRUE(cherryPick.isCherryPickInProgress());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "CHERRY_PICK_HEAD"), initialCommitHash);
}

TEST_F(CherryPickTests, cherryPickEmptyCommitFromCurrentBranch_commitAfterStop)
{
    auto commits = repository->Commits();
    auto cherryPick = repository->CherryPick();


    auto initialCommitHash = createCommitWithTestAuthorCommiterWithoutParent("Initial commit");
    auto secondCommitHash = commits.createCommit("Second commit");

    cherryPick.cherryPickCommit(initialCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP);
    auto cherryPickedHash = cherryPick.commitEmptyCherryPickedCommit();


    ASSERT_TRUE(cherryPickedHash.has_value());
    auto cherryPickedInfo = commits.getCommitInfo(cherryPickedHash.value());
    EXPECT_EQ(cherryPickedHash, commits.getHeadCommitHash());
    EXPECT_NE(cherryPickedHash, initialCommitHash);
    EXPECT_NE(cherryPickedHash, secondCommitHash);
    checkCommitAuthorEqualTest(cherryPickedInfo);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), secondCommitHash);
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "CHERRY_PICK_HEAD"));
}

TEST_F(CherryPickTests, tryCommitEmptyCherryPickedCommitWhenNoCherryPickInProgress)
{
    auto cherryPick = repository->CherryPick();
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    auto cherryPickedHash = cherryPick.commitEmptyCherryPickedCommit();

    ASSERT_FALSE(cherryPickedHash.has_value());
    EXPECT_EQ(cherryPickedHash.error(), CppGit::Error::NO_CHERRY_PICK_IN_PROGRESS);
}

TEST_F(CherryPickTests, cherryPickDiffAlreadyExistFromAnotherCommitBranch)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto cherryPick = repository->CherryPick();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    index.add("file.txt");
    auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);

    branches.changeCurrentBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    index.add("file.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    auto cherryPickedHash = cherryPick.cherryPickCommit(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP);


    ASSERT_FALSE(cherryPickedHash.has_value());
    EXPECT_EQ(cherryPickedHash.error(), CppGit::Error::CHERRY_PICK_EMPTY_COMMIT);
    EXPECT_EQ(thirdCommitHash, commits.getHeadCommitHash());
    EXPECT_TRUE(cherryPick.isCherryPickInProgress());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "CHERRY_PICK_HEAD"), secondCommitHash);
}

TEST_F(CherryPickTests, cherryPick_conflict_diffAlreadyExistButThenChanged)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto cherryPick = repository->CherryPick();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified.");
    index.add("file.txt");
    auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);

    branches.changeCurrentBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified.");
    index.add("file.txt");
    commits.createCommit("Third commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    index.add("file.txt");
    auto fourthCommitHash = commits.createCommit("Fourth commit");

    auto cherryPickedHash = cherryPick.cherryPickCommit(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP);


    ASSERT_FALSE(cherryPickedHash.has_value());
    EXPECT_EQ(cherryPickedHash.error(), CppGit::Error::CHERRY_PICK_CONFLICT);
    EXPECT_EQ(fourthCommitHash, commits.getHeadCommitHash());
    EXPECT_TRUE(cherryPick.isCherryPickInProgress());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "CHERRY_PICK_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2.\n=======\nHello, World! Modified.\n>>>>>>> " + secondCommitHash + "\n");
}

TEST_F(CherryPickTests, cherryPick_conflict_resolve)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto cherryPick = repository->CherryPick();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    index.add("file.txt");
    auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);

    branches.changeCurrentBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2");
    index.add("file.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    cherryPick.cherryPickCommit(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP);
    ASSERT_TRUE(cherryPick.isCherryPickInProgress());

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Conflict resolved");
    index.add("file.txt");

    auto cherryPickResolvedHash = cherryPick.cherryPickContinue();


    ASSERT_TRUE(cherryPickResolvedHash.has_value());
    auto cherryPickedInfo = commits.getCommitInfo(cherryPickResolvedHash.value());
    EXPECT_EQ(cherryPickResolvedHash, commits.getHeadCommitHash());
    EXPECT_NE(secondCommitHash, cherryPickResolvedHash);
    EXPECT_NE(thirdCommitHash, cherryPickResolvedHash);
    EXPECT_EQ(cherryPickedInfo.getMessage(), "Second commit");
    checkCommitAuthorEqualTest(cherryPickedInfo);
    checkCommitCommiterNotEqualTest(cherryPickedInfo);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! Conflict resolved");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "CHERRY_PICK_HEAD"));
}

TEST_F(CherryPickTests, cherryPick_bothConflictAndNotFiles)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto cherryPick = repository->CherryPick();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified.");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World 2!");
    index.add("file.txt");
    index.add("file2.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2.");
    index.add("file.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    auto cherryPickedHash = cherryPick.cherryPickCommit(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP);


    ASSERT_FALSE(cherryPickedHash.has_value());
    EXPECT_EQ(cherryPickedHash.error(), CppGit::Error::CHERRY_PICK_CONFLICT);
    EXPECT_EQ(thirdCommitHash, commits.getHeadCommitHash());
    EXPECT_TRUE(cherryPick.isCherryPickInProgress());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "CHERRY_PICK_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2.\n=======\nHello, World! Modified.\n>>>>>>> " + secondCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello, World 2!");
}

TEST_F(CherryPickTests, cherryPick_conflictTwoFiles)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto cherryPick = repository->CherryPick();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");
    index.add("file.txt");
    index.add("file2.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World! Modified!");
    index.add("file.txt");
    index.add("file2.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World! Modified 2!");
    index.add("file.txt");
    index.add("file2.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    auto cherryPickedHash = cherryPick.cherryPickCommit(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP);


    ASSERT_FALSE(cherryPickedHash.has_value());
    EXPECT_EQ(cherryPickedHash.error(), CppGit::Error::CHERRY_PICK_CONFLICT);
    EXPECT_EQ(thirdCommitHash, commits.getHeadCommitHash());
    EXPECT_TRUE(cherryPick.isCherryPickInProgress());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "CHERRY_PICK_HEAD"), secondCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "<<<<<<< HEAD\nHello, World! Modified 2!\n=======\nHello, World! Modified!\n>>>>>>> " + secondCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "<<<<<<< HEAD\nHello, World! Modified 2!\n=======\nHello, World! Modified!\n>>>>>>> " + secondCommitHash + "\n");
}

TEST_F(CherryPickTests, cherryPick_abort)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto branches = repository->Branches();
    auto cherryPick = repository->CherryPick();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto initialCommitHash = commits.createCommit("Initial commit");
    branches.createBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    index.add("file.txt");
    auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);

    branches.changeCurrentBranch("second-branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified 2");
    index.add("file.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    cherryPick.cherryPickCommit(secondCommitHash, CppGit::CherryPickEmptyCommitStrategy::STOP);

    auto abortResult = cherryPick.cherryPickAbort();


    ASSERT_TRUE(abortResult == CppGit::Error::NO_ERROR);

    EXPECT_EQ(commits.getHeadCommitHash(), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file.txt"), "Hello, World! Modified 2");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "CHERRY_PICK_HEAD"));
}

TEST_F(CherryPickTests, cherryPick_abort_noCherryPickInProgress)
{
    auto cherryPick = repository->CherryPick();
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    auto abortResult = cherryPick.cherryPickAbort();

    EXPECT_EQ(abortResult, CppGit::Error::NO_CHERRY_PICK_IN_PROGRESS);
}
