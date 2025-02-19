#include "Branches.hpp"
#include "Commits.hpp"
#include "CommitsHistory.hpp"
#include "Index.hpp"
#include "Rebase.hpp"
#include "RebaseFixture.hpp"
#include "RebaseTodoCommand.hpp"
#include "_details/FileUtility.hpp"
#include "_details/Parser/Parser.hpp"

#include <gtest/gtest.h>
#include <regex>

class RebaseInteractiveFixupTest : public RebaseFixture
{
};

TEST_F(RebaseInteractiveFixupTest, fixupCommit)
{
    const auto commits = repository->Commits();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    const auto branches = repository->Branches();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    const auto thirdCommitHash = commits.createCommit("Third commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::FIXUP;

    const auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);

    ASSERT_TRUE(rebaseResult.has_value());

    EXPECT_EQ(commits.getHeadCommitHash(), rebaseResult.value());
    const auto currentBranchName = branches.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branches.getHashBranchRefersTo(currentBranchName), rebaseResult.value());

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(commitsLog[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}

TEST_F(RebaseInteractiveFixupTest, fixupTwoCommits)
{
    const auto commits = repository->Commits();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    const auto branches = repository->Branches();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    commits.createCommit("Third commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    const auto fourthCommitHash = commits.createCommit("Fourth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;

    const auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_TRUE(rebaseResult.has_value());

    EXPECT_EQ(commits.getHeadCommitHash(), rebaseResult.value());
    const auto currentBranchName = branches.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branches.getHashBranchRefersTo(currentBranchName), rebaseResult.value());

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(commitsLog[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveFixupTest, breakAfter_noRewrritenListsBefore)
{
    const auto commits = repository->Commits();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    const auto thirdCommitHash = commits.createCommit("Third commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    const auto fourthCommitHash = commits.createCommit("Fourth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace(todoCommands.begin() + 2, CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(commitsLog[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file3.txt"));

    const auto doneFileExpected = "pick " + secondCommitHash + " Second commit\n"
                                + "fixup " + thirdCommitHash + " Third commit\n"
                                + "break\n";
    const auto rewrittenListExpected = secondCommitHash + " " + commits.getHeadCommitHash() + "\n"
                                     + thirdCommitHash + " " + commits.getHeadCommitHash() + "\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "pick " + fourthCommitHash + " Fourth commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), rewrittenListExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveFixupTest, breakAfter_rewrritenListsBefore)
{
    const auto commits = repository->Commits();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file4.txt");
    const auto fifthCommitHash = commits.createCommit("Fifth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace(todoCommands.begin() + 4, CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(commitsLog[1]);
    EXPECT_EQ(commitsLog[2].getMessage(), "Fourth commit");
    EXPECT_EQ(commitsLog[2].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(commitsLog[2]);

    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    const auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                                + "pick " + thirdCommitHash + " Third commit\n"
                                + "pick " + fourthCommitHash + " Fourth commit\n"
                                + "fixup " + fifthCommitHash + " Fifth commit\n"
                                + "break\n";
    const auto rewrittenListExpected = thirdCommitHash + " " + commitsLog[1].getHash() + "\n"
                                     + fourthCommitHash + " " + commits.getHeadCommitHash() + "\n"
                                     + fifthCommitHash + " " + commits.getHeadCommitHash() + "\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fifthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), rewrittenListExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fifthCommitHash);
}

TEST_F(RebaseInteractiveFixupTest, fixupAfterBreak_breakAfter_noRewrritenListsBefore)
{
    const auto commits = repository->Commits();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    const auto secondCommitHash = createCommitWithTestAuthorCommiter("Second commit", initialCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    const auto thirdCommitHash = commits.createCommit("Third commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[1].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace(todoCommands.begin() + 1, CppGit::RebaseTodoCommandType::BREAK);
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);

    const auto continueRebaseResult = rebase.continueRebase();


    ASSERT_FALSE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.error(), CppGit::Error::REBASE_BREAK);

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Second commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(commitsLog[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");

    const auto doneFileExpected = "pick " + secondCommitHash + " Second commit\n"
                                + "break\n"
                                + "fixup " + thirdCommitHash + " Third commit\n"
                                + "break\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), thirdCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), thirdCommitHash + " " + commitsLog[1].getHash() + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), thirdCommitHash);
}

TEST_F(RebaseInteractiveFixupTest, fixupAfterBreak_breakAfter_rewrritenListsBefore)
{
    const auto commits = repository->Commits();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    const auto fourthCommitHash = createCommitWithTestAuthorCommiter("Fourth commit", thirdCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file4.txt");
    const auto fifthCommitHash = commits.createCommit("Fifth commit", fourthCommitHash);

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace(todoCommands.begin() + 3, CppGit::RebaseTodoCommandType::BREAK);
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);

    const auto continueRebaseResult = rebase.continueRebase();


    ASSERT_FALSE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.error(), CppGit::Error::REBASE_BREAK);

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 3);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(commitsLog[1]);
    EXPECT_EQ(commitsLog[2].getMessage(), "Fourth commit");
    EXPECT_EQ(commitsLog[2].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(commitsLog[2]);

    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file1.txt"));

    const auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                                + "pick " + thirdCommitHash + " Third commit\n"
                                + "pick " + fourthCommitHash + " Fourth commit\n"
                                + "break\n"
                                + "fixup " + fifthCommitHash + " Fifth commit\n"
                                + "break\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fifthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fifthCommitHash);
    // When we do pick, break and then fixup
    // rewritten-list should contains previous rewritten from pick
    // and new rewritten from fixup
    const auto rewrittenList = CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list");
    const auto splittedRewrittenList = CppGit::Parser::splitToStringViewsVector(rewrittenList, "\n");
    EXPECT_EQ(splittedRewrittenList[0], thirdCommitHash + " " + commitsLog[1].getHash());
    EXPECT_NE(splittedRewrittenList[1], fourthCommitHash + " " + commitsLog[2].getHash());
    EXPECT_TRUE(std::regex_match(std::string{ splittedRewrittenList[1] }, std::regex{ "(^" + fourthCommitHash + " " + R"([\d\w]{40})$)" }));
    EXPECT_EQ(splittedRewrittenList[2], fifthCommitHash + " " + commitsLog[2].getHash());
}

TEST_F(RebaseInteractiveFixupTest, fixupAfterBreak_breakAfter_pickAsFirstRewrriten)
{
    const auto commits = repository->Commits();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file3.txt");
    const auto fourthCommitHash = commits.createCommit("Fourth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace(todoCommands.begin() + 2, CppGit::RebaseTodoCommandType::BREAK);
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_BREAK);

    const auto rebaseContinueResult = rebase.continueRebase();


    ASSERT_FALSE(rebaseContinueResult.has_value());
    EXPECT_EQ(rebaseContinueResult.error(), CppGit::Error::REBASE_BREAK);

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "");
    checkTestAuthorPreservedCommitterModified(commitsLog[1]);

    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file1.txt"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    const auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                                + "pick " + thirdCommitHash + " Third commit\n"
                                + "break\n"
                                + "fixup " + fourthCommitHash + " Fourth commit\n"
                                + "break\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
    // When we do pick, break and then fixup
    // rewritten-list should contains: previous rewritten from pick
    // and new rewritten from fixup
    const auto rewrittenList = CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list");
    const auto splittedRewrittenList = CppGit::Parser::splitToStringViewsVector(rewrittenList, "\n");
    EXPECT_NE(splittedRewrittenList[0], thirdCommitHash + " " + commitsLog[1].getHash());
    EXPECT_TRUE(std::regex_match(std::string{ splittedRewrittenList[0] }, std::regex{ "(^" + thirdCommitHash + " " + R"([\d\w]{40})$)" }));
    EXPECT_EQ(splittedRewrittenList[1], fourthCommitHash + " " + commitsLog[1].getHash());
}

TEST_F(RebaseInteractiveFixupTest, conflictDuringLastFixup_stop)
{
    const auto commits = repository->Commits();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", "Third commit description", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file1.txt");
    index.add("file3.txt");
    const auto fourthCommitHash = commits.createCommit("Fourth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;

    const auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description");
    checkTestAuthorPreservedCommitterModified(commitsLog[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\n=======\nHello World 1, new!\n>>>>>>> " + fourthCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    constexpr auto* expectedMessage = "Third commit\n\nThird commit description";
    EXPECT_EQ(rebase.getStoppedMessage(), expectedMessage);

    const auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                                + "pick " + thirdCommitHash + " Third commit\n"
                                + "fixup " + fourthCommitHash + " Fourth commit\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "amend"), commits.getHeadCommitHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "current-fixups"), "fixup " + fourthCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), expectedMessage);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-pending"), thirdCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveFixupTest, conflictDuringLastFixup_continue)
{
    const auto commits = repository->Commits();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    const auto branches = repository->Branches();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    createCommitWithTestAuthorCommiter("Third commit", "Third commit description", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file1.txt");
    index.add("file3.txt");
    const auto fourthCommitHash = commits.createCommit("Fourth commit", "Fourth commit description");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;

    const auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    index.add("file1.txt");
    const auto continueRebaseResult = rebase.continueRebase();


    ASSERT_TRUE(continueRebaseResult.has_value());

    EXPECT_EQ(commits.getHeadCommitHash(), continueRebaseResult.value());
    const auto currentBranchName = branches.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branches.getHashBranchRefersTo(currentBranchName), continueRebaseResult.value());

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description");
    checkTestAuthorPreservedCommitterModified(commitsLog[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveFixupTest, conflictDuringLastFixup_breakAfterContinue)
{
    const auto commits = repository->Commits();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", "Third commit description", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file1.txt");
    index.add("file3.txt");
    const auto fourthCommitHash = commits.createCommit("Fourth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    index.add("file1.txt");
    const auto continueRebaseResult = rebase.continueRebase();


    ASSERT_FALSE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.error(), CppGit::Error::REBASE_BREAK);

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description");
    checkTestAuthorPreservedCommitterModified(commitsLog[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");

    const auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                                + "pick " + thirdCommitHash + " Third commit\n"
                                + "fixup " + fourthCommitHash + " Fourth commit\n"
                                + "break\n";
    const auto rewrittenListExpected = thirdCommitHash + " " + commitsLog[1].getHash() + "\n"
                                     + fourthCommitHash + " " + commitsLog[1].getHash() + "\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), rewrittenListExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fourthCommitHash);
}

TEST_F(RebaseInteractiveFixupTest, conflictDuringNotLastFixup_stop)
{
    const auto commits = repository->Commits();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", "Third commit description", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file1.txt");
    index.add("file3.txt");
    const auto fourthCommitHash = commits.createCommit("Fourth commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file4.txt");
    const auto fifthCommitHash = commits.createCommit("Fifth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;

    const auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);


    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description");
    checkTestAuthorPreservedCommitterModified(commitsLog[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "<<<<<<< HEAD\n=======\nHello World 1, new!\n>>>>>>> " + fourthCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "file4.txt"));

    constexpr auto* expectedMessage = "Third commit\n\nThird commit description";
    EXPECT_EQ(rebase.getStoppedMessage(), expectedMessage);

    const auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                                + "pick " + thirdCommitHash + " Third commit\n"
                                + "fixup " + fourthCommitHash + " Fourth commit\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-list"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "REBASE_HEAD"), fourthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "amend"), commits.getHeadCommitHash());
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "current-fixups"), "fixup " + fourthCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "fixup " + fifthCommitHash + " Fifth commit\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "message"), expectedMessage);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fifthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-pending"), thirdCommitHash + "\n");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fifthCommitHash);
}

TEST_F(RebaseInteractiveFixupTest, conflictDuringNotLastFixup_continue)
{
    const auto commits = repository->Commits();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    const auto branches = repository->Branches();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    createCommitWithTestAuthorCommiter("Third commit", "Third commit description", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file1.txt");
    index.add("file3.txt");
    commits.createCommit("Fourth commit", "Fourth commit description");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file4.txt");
    const auto fifthCommitHash = commits.createCommit("Fifth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;

    const auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    index.add("file1.txt");
    const auto continueRebaseResult = rebase.continueRebase();


    ASSERT_TRUE(continueRebaseResult.has_value());

    EXPECT_EQ(commits.getHeadCommitHash(), continueRebaseResult.value());
    const auto currentBranchName = branches.getCurrentBranchName();
    EXPECT_EQ(currentBranchName, "refs/heads/main");
    EXPECT_EQ(branches.getHashBranchRefersTo(currentBranchName), continueRebaseResult.value());

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description");
    checkTestAuthorPreservedCommitterModified(commitsLog[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fifthCommitHash);
}

TEST_F(RebaseInteractiveFixupTest, conflictDuringNotLastFixup_breakAfterContinue)
{
    const auto commits = repository->Commits();
    const auto rebase = repository->Rebase();
    const auto index = repository->Index();
    auto commitsHistory = repository->CommitsHistory();
    commitsHistory.setOrder(CppGit::CommitsHistory::Order::REVERSE);


    const auto initialCommitHash = commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1!");
    index.add("file1.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello World 2!");
    index.add("file2.txt");
    const auto thirdCommitHash = createCommitWithTestAuthorCommiter("Third commit", "Third commit description", secondCommitHash);
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, new!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file3.txt", "Hello World 3!");
    index.add("file1.txt");
    index.add("file3.txt");
    const auto fourthCommitHash = commits.createCommit("Fourth commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file4.txt", "Hello World 4!");
    index.add("file4.txt");
    const auto fifthCommitHash = commits.createCommit("Fifth commit");

    auto todoCommands = rebase.getDefaultTodoCommands(initialCommitHash);
    todoCommands[0].type = CppGit::RebaseTodoCommandType::DROP;
    todoCommands[2].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands[3].type = CppGit::RebaseTodoCommandType::FIXUP;
    todoCommands.emplace_back(CppGit::RebaseTodoCommandType::BREAK);

    const auto rebaseResult = rebase.interactiveRebase(initialCommitHash, todoCommands);
    ASSERT_FALSE(rebaseResult.has_value());
    EXPECT_EQ(rebaseResult.error(), CppGit::Error::REBASE_CONFLICT);

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello World 1, resolved!");
    index.add("file1.txt");
    const auto continueRebaseResult = rebase.continueRebase();


    ASSERT_FALSE(continueRebaseResult.has_value());
    EXPECT_EQ(continueRebaseResult.error(), CppGit::Error::REBASE_BREAK);

    const auto commitsLog = commitsHistory.getCommitsLogDetailed();
    ASSERT_EQ(commitsLog.size(), 2);
    EXPECT_EQ(commitsLog[0].getHash(), initialCommitHash);
    EXPECT_EQ(commitsLog[1].getMessage(), "Third commit");
    EXPECT_EQ(commitsLog[1].getDescription(), "Third commit description");
    checkTestAuthorPreservedCommitterModified(commitsLog[1]);

    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file1.txt"), "Hello World 1, resolved!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file2.txt"), "Hello World 2!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file3.txt"), "Hello World 3!");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / "file4.txt"), "Hello World 4!");

    const auto doneFileExpected = "drop " + secondCommitHash + " Second commit\n"
                                + "pick " + thirdCommitHash + " Third commit\n"
                                + "fixup " + fourthCommitHash + " Fourth commit\n"
                                + "fixup " + fifthCommitHash + " Fifth commit\n"
                                + "break\n";
    const auto rewrittenListExpected = thirdCommitHash + " " + commitsLog[1].getHash() + "\n"
                                     + fourthCommitHash + " " + commitsLog[1].getHash() + "\n"
                                     + fifthCommitHash + " " + commitsLog[1].getHash() + "\n";
    ASSERT_TRUE(std::filesystem::exists(rebaseDirPath));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "REBASE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "amend"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "author-script"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "current-fixups"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "message"));
    EXPECT_FALSE(std::filesystem::exists(rebaseDirPath / "rewritten-pending"));
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "done"), doneFileExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "git-rebase-todo"), "");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "head-name"), "refs/heads/main");
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "onto"), initialCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "orig-head"), fifthCommitHash);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(rebaseDirPath / "rewritten-list"), rewrittenListExpected);
    EXPECT_EQ(CppGit::_details::FileUtility::readFile(repositoryPath / ".git" / "ORIG_HEAD"), fifthCommitHash);
}
