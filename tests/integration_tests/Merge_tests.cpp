#include "BaseRepositoryFixture.hpp"
#include "Branches.hpp"
#include "Commits.hpp"
#include "Merge.hpp"

#include <fstream>
#include <gtest/gtest.h>

class MergeTests : public BaseRepositoryFixture
{
protected:
    static auto readFile(const std::filesystem::path& filePath) -> std::string
    {
        auto file = std::ifstream{ filePath };
        std::ostringstream content;
        content << file.rdbuf();
        return content.str();
    }
};

TEST_F(MergeTests, canFastForward_emptyRepo_sameBranch)
{
    auto merge = repository->Merge();

    EXPECT_THROW(merge.canFastForward("main"), std::runtime_error);
}

TEST_F(MergeTests, canFastForward_emptyRepo_head)
{
    auto merge = repository->Merge();

    EXPECT_THROW(merge.canFastForward("HEAD"), std::runtime_error);
}

TEST_F(MergeTests, canFastForward_sameBranch)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto merge = repository->Merge();

    EXPECT_TRUE(merge.canFastForward("main"));
}

TEST_F(MergeTests, canFastForward_head)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto merge = repository->Merge();

    EXPECT_TRUE(merge.canFastForward("HEAD"));
}

TEST_F(MergeTests, canFastForward_bothBranchesSameCommit)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    auto merge = repository->Merge();

    EXPECT_TRUE(merge.canFastForward("second_branch"));
}

TEST_F(MergeTests, canFastForward_linearBehind)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");
    branches.changeCurrentBranch("second_branch");

    commits.createCommit("Second commit");

    branches.changeCurrentBranch("main");

    auto merge = repository->Merge();

    EXPECT_TRUE(merge.canFastForward("second_branch"));
}

TEST_F(MergeTests, canFastForward_linearAhead)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    commits.createCommit("Second commit");

    auto merge = repository->Merge();

    EXPECT_FALSE(merge.canFastForward("second_branch"));
}

TEST_F(MergeTests, canFastForward_changesInBothBranches)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    commits.createCommit("Third commit");

    branches.changeCurrentBranch("main");

    auto merge = repository->Merge();

    EXPECT_FALSE(merge.canFastForward("second_branch"));
}

TEST_F(MergeTests, anythingToMerge_emptyRepo_sameBranch)
{
    auto merge = repository->Merge();

    EXPECT_THROW(merge.isAnythingToMerge("main"), std::runtime_error);
}

TEST_F(MergeTests, anythingToMerge_emptyRepo_head)
{
    auto merge = repository->Merge();

    EXPECT_THROW(merge.isAnythingToMerge("HEAD"), std::runtime_error);
}

TEST_F(MergeTests, anythingToMerge_sameBranch)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto merge = repository->Merge();

    EXPECT_FALSE(merge.isAnythingToMerge("main"));
}

TEST_F(MergeTests, anythingToMerge_head)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto merge = repository->Merge();

    EXPECT_FALSE(merge.isAnythingToMerge("HEAD"));
}

TEST_F(MergeTests, anythingToMerge_bothBranchesSameCommit)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    auto merge = repository->Merge();

    EXPECT_FALSE(merge.isAnythingToMerge("second_branch"));
}

TEST_F(MergeTests, anythingToMerge_linearBehind)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");
    branches.changeCurrentBranch("second_branch");

    commits.createCommit("Second commit");

    branches.changeCurrentBranch("main");

    auto merge = repository->Merge();

    EXPECT_TRUE(merge.isAnythingToMerge("second_branch"));
}

TEST_F(MergeTests, anythingToMerge_linearAhead)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    commits.createCommit("Second commit");

    auto merge = repository->Merge();

    EXPECT_FALSE(merge.isAnythingToMerge("second_branch"));
}

TEST_F(MergeTests, anythingToMerge_changesInBothBranches)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    commits.createCommit("Third commit");

    branches.changeCurrentBranch("main");

    auto merge = repository->Merge();

    EXPECT_TRUE(merge.isAnythingToMerge("second_branch"));
}

TEST_F(MergeTests, mergeFastForward_emptyRepo_sameBranch)
{
    auto merge = repository->Merge();

    EXPECT_THROW(merge.mergeFastForward("main"), std::runtime_error);
}

TEST_F(MergeTests, mergeFastForward_sameBranch)
{
    auto commits = repository->Commits();
    auto initialCommitHash = commits.createCommit("Initial commit");

    auto merge = repository->Merge();
    auto mergeCommitHash = merge.mergeFastForward("main");

    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(mergeCommitHash, initialCommitHash);
}

