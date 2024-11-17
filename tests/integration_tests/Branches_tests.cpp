#include "BaseRepositoryFixture.hpp"
#include "Branch.hpp"
#include "Branches.hpp"
#include "Commits.hpp"
#include "Index.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

class BranchesTests : public BaseRepositoryFixture
{
public:
    void SetUp() override
    {
        BaseRepositoryFixture::SetUp();
        const auto& commits = repository->Commits();
        initialCommitHash = commits.createCommit("Initial commit");
    }

protected:
    std::string initialCommitHash;
};

TEST_F(BranchesTests, branchesAfterInitialCommit_getAllBranches)
{
    const auto& branches = repository->Branches();
    const auto allBranches = branches.getAllBranches();

    ASSERT_EQ(allBranches.size(), 1);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, branchesAfterInitialCommit_getCurrentBranchRef)
{
    const auto& branches = repository->Branches();
    const auto currentBranchRef = branches.getCurrentBranchRef();

    EXPECT_EQ(currentBranchRef, "refs/heads/main");
}

TEST_F(BranchesTests, branchesAfterInitialCommit_getLocalBranches)
{
    const auto& branches = repository->Branches();
    const auto allBranches = branches.getLocalBranches();

    ASSERT_EQ(allBranches.size(), 1);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, branchesAfterInitialCommit_getRemoteBranches)
{
    const auto& branches = repository->Branches();
    const auto allBranches = branches.getRemoteBranches();

    ASSERT_EQ(allBranches.size(), 0);
}

TEST_F(BranchesTests, createLocalBranch_fullName)
{
    const auto& branches = repository->Branches();
    branches.createBranch("refs/heads/new_branch");

    const auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");

    const auto currentBranch = branches.getCurrentBranchRef();
    EXPECT_EQ(currentBranch, "refs/heads/main");

    const auto hashBranchRefersTo = branches.getHashBranchRefersTo("refs/heads/new_branch");
    EXPECT_EQ(hashBranchRefersTo, initialCommitHash);
}

TEST_F(BranchesTests, createLocalBranch_shortName)
{
    const auto& branches = repository->Branches();
    branches.createBranch("new_branch");

    const auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");

    const auto currentBranch = branches.getCurrentBranchRef();
    EXPECT_EQ(currentBranch, "refs/heads/main");

    const auto hashBranchRefersTo = branches.getHashBranchRefersTo("new_branch");
    EXPECT_EQ(hashBranchRefersTo, initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromBranch_fullName)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();
    branches.createBranch("new_branch");

    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");

    auto secondCommitHash = commits.createCommit("second commit");
    branches.createBranchFromBranch("refs/heads/new_branch_from_new", allBranches[1]);

    allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 3);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    EXPECT_EQ(allBranches[2].getRefName(), "refs/heads/new_branch_from_new");

    EXPECT_EQ(branches.getHashBranchRefersTo("main"), secondCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("new_branch"), initialCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("new_branch_from_new"), initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromBranch_shortName)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();
    branches.createBranch("new_branch");

    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");

    auto secondCommitHash = commits.createCommit("second commit");
    branches.createBranchFromBranch("new_branch_from_new", allBranches[1]);

    allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 3);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    EXPECT_EQ(allBranches[2].getRefName(), "refs/heads/new_branch_from_new");

    EXPECT_EQ(branches.getHashBranchRefersTo("main"), secondCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("new_branch"), initialCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("new_branch_from_new"), initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromCommit_fullName)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();

    auto secondCommitHash = commits.createCommit("second commit");
    branches.createBranchFromCommit("refs/heads/new_branch_from_commit", initialCommitHash);

    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch_from_commit");

    EXPECT_EQ(branches.getHashBranchRefersTo("main"), secondCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("refs/heads/new_branch_from_commit"), initialCommitHash);
}

TEST_F(BranchesTests, createBranchFromCommit_shortName)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();

    auto secondCommitHash = commits.createCommit("second commit");
    branches.createBranchFromCommit("new_branch_from_commit", initialCommitHash);

    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch_from_commit");

    EXPECT_EQ(branches.getHashBranchRefersTo("main"), secondCommitHash);
    EXPECT_EQ(branches.getHashBranchRefersTo("refs/heads/new_branch_from_commit"), initialCommitHash);
}

