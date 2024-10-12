#include "BaseRepositoryFixture.hpp"
#include "Commits.hpp"
#include "Diff.hpp"
#include "Index.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

class DiffTests : public BaseRepositoryFixture
{ };

TEST_F(DiffTests, emptyRepo)
{
    auto diff = repository->Diff();

    ASSERT_THROW(diff.getDiff(), std::runtime_error);
}

TEST_F(DiffTests, singleEmptyCommit)
{
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    auto diff = repository->Diff();
    auto diffFiles = diff.getDiff();

    ASSERT_EQ(diffFiles.size(), 0);
}

TEST_F(DiffTests, singleCommit)
{
    auto commits = repository->Commits();
    auto index = repository->Index();

    std::ofstream file(repositoryPath / "test.txt");
    file << "Hello, World!";
    file.close();
    index.add("test.txt");
    commits.createCommit("Initial commit");

    auto diff = repository->Diff();
    auto diffFiles = diff.getDiff();

    ASSERT_EQ(diffFiles.size(), 0); // cant compare as we dont have any previous commit, so no diff
}

TEST_F(DiffTests, twoCommitsAddedFileWithContent)
{
    auto commits = repository->Commits();
    auto index = repository->Index();

    std::ofstream file(repositoryPath / "test.txt");
    file.close();
    index.add("test.txt");
    commits.createCommit("Initial commit");

    std::ofstream file2(repositoryPath / "test2.txt");
    file2 << "Hello, World!";
    file2.close();
    index.add("test2.txt");
    commits.createCommit("Second commit");

    auto diff = repository->Diff();
    auto diffFiles = diff.getDiff();

    ASSERT_EQ(diffFiles.size(), 1);

    auto diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.isCombined, CppGit::DiffType::NORMAL);
    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile.fileA, "/dev/null");
    EXPECT_EQ(diffFile.fileB, "test2.txt");
    EXPECT_EQ(diffFile.newMode, 100'644);
    EXPECT_EQ(diffFile.hunkRangeAfter.first, 1);
    EXPECT_EQ(diffFile.hunkRangeAfter.second, -1);

    // There maybe "\\ No newline at end of file", so better look for added line
    bool foundAddedLine = false;
    for (const auto& line : diffFile.hunkContent)
    {
        if (line == "+Hello, World!")
        {
            foundAddedLine = true;
        }
    }
    EXPECT_TRUE(foundAddedLine);
}

TEST_F(DiffTests, twoCommitsAddedFileWithoutContent)
{
    auto commits = repository->Commits();
    auto index = repository->Index();

    std::ofstream file(repositoryPath / "test.txt");
    file.close();
    index.add("test.txt");
    commits.createCommit("Initial commit");

    std::ofstream file2(repositoryPath / "test2.txt");
    file2.close();
    index.add("test2.txt");
    commits.createCommit("Second commit");

    auto diff = repository->Diff();
    auto diffFiles = diff.getDiff();

    ASSERT_EQ(diffFiles.size(), 1);

    auto diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.isCombined, CppGit::DiffType::NORMAL);
    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile.fileA, "/dev/null");
    EXPECT_EQ(diffFile.fileB, "test2.txt");
    EXPECT_EQ(diffFile.newMode, 100'644);
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 0);
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}