TEST_F(MergeTests, mergeFastForward_bothBranchesSameCommit)
{
    auto commits = repository->Commits();
    auto initialCommitHash = commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    auto merge = repository->Merge();
    auto mergeCommitHash = merge.mergeFastForward("second_branch");

    EXPECT_EQ(commits.getHeadCommitHash(), initialCommitHash);
    EXPECT_EQ(mergeCommitHash, initialCommitHash);
}

TEST_F(MergeTests, mergeFastForward_linearBehind)
{
    auto commits = repository->Commits();
    auto initialCommitHash = commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");
    branches.changeCurrentBranch("second_branch");

    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("main");

    auto merge = repository->Merge();
    auto mergeCommitHash = merge.mergeFastForward("second_branch");

    EXPECT_EQ(commits.getHeadCommitHash(), secondCommitHash);
    EXPECT_EQ(mergeCommitHash, secondCommitHash);
}

TEST_F(MergeTests, mergeFastForward_linearAhead)
{
    auto commits = repository->Commits();
    auto initialCommitHash = commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    auto secondCommitHash = commits.createCommit("Second commit");

    auto merge = repository->Merge();
    auto mergeCommitHash = merge.mergeFastForward("second_branch");

    EXPECT_EQ(commits.getHeadCommitHash(), secondCommitHash);
    EXPECT_EQ(mergeCommitHash, secondCommitHash);
}

TEST_F(MergeTests, mergeFastForward_changesInBothBranches)
{
    auto commits = repository->Commits();
    auto initialCommitHash = commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");
    auto thirdCommitHash = commits.createCommit("Third commit");

    branches.changeCurrentBranch("main");

    auto merge = repository->Merge();

    EXPECT_THROW(merge.mergeFastForward("second_branch"), std::runtime_error);
}

TEST_F(MergeTests, mergeFastForward_dirtyRepo)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto initialCommitHash = commits.createCommit("Initial commit");

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Second commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");
    branches.changeCurrentBranch("second_branch");

    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("main");

    file.open(repositoryPath / "file.txt", std::ios::app);
    file << "Hello, World! Modified.";
    file.close();

    ASSERT_TRUE(index.isDirty());

    auto merge = repository->Merge();
    EXPECT_THROW(merge.mergeFastForward("second_branch"), std::runtime_error);
}

TEST_F(MergeTests, mergeNoFastForward_emptyRepo_sameBranch)
{
    auto merge = repository->Merge();

    EXPECT_THROW(merge.mergeNoFastForward("main", "Merge commit"), std::runtime_error);
}

TEST_F(MergeTests, mergeNoFastForward_sameBranch)
{
    auto commits = repository->Commits();
    auto initialCommitHash = commits.createCommit("Initial commit");

    auto merge = repository->Merge();

    EXPECT_THROW(merge.mergeNoFastForward("main", "merge commit"), std::runtime_error);
}

TEST_F(MergeTests, mergeNoFastForward_bothBranchesSameCommit)
{
    auto commits = repository->Commits();
    auto initialCommitHash = commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    auto merge = repository->Merge();

    EXPECT_THROW(merge.mergeNoFastForward("second_branch", "merge commit"), std::runtime_error);
}

TEST_F(MergeTests, mergeNoFastForward_linearBehind)
{
    auto commits = repository->Commits();
    auto initialCommitHash = commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    std::ofstream file(repositoryPath / "file.txt");
    file.close();

    auto index = repository->Index();
    index.add("file.txt");

    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    auto merge = repository->Merge();
    auto mergeCommitHash = merge.mergeNoFastForward("main", "Merge commit");

    ASSERT_EQ(commits.getHeadCommitHash(), mergeCommitHash);

    auto mergeCommit = commits.getCommitInfo(mergeCommitHash);

    EXPECT_EQ(mergeCommit.getMessage(), "Merge commit");
    ASSERT_EQ(mergeCommit.getParents().size(), 2);
    EXPECT_EQ(mergeCommit.getParents()[0], initialCommitHash);
    EXPECT_EQ(mergeCommit.getParents()[1], secondCommitHash);
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file.txt"));
}

TEST_F(MergeTests, mergeNoFastForward_linearAhead)
{
    auto commits = repository->Commits();
    auto initialCommitHash = commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");
    branches.changeCurrentBranch("second_branch");

    std::ofstream file(repositoryPath / "file.txt");
    file.close();

    auto index = repository->Index();
    index.add("file.txt");

    auto secondCommitHash = commits.createCommit("Second commit");

    auto merge = repository->Merge();

    EXPECT_THROW(merge.mergeNoFastForward("main", "Merge commit"), std::runtime_error);
}

