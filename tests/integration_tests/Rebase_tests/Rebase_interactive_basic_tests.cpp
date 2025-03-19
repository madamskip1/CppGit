
#include "RebaseFixture.hpp"

#include <CppGit/BranchesManager.hpp>
#include <CppGit/CommitsLogManager.hpp>
#include <CppGit/CommitsManager.hpp>
#include <CppGit/IndexManager.hpp>
#include <CppGit/RebaseTodoCommand.hpp>
#include <CppGit/Rebaser.hpp>
#include <CppGit/_details/FileUtility.hpp>
#include <gtest/gtest.h>

class RebaseInteractiveBasicTests : public RebaseFixture
{
};

TEST_F(RebaseInteractiveBasicTests, getTodoCommandsList_anotherOnto)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    indexManager.add("file1.txt");
    commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    const auto todoCommands = rebaser.getDefaultTodoCommands("main");

    ASSERT_EQ(todoCommands.size(), 2);
    const auto& firstCommand = todoCommands[0];
    EXPECT_EQ(firstCommand.type, CppGit::RebaseTodoCommandType::PICK);
    EXPECT_EQ(firstCommand.hash, thirdCommitHash);
    EXPECT_EQ(firstCommand.message, "Third commit");
    const auto& secondCommand = todoCommands[1];
    EXPECT_EQ(secondCommand.type, CppGit::RebaseTodoCommandType::PICK);
    EXPECT_EQ(secondCommand.hash, fourthCommitHash);
    EXPECT_EQ(secondCommand.message, "Fourth commit");
}

TEST_F(RebaseInteractiveBasicTests, getTodoCommandsList)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    indexManager.add("file1.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    const auto todoCommands = rebaser.getDefaultTodoCommands(secondCommitHash);

    ASSERT_EQ(todoCommands.size(), 2);
    const auto& firstCommand = todoCommands[0];
    EXPECT_EQ(firstCommand.type, CppGit::RebaseTodoCommandType::PICK);
    EXPECT_EQ(firstCommand.hash, thirdCommitHash);
    EXPECT_EQ(firstCommand.message, "Third commit");
    const auto& secondCommand = todoCommands[1];
    EXPECT_EQ(secondCommand.type, CppGit::RebaseTodoCommandType::PICK);
    EXPECT_EQ(secondCommand.hash, fourthCommitHash);
    EXPECT_EQ(secondCommand.message, "Fourth commit");
}


TEST_F(RebaseInteractiveBasicTests, ontoAnotherBranch)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    branchesManager.createBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    indexManager.add("file1.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");

    branchesManager.changeBranch("second_branch");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", initialCommitHash);
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);

    const auto todoCommands = rebaser.getDefaultTodoCommands("main");

    const auto rebaseResult = rebaser.interactiveRebase("main", todoCommands);

    ASSERT_TRUE(rebaseResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), rebaseResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/second_branch");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), rebaseResult.value());
    EXPECT_EQ(branchesManager.getCurrentBranchName(), "refs/heads/second_branch");

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 4);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getHash(), secondCommitHash);
    EXPECT_EQ(log[2].getMessage(), "Third commit");
    EXPECT_EQ(log[2].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(log[2]);
    EXPECT_EQ(log[3].getMessage(), "Fourth commit");
    EXPECT_EQ(log[3].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(log[3]);

    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveBasicTests, sameBranch)
{
    const auto rebaser = repository->Rebaser();
    const auto commitsManager = repository->CommitsManager();
    const auto branchesManager = repository->BranchesManager();
    const auto indexManager = repository->IndexManager();
    auto commitsLogManager = repository->CommitsLogManager();
    commitsLogManager.setOrder(CppGit::CommitsLogManager::Order::REVERSE);


    const auto initialCommitHash = commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "");
    indexManager.add("file1.txt");
    const auto secondCommitHash = commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "");
    indexManager.add("file2.txt");
    const auto thirdCommitHash = commitsManager.createCommit("Third commit");
    const auto fourthCommitHash = commitsManager.createCommit("Fourth commit", thirdCommitHash);

    const auto todoCommands = rebaser.getDefaultTodoCommands(secondCommitHash);

    const auto rebaseResult = rebaser.interactiveRebase(secondCommitHash, todoCommands);


    ASSERT_TRUE(rebaseResult.has_value());

    EXPECT_EQ(commitsManager.getHeadCommitHash(), rebaseResult.value());
    const auto currentBranchName = branchesManager.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branchesManager.getHashBranchRefersTo(currentBranchName), rebaseResult.value());

    const auto log = commitsLogManager.getCommitsLogDetailed();
    ASSERT_EQ(log.size(), 4);
    EXPECT_EQ(log[0].getHash(), initialCommitHash);
    EXPECT_EQ(log[1].getHash(), secondCommitHash);
    EXPECT_EQ(log[2].getHash(), thirdCommitHash);
    EXPECT_EQ(log[3].getHash(), fourthCommitHash);

    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}