TEST_F(BranchesTests, deleteBranch_fullName)
{
    const auto& branches = repository->Branches();
    branches.createBranch("new_branch");

    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");

    branches.deleteBranch("refs/heads/new_branch");

    allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 1);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, deleteBranch_shortName)
{
    const auto& branches = repository->Branches();
    branches.createBranch("new_branch");

    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");

    branches.deleteBranch("new_branch");

    allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 1);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
}

TEST_F(BranchesTests, getCurrentBranchHashRefsTo_fullName)
{
    const auto& branches = repository->Branches();
    auto hash = branches.getHashBranchRefersTo("refs/heads/main");

    EXPECT_EQ(hash, initialCommitHash);
}

TEST_F(BranchesTests, getCurrentBranchHashRefsTo_shortName)
{
    const auto& branches = repository->Branches();
    auto hash = branches.getHashBranchRefersTo("main");

    EXPECT_EQ(hash, initialCommitHash);
}

TEST_F(BranchesTests, changeBranchRef_fullName)
{
    const auto& commits = repository->Commits();
    const auto& branches = repository->Branches();
    auto secondCommitHash = commits.createCommit("second commit");

    auto hashBeforeChange = branches.getHashBranchRefersTo("main");

    ASSERT_EQ(hashBeforeChange, secondCommitHash);

    branches.changeBranchRef("refs/heads/main", initialCommitHash);
    auto hashAfterChange = branches.getHashBranchRefersTo("main");

    EXPECT_EQ(hashAfterChange, initialCommitHash);
}

TEST_F(BranchesTests, changeBranchRef_shortName)
{
    const auto& commits = repository->Commits();
    const auto& branches = repository->Branches();
    auto secondCommitHash = commits.createCommit("second commit");

    auto hashBeforeChange = branches.getHashBranchRefersTo("main");

    ASSERT_EQ(hashBeforeChange, secondCommitHash);

    branches.changeBranchRef("main", initialCommitHash);
    auto hashAfterChange = branches.getHashBranchRefersTo("main");

    EXPECT_EQ(hashAfterChange, initialCommitHash);
}

TEST_F(BranchesTests, currentBranchInfo)
{
    const auto& branches = repository->Branches();
    const auto currentBranch = branches.getCurrentBranch();

    EXPECT_EQ(currentBranch.getRefName(), "refs/heads/main");
    EXPECT_EQ(currentBranch.getUpstreamPull(), "");
    EXPECT_EQ(currentBranch.getUpstreamPush(), "");
    EXPECT_TRUE(currentBranch.isLocalBranch());
}

TEST_F(BranchesTests, changeBranch_shortName)
{
    const auto& branches = repository->Branches();
    branches.createBranch("new_branch");

    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    EXPECT_EQ(branches.getCurrentBranchRef(), "refs/heads/main");

    branches.changeCurrentBranch("new_branch");

    EXPECT_EQ(branches.getCurrentBranchRef(), "refs/heads/new_branch");
}

TEST_F(BranchesTests, changeBranch_fullName)
{
    const auto& branches = repository->Branches();
    branches.createBranch("new_branch");

    auto allBranches = branches.getAllBranches();
    ASSERT_EQ(allBranches.size(), 2);
    EXPECT_EQ(allBranches[0].getRefName(), "refs/heads/main");
    EXPECT_EQ(allBranches[1].getRefName(), "refs/heads/new_branch");
    EXPECT_EQ(branches.getCurrentBranchRef(), "refs/heads/main");

    branches.changeCurrentBranch("refs/heads/new_branch");

    EXPECT_EQ(branches.getCurrentBranchRef(), "refs/heads/new_branch");
}

TEST_F(BranchesTests, changeCurrentBranchRef)
{
    const auto& commits = repository->Commits();
    const auto& branches = repository->Branches();
    auto secondCommitHash = commits.createCommit("second commit");

    auto hashBeforeChange = branches.getHashBranchRefersTo("main");

    ASSERT_EQ(hashBeforeChange, secondCommitHash);

    branches.changeCurrentBranchRef(initialCommitHash);
    auto hashAfterChange = branches.getHashBranchRefersTo("main");

    EXPECT_EQ(hashAfterChange, initialCommitHash);
}