TEST_F(MergeTests, mergeNoFastForward_dirtyRepo)
{
    auto commits = repository->Commits();
    auto index = repository->Index();
    auto initialCommitHash = commits.createCommit("Initial commit");

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Second commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");
    branches.changeCurrentBranch("second_branch");

    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("main");

    file.open(repositoryPath / "file.txt", std::ios::app);
    file << "Hello, World! Modified.";
    file.close();

    ASSERT_TRUE(index.isDirty());

    auto merge = repository->Merge();
    EXPECT_THROW(merge.mergeNoFastForward("second_branch", "merge commit"), std::runtime_error);
}

TEST_F(MergeTests, mergeNoFastForward_changesInBothBranches_noConflict)
{
    auto commits = repository->Commits();
    auto initialCommitHash = commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    std::ofstream file(repositoryPath / "file.txt");
    file.close();

    auto index = repository->Index();
    index.add("file.txt");

    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    std::ofstream file2(repositoryPath / "file2.txt");
    file2.close();
    index.add("file2.txt");

    auto thirdCommitHash = commits.createCommit("Third commit");

    auto merge = repository->Merge();
    auto mergeCommitHash = merge.mergeNoFastForward("main", "Merge commit");

    ASSERT_EQ(commits.getHeadCommitHash(), mergeCommitHash);

    auto mergeCommit = commits.getCommitInfo(mergeCommitHash);

    EXPECT_EQ(mergeCommit.getMessage(), "Merge commit");
    ASSERT_EQ(mergeCommit.getParents().size(), 2);
    EXPECT_EQ(mergeCommit.getParents()[0], thirdCommitHash);
    EXPECT_EQ(mergeCommit.getParents()[1], secondCommitHash);
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file2.txt"));
}

TEST_F(MergeTests, mergeNoFastForward_changesInBothBranches_conflict_sameFileName)
{
    auto commits = repository->Commits();
    auto index = repository->Index();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    file.open(repositoryPath / "file.txt", std::ios::out | std::ios::trunc);
    file << "Hello, World! Modified 1.";
    file.close();
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    file.open(repositoryPath / "file.txt", std::ios::out | std::ios::trunc);
    file << "Hello, World! Modified 2.";
    file.close();
    index.add("file.txt");
    auto thirdCommithash = commits.createCommit("Third commit");

    auto merge = repository->Merge();

    EXPECT_THROW(merge.mergeNoFastForward("main", "Merge commit"), std::runtime_error);

    EXPECT_TRUE(merge.isThereAnyConflict());

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_HEAD"));
    EXPECT_EQ(readFile(repositoryPath / ".git" / "MERGE_HEAD"), secondCommitHash);

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_MSG"));
    EXPECT_EQ(readFile(repositoryPath / ".git" / "MERGE_MSG"), "Merge commit");

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_MODE"));
    EXPECT_EQ(readFile(repositoryPath / ".git" / "MERGE_MODE"), "no-ff");

    ASSERT_TRUE(merge.isMergeInProgress());

    ASSERT_THROW(merge.continueMerge(), std::runtime_error);

    auto fileContent = readFile(repositoryPath / "file.txt");
    EXPECT_EQ(fileContent, "<<<<<<< HEAD\nHello, World! Modified 2.\n=======\nHello, World! Modified 1.\n>>>>>>> main\n");

    file.open(repositoryPath / "file.txt", std::ios::out);
    file << "Hello, World! Merge resolved.";
    file.close();
    index.add("file.txt");

    auto mergeCommitHash = merge.continueMerge();

    EXPECT_FALSE(merge.isMergeInProgress());
    EXPECT_FALSE(merge.isThereAnyConflict());

    ASSERT_EQ(commits.getHeadCommitHash(), mergeCommitHash);

    auto mergeCommit = commits.getCommitInfo(mergeCommitHash);

    EXPECT_EQ(mergeCommit.getMessage(), "Merge commit");
    ASSERT_EQ(mergeCommit.getParents().size(), 2);
    EXPECT_EQ(mergeCommit.getParents()[0], thirdCommithash);
    EXPECT_EQ(mergeCommit.getParents()[1], secondCommitHash);
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file.txt"));

    auto fileContentAfterMerge = readFile(repositoryPath / "file.txt");
    EXPECT_EQ(fileContentAfterMerge, "Hello, World! Merge resolved.");
}