TEST_F(DiffTests, fileModdified)
{
    auto commits = repository->Commits();
    auto index = repository->Index();

    std::ofstream file(repositoryPath / "test.txt");
    file << "Hello, World!";
    file.close();
    index.add("test.txt");
    commits.createCommit("Initial commit");

    std::ofstream file2(repositoryPath / "test.txt");
    file2 << "Hello, World! Modified";
    file2.close();
    index.add("test.txt");
    commits.createCommit("Second commit");

    auto diff = repository->Diff();
    auto diffFiles = diff.getDiff();

    ASSERT_EQ(diffFiles.size(), 1);

    auto diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.isCombined, CppGit::DiffType::NORMAL);
    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::MODDIFIED);
    EXPECT_EQ(diffFile.fileA, "test.txt");
    EXPECT_EQ(diffFile.fileB, "test.txt");
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 1);
    EXPECT_EQ(diffFile.hunkRangesBefore[0].first, 1);
    EXPECT_EQ(diffFile.hunkRangeAfter.first, 1);

    bool foundAddedLine = false;
    bool foundRemovedLine = false;
    for (const auto& line : diffFile.hunkContent)
    {
        if (line == "+Hello, World! Modified")
        {
            foundAddedLine = true;
        }
        else if (line == "-Hello, World!")
        {
            foundRemovedLine = true;
        }
    }
    EXPECT_TRUE(foundAddedLine);
    EXPECT_TRUE(foundRemovedLine);
}

TEST_F(DiffTests, multipleFile)
{
    auto commits = repository->Commits();
    auto index = repository->Index();

    std::ofstream file1(repositoryPath / "test1.txt");
    file1.close();
    index.add("test1.txt");
    commits.createCommit("Initial commit");

    std::ofstream file1Modified(repositoryPath / "test1.txt");
    file1Modified << "Hello, World!";
    file1Modified.close();
    index.add("test1.txt");

    std::ofstream file2(repositoryPath / "test2.txt");
    file2.close();
    index.add("test2.txt");

    commits.createCommit("Second commit");

    auto diff = repository->Diff();
    auto diffFiles = diff.getDiff();

    ASSERT_EQ(diffFiles.size(), 2);

    const CppGit::DiffFile* test1DiffFile = nullptr;
    const CppGit::DiffFile* test2DiffFile = nullptr;

    for (const auto& diffFile : diffFiles)
    {
        if (diffFile.fileB == "test1.txt")
        {
            test1DiffFile = &diffFile;
        }
        else if (diffFile.fileB == "test2.txt")
        {
            test2DiffFile = &diffFile;
        }
    }
    ASSERT_NE(test1DiffFile, nullptr);
    ASSERT_NE(test2DiffFile, nullptr);

    EXPECT_EQ(test1DiffFile->isCombined, CppGit::DiffType::NORMAL);
    EXPECT_EQ(test1DiffFile->diffStatus, CppGit::DiffStatus::MODDIFIED);
    EXPECT_EQ(test1DiffFile->fileA, "test1.txt");
    EXPECT_EQ(test1DiffFile->fileB, "test1.txt");
    ASSERT_EQ(test1DiffFile->hunkRangesBefore.size(), 1);
    EXPECT_EQ(test1DiffFile->hunkRangesBefore[0].first, 0);
    EXPECT_EQ(test1DiffFile->hunkRangeAfter.first, 1);

    bool foundAddedLine = false;
    for (const auto& line : test1DiffFile->hunkContent)
    {
        if (line == "+Hello, World!")
        {
            foundAddedLine = true;
        }
    }
    EXPECT_TRUE(foundAddedLine);

    EXPECT_EQ(test2DiffFile->isCombined, CppGit::DiffType::NORMAL);
    EXPECT_EQ(test2DiffFile->diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(test2DiffFile->fileA, "/dev/null");
    EXPECT_EQ(test2DiffFile->fileB, "test2.txt");
    EXPECT_EQ(test2DiffFile->newMode, 100'644);
    ASSERT_EQ(test2DiffFile->hunkRangesBefore.size(), 0);
    ASSERT_EQ(test2DiffFile->hunkContent.size(), 0);
}

TEST_F(DiffTests, getGivenFileDiff)
{
    auto commits = repository->Commits();
    auto index = repository->Index();

    std::ofstream file1(repositoryPath / "test1.txt");
    file1.close();
    index.add("test1.txt");
    commits.createCommit("Initial commit");

    std::ofstream file1Modified(repositoryPath / "test1.txt");
    file1Modified << "Hello, World!";
    file1Modified.close();
    index.add("test1.txt");

    std::ofstream file2(repositoryPath / "test2.txt");
    file2.close();
    index.add("test2.txt");

    commits.createCommit("Second commit");

    auto diff = repository->Diff();
    auto diffFiles = diff.getDiffFile("test2.txt");

    ASSERT_EQ(diffFiles.size(), 1);

    auto diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.isCombined, CppGit::DiffType::NORMAL);
    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile.fileA, "/dev/null");
    EXPECT_EQ(diffFile.fileB, "test2.txt");
    EXPECT_EQ(diffFile.newMode, 100'644);
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 0);
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}