TEST_F(BranchesTests, changeBranch_shouldDeleteFile)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();
    const auto& index = repository->Index();
    branches.createBranch("new_branch");
    branches.changeCurrentBranch("new_branch");

    ASSERT_EQ(branches.getCurrentBranchRef(), "refs/heads/new_branch");

    std::ofstream file(repositoryPath / "new_file.txt");
    file.close();
    index.add("new_file.txt");
    commits.createCommit("Added new file");

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / "new_file.txt"));

    branches.changeCurrentBranch("main");
    ASSERT_EQ(branches.getCurrentBranchRef(), "refs/heads/main");

    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "new_file.txt"));
}

TEST_F(BranchesTests, changeBranch_shouldCreateFile)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();
    const auto& index = repository->Index();
    branches.createBranch("new_branch");

    std::ofstream file(repositoryPath / "new_file.txt");
    file.close();
    index.add("new_file.txt");
    commits.createCommit("Added new file");

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / "new_file.txt"));

    branches.changeCurrentBranch("new_branch");
    ASSERT_EQ(branches.getCurrentBranchRef(), "refs/heads/new_branch");

    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "new_file.txt"));

    branches.changeCurrentBranch("main");
    ASSERT_EQ(branches.getCurrentBranchRef(), "refs/heads/main");

    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "new_file.txt"));
}

TEST_F(BranchesTests, changeBranch_shouldChangeFileContent)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();
    const auto& index = repository->Index();
    branches.createBranch("new_branch");

    std::ofstream file(repositoryPath / "new_file.txt");
    file << "Initial content";
    file.close();
    index.add("new_file.txt");
    commits.createCommit("Added new file");

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / "new_file.txt"));

    branches.changeCurrentBranch("new_branch");
    ASSERT_EQ(branches.getCurrentBranchRef(), "refs/heads/new_branch");

    std::ofstream file2(repositoryPath / "new_file.txt");
    file2 << "Changed content";
    file2.close();
    index.add("new_file.txt");
    commits.createCommit("Changed file content");

    auto new_fileContent = getFileContent(repositoryPath / "new_file.txt");
    ASSERT_EQ(new_fileContent, "Changed content");

    branches.changeCurrentBranch("main");
    ASSERT_EQ(branches.getCurrentBranchRef(), "refs/heads/main");

    new_fileContent = getFileContent(repositoryPath / "new_file.txt");
    EXPECT_EQ(new_fileContent, "Initial content");
}

TEST_F(BranchesTests, changeBranch_shouldKeepUntrackedFile)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();
    const auto& index = repository->Index();

    branches.createBranch("new_branch");
    branches.changeCurrentBranch("new_branch");
    ASSERT_EQ(branches.getCurrentBranchRef(), "refs/heads/new_branch");

    std::ofstream tracked(repositoryPath / "tracked.txt");
    tracked << "Initial content";
    tracked.close();
    index.add("tracked.txt");
    commits.createCommit("Added tracked file");

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / "tracked.txt"));

    std::ofstream untracked(repositoryPath / "untracked.txt");
    untracked << "New file";
    untracked.close();

    ASSERT_TRUE(std::filesystem::exists(repositoryPath / "untracked.txt"));

    branches.changeCurrentBranch("main");
    ASSERT_EQ(branches.getCurrentBranchRef(), "refs/heads/main");

    EXPECT_FALSE(std::filesystem::exists(repositoryPath / "tracked.txt"));
    EXPECT_TRUE(std::filesystem::exists(repositoryPath / "untracked.txt"));
}

TEST_F(BranchesTests, changeBranch_shouldFailIfWorktreeDirty)
{
    const auto& branches = repository->Branches();
    const auto& commits = repository->Commits();
    const auto& index = repository->Index();
    branches.createBranch("new_branch");

    std::ofstream file(repositoryPath / "new_file.txt");
    file.close();
    index.add("new_file.txt");
    commits.createCommit("Added new file");

    std::ofstream file2(repositoryPath / "new_file.txt");
    file2 << "Changed content";
    file2.close();
    index.add("new_file.txt");

    ASSERT_THROW(branches.changeCurrentBranch("new_branch"), std::runtime_error);
}