TEST_F(MergeTests, mergeNoFastForward_changesInBothBranches_conflict_sameFileName_abort)
{
    auto commits = repository->Commits();
    auto index = repository->Index();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    file.open(repositoryPath / "file.txt", std::ios::out | std::ios::trunc);
    file << "Hello, World! Modified 1.";
    file.close();
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    file.open(repositoryPath / "file.txt", std::ios::out | std::ios::trunc);
    file << "Hello, World! Modified 2.";
    file.close();
    index.add("file.txt");
    commits.createCommit("Third commit");

    auto merge = repository->Merge();

    EXPECT_THROW(merge.mergeNoFastForward("main", "Merge commit"), std::runtime_error);

    EXPECT_TRUE(merge.isThereAnyConflict());

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_HEAD"));
    EXPECT_EQ(readFile(repositoryPath / ".git" / "MERGE_HEAD"), secondCommitHash);

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_MSG"));
    EXPECT_EQ(readFile(repositoryPath / ".git" / "MERGE_MSG"), "Merge commit");

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_MODE"));
    EXPECT_EQ(readFile(repositoryPath / ".git" / "MERGE_MODE"), "no-ff");

    ASSERT_TRUE(merge.isMergeInProgress());

    merge.abortMerge();

    EXPECT_FALSE(merge.isMergeInProgress());
    EXPECT_FALSE(index.isDirty());
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_HEAD"));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_MSG"));
    EXPECT_FALSE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_MODE"));

    auto fileContent = readFile(repositoryPath / "file.txt");

    EXPECT_EQ(fileContent, "Hello, World! Modified 2.");
}

TEST_F(MergeTests, mergeNoFastForward_changesInBothBranches_conflict_sameFileName_notExistsInAncestorYet)
{
    auto commits = repository->Commits();
    auto index = repository->Index();

    commits.createCommit("Initial commit");

    auto branches = repository->Branches();
    branches.createBranch("second_branch");

    std::ofstream file(repositoryPath / "file.txt", std::ios::out);
    file << "Hello, World! Modified 1.";
    file.close();
    index.add("file.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

    branches.changeCurrentBranch("second_branch");

    file.open(repositoryPath / "file.txt", std::ios::out);
    file << "Hello, World! Modified 2.";
    file.close();
    index.add("file.txt");
    auto thirdCommithash = commits.createCommit("Third commit");

    auto merge = repository->Merge();

    EXPECT_THROW(merge.mergeNoFastForward("main", "Merge commit"), std::runtime_error);

    EXPECT_TRUE(merge.isThereAnyConflict());

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_HEAD"));
    EXPECT_EQ(readFile(repositoryPath / ".git" / "MERGE_HEAD"), secondCommitHash);

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_MSG"));
    EXPECT_EQ(readFile(repositoryPath / ".git" / "MERGE_MSG"), "Merge commit");

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / ".git" / "MERGE_MODE"));
    EXPECT_EQ(readFile(repositoryPath / ".git" / "MERGE_MODE"), "no-ff");

    ASSERT_TRUE(merge.isMergeInProgress());

    ASSERT_THROW(merge.continueMerge(), std::runtime_error);

    auto fileContent = readFile(repositoryPath / "file.txt");
    EXPECT_EQ(fileContent, "<<<<<<< HEAD\nHello, World! Modified 2.\n=======\nHello, World! Modified 1.\n>>>>>>> main\n");

    file.open(repositoryPath / "file.txt", std::ios::out);
    file << "Hello, World! Merge resolved.";
    file.close();
    index.add("file.txt");

    auto mergeCommitHash = merge.continueMerge();

    EXPECT_FALSE(merge.isMergeInProgress());
    EXPECT_FALSE(merge.isThereAnyConflict());

    ASSERT_EQ(commits.getHeadCommitHash(), mergeCommitHash);

    auto mergeCommit = commits.getCommitInfo(mergeCommitHash);

    EXPECT_EQ(mergeCommit.getMessage(), "Merge commit");
    ASSERT_EQ(mergeCommit.getParents().size(), 2);
    EXPECT_EQ(mergeCommit.getParents()[0], thirdCommithash);
    EXPECT_EQ(mergeCommit.getParents()[1], secondCommitHash);
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "file.txt"));

    auto fileContentAfterMerge = readFile(repositoryPath / "file.txt");
    EXPECT_EQ(fileContentAfterMerge, "Hello, World! Merge resolved.");
}