TEST_F(DiffTests, getGivenFileDiffNoFile)
{
    auto commits = repository->Commits();
    auto index = repository->Index();

    std::ofstream file(repositoryPath / "test.txt");
    file.close();
    index.add("test.txt");
    commits.createCommit("Initial commit");

    std::ofstream file2(repositoryPath / "test.txt");
    file2 << "Hello, World! Modified";
    file2.close();
    index.add("test.txt");
    commits.createCommit("Second commit");

    auto diff = repository->Diff();
    auto diffFiles = diff.getDiffFile("test2.txt");

    ASSERT_EQ(diffFiles.size(), 0);
}

TEST_F(DiffTests, givenCommitDiff)
{
    auto commits = repository->Commits();
    auto index = repository->Index();

    std::ofstream file1(repositoryPath / "test1.txt");
    file1.close();
    index.add("test1.txt");
    commits.createCommit("Initial commit");

    std::ofstream file2(repositoryPath / "test2.txt");
    file2.close();
    index.add("test2.txt");
    auto secondCommitHash = commits.createCommit("Second commit");

    std::ofstream file3(repositoryPath / "test3.txt");
    file3.close();
    index.add("test3.txt");
    commits.createCommit("Third commit");

    auto diff = repository->Diff();
    auto diffFiles = diff.getDiff(secondCommitHash);

    ASSERT_EQ(diffFiles.size(), 1);

    auto diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.isCombined, CppGit::DiffType::NORMAL);
    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile.fileA, "/dev/null");
    EXPECT_EQ(diffFile.fileB, "test2.txt");
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}

TEST_F(DiffTests, givenCommitDiffRelativeWithTilde)
{
    auto commits = repository->Commits();
    auto index = repository->Index();

    std::ofstream file1(repositoryPath / "test1.txt");
    file1.close();
    index.add("test1.txt");
    commits.createCommit("Initial commit");

    std::ofstream file2(repositoryPath / "test2.txt");
    file2.close();
    index.add("test2.txt");
    commits.createCommit("Second commit");

    std::ofstream file3(repositoryPath / "test3.txt");
    file3.close();
    index.add("test3.txt");
    commits.createCommit("Third commit");

    auto diff = repository->Diff();
    auto diffFiles = diff.getDiff("HEAD~1");

    ASSERT_EQ(diffFiles.size(), 1);

    auto diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.isCombined, CppGit::DiffType::NORMAL);
    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile.fileA, "/dev/null");
    EXPECT_EQ(diffFile.fileB, "test2.txt");
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}

TEST_F(DiffTests, givenCommitDiffRelativeWithCaret)
{
    auto commits = repository->Commits();
    auto index = repository->Index();

    std::ofstream file1(repositoryPath / "test1.txt");
    file1.close();
    index.add("test1.txt");
    commits.createCommit("Initial commit");

    std::ofstream file2(repositoryPath / "test2.txt");
    file2.close();
    index.add("test2.txt");
    commits.createCommit("Second commit");

    std::ofstream file3(repositoryPath / "test3.txt");
    file3.close();
    index.add("test3.txt");
    commits.createCommit("Third commit");

    auto diff = repository->Diff();
    auto diffFiles = diff.getDiff("HEAD^");

    ASSERT_EQ(diffFiles.size(), 1);

    auto diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.isCombined, CppGit::DiffType::NORMAL);
    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile.fileA, "/dev/null");
    EXPECT_EQ(diffFile.fileB, "test2.txt");
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}

TEST_F(DiffTests, diffBetweenTwoCommits)
{
    auto commits = repository->Commits();
    auto index = repository->Index();

    std::ofstream file1(repositoryPath / "test1.txt");
    file1.close();
    index.add("test1.txt");
    auto firstCommithash = commits.createCommit("Initial commit");

    std::ofstream file2(repositoryPath / "test2.txt");
    file2.close();
    index.add("test2.txt");
    commits.createCommit("Second commit");

    std::ofstream file1Modified(repositoryPath / "test1.txt");
    file1Modified << "Hello, World!";
    file1Modified.close();
    index.add("test1.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    auto diff = repository->Diff();
    auto diffFiles = diff.getDiff(firstCommithash, thirdCommitHash);

    ASSERT_EQ(diffFiles.size(), 2);

    const CppGit::DiffFile* test1DiffFile = nullptr;
    const CppGit::DiffFile* test2DiffFile = nullptr;

    for (const auto& diffFile : diffFiles)
    {
        if (diffFile.fileB == "test1.txt")
        {
            test1DiffFile = &diffFile;
        }
        else if (diffFile.fileB == "test2.txt")
        {
            test2DiffFile = &diffFile;
        }
    }
    ASSERT_NE(test1DiffFile, nullptr);
    ASSERT_NE(test2DiffFile, nullptr);

    EXPECT_EQ(test1DiffFile->isCombined, CppGit::DiffType::NORMAL);
    EXPECT_EQ(test1DiffFile->diffStatus, CppGit::DiffStatus::MODDIFIED);
    EXPECT_EQ(test1DiffFile->fileA, "test1.txt");
    EXPECT_EQ(test1DiffFile->fileB, "test1.txt");
    ASSERT_EQ(test1DiffFile->hunkRangesBefore.size(), 1);
    EXPECT_EQ(test1DiffFile->hunkRangesBefore[0].first, 0);
    EXPECT_EQ(test1DiffFile->hunkRangeAfter.first, 1);

    bool foundAddedLine = false;
    for (const auto& line : test1DiffFile->hunkContent)
    {
        if (line == "+Hello, World!")
        {
            foundAddedLine = true;
        }
    }
    EXPECT_TRUE(foundAddedLine);

    EXPECT_EQ(test2DiffFile->isCombined, CppGit::DiffType::NORMAL);
    EXPECT_EQ(test2DiffFile->diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(test2DiffFile->fileA, "/dev/null");
    EXPECT_EQ(test2DiffFile->fileB, "test2.txt");
    EXPECT_EQ(test2DiffFile->newMode, 100'644);
    ASSERT_EQ(test2DiffFile->hunkRangesBefore.size(), 0);
    ASSERT_EQ(test2DiffFile->hunkContent.size(), 0);
}

TEST_F(DiffTests, diffBetweenTwoCommitsGivenFile)
{
    auto commits = repository->Commits();
    auto index = repository->Index();

    std::ofstream file1(repositoryPath / "test1.txt");
    file1.close();
    index.add("test1.txt");
    auto firstCommithash = commits.createCommit("Initial commit");

    std::ofstream file2(repositoryPath / "test2.txt");
    file2.close();
    index.add("test2.txt");
    commits.createCommit("Second commit");

    std::ofstream file1Modified(repositoryPath / "test1.txt");
    file1Modified << "Hello, World!";
    file1Modified.close();
    index.add("test1.txt");
    auto thirdCommitHash = commits.createCommit("Third commit");

    auto diff = repository->Diff();
    auto diffFiles = diff.getDiffFile(firstCommithash, thirdCommitHash, std::filesystem::path{ "test2.txt" });

    ASSERT_EQ(diffFiles.size(), 1);

    auto diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.isCombined, CppGit::DiffType::NORMAL);
    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile.fileA, "/dev/null");
    EXPECT_EQ(diffFile.fileB, "test2.txt");
    EXPECT_EQ(diffFile.newMode, 100'644);
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 0);
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}
