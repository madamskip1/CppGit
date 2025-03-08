#include "BaseRepositoryFixture.hpp"
#include "CppGit/Commits.hpp"
#include "CppGit/Diff.hpp"
#include "CppGit/Index.hpp"
#include "CppGit/_details/FileUtility.hpp"

#include <filesystem>
#include <gtest/gtest.h>

class DiffTests : public BaseRepositoryFixture
{ };

TEST_F(DiffTests, emptyRepo)
{
    const auto diff = repository->Diff();

    ASSERT_THROW(diff.getDiff(), std::runtime_error);
}

TEST_F(DiffTests, singleEmptyCommit)
{
    const auto commits = repository->Commits();
    const auto diff = repository->Diff();


    commits.createCommit("Initial commit");


    const auto diffFiles = diff.getDiff();
    ASSERT_EQ(diffFiles.size(), 0);
}

TEST_F(DiffTests, singleCommit)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto diff = repository->Diff();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test.txt", "Hello, World!");
    index.add("test.txt");
    commits.createCommit("Initial commit");


    const auto diffFiles = diff.getDiff();
    ASSERT_EQ(diffFiles.size(), 0); // cant compare as we dont have any previous commit, so no diff
}

TEST_F(DiffTests, twoCommitsAddedFileWithContent)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto diff = repository->Diff();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test.txt", "");
    commits.createCommit("Initial commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test2.txt", "Hello, World!");
    index.add("test2.txt");
    commits.createCommit("Second commit");


    const auto diffFiles = diff.getDiff();
    ASSERT_EQ(diffFiles.size(), 1);
    const auto& diffFile = diffFiles[0];
    EXPECT_EQ(diffFile.isCombined, false);
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
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto diff = repository->Diff();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test.txt", "");
    index.add("test.txt");
    commits.createCommit("Initial commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test2.txt", "");
    index.add("test2.txt");
    commits.createCommit("Second commit");


    const auto diffFiles = diff.getDiff();
    ASSERT_EQ(diffFiles.size(), 1);
    const auto& diffFile = diffFiles[0];
    EXPECT_EQ(diffFile.isCombined, false);
    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile.fileA, "/dev/null");
    EXPECT_EQ(diffFile.fileB, "test2.txt");
    EXPECT_EQ(diffFile.newMode, 100'644);
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 0);
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}

TEST_F(DiffTests, fileModdified)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto diff = repository->Diff();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test.txt", "Hello, World!");
    index.add("test.txt");
    commits.createCommit("Initial commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test.txt", "Hello, World! Modified");
    index.add("test.txt");
    commits.createCommit("Second commit");


    auto diffFiles = diff.getDiff();
    ASSERT_EQ(diffFiles.size(), 1);
    const auto& diffFile = diffFiles[0];
    EXPECT_EQ(diffFile.isCombined, false);
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
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto diff = repository->Diff();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test1.txt", "");
    index.add("test1.txt");
    commits.createCommit("Initial commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test1.txt", "Hello, World!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test2.txt", "");
    index.add("test1.txt");
    index.add("test2.txt");
    commits.createCommit("Second commit");


    const auto diffFiles = diff.getDiff();
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

    EXPECT_EQ(test1DiffFile->isCombined, false);
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

    EXPECT_EQ(test2DiffFile->isCombined, false);
    EXPECT_EQ(test2DiffFile->diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(test2DiffFile->fileA, "/dev/null");
    EXPECT_EQ(test2DiffFile->fileB, "test2.txt");
    EXPECT_EQ(test2DiffFile->newMode, 100'644);
    ASSERT_EQ(test2DiffFile->hunkRangesBefore.size(), 0);
    ASSERT_EQ(test2DiffFile->hunkContent.size(), 0);
}

TEST_F(DiffTests, getGivenFileDiff)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto diff = repository->Diff();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test1.txt", "");
    index.add("test1.txt");
    commits.createCommit("Initial commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test1.txt", "Hello, World!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test2.txt", "");
    index.add("test1.txt");
    index.add("test2.txt");
    commits.createCommit("Second commit");


    const auto diffFiles = diff.getDiffFile("test2.txt");
    ASSERT_EQ(diffFiles.size(), 1);
    const auto& diffFile = diffFiles[0];
    EXPECT_EQ(diffFile.isCombined, false);
    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile.fileA, "/dev/null");
    EXPECT_EQ(diffFile.fileB, "test2.txt");
    EXPECT_EQ(diffFile.newMode, 100'644);
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 0);
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}


TEST_F(DiffTests, getGivenFileDiffNoFile)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto diff = repository->Diff();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test.txt", "");
    index.add("test.txt");
    commits.createCommit("Initial commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test.txt", "Hello, World! Modified");
    index.add("test.txt");
    commits.createCommit("Second commit");


    const auto diffFiles = diff.getDiffFile("test2.txt");
    ASSERT_EQ(diffFiles.size(), 0);
}

TEST_F(DiffTests, givenCommitDiff)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto diff = repository->Diff();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test1.txt", "");
    index.add("test1.txt");
    commits.createCommit("Initial commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test2.txt", "");
    index.add("test2.txt");
    const auto secondCommitHash = commits.createCommit("Second commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test3.txt", "");
    index.add("test3.txt");
    commits.createCommit("Third commit");


    const auto diffFiles = diff.getDiff(secondCommitHash);
    ASSERT_EQ(diffFiles.size(), 1);
    const auto& diffFile = diffFiles[0];
    EXPECT_EQ(diffFile.isCombined, false);
    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile.fileA, "/dev/null");
    EXPECT_EQ(diffFile.fileB, "test2.txt");
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}

TEST_F(DiffTests, givenCommitDiffRelativeWithTilde)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto diff = repository->Diff();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test1.txt", "");
    index.add("test1.txt");
    commits.createCommit("Initial commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test2.txt", "");
    index.add("test2.txt");
    commits.createCommit("Second commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test3.txt", "");
    index.add("test3.txt");
    commits.createCommit("Third commit");


    const auto diffFiles = diff.getDiff("HEAD~1");
    ASSERT_EQ(diffFiles.size(), 1);
    const auto& diffFile = diffFiles[0];
    EXPECT_EQ(diffFile.isCombined, false);
    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile.fileA, "/dev/null");
    EXPECT_EQ(diffFile.fileB, "test2.txt");
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}

TEST_F(DiffTests, givenCommitDiffRelativeWithCaret)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto diff = repository->Diff();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test1.txt", "");
    index.add("test1.txt");
    commits.createCommit("Initial commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test2.txt", "");
    index.add("test2.txt");
    commits.createCommit("Second commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test3.txt", "");
    index.add("test3.txt");
    commits.createCommit("Third commit");


    const auto diffFiles = diff.getDiff("HEAD^");
    ASSERT_EQ(diffFiles.size(), 1);
    const auto& diffFile = diffFiles[0];
    EXPECT_EQ(diffFile.isCombined, false);
    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile.fileA, "/dev/null");
    EXPECT_EQ(diffFile.fileB, "test2.txt");
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}

TEST_F(DiffTests, diffBetweenTwoCommits)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto diff = repository->Diff();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test1.txt", "");
    index.add("test1.txt");
    const auto initialCommitHash = commits.createCommit("Initial commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test2.txt", "");
    index.add("test2.txt");
    commits.createCommit("Second commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test1.txt", "Hello, World!");
    index.add("test1.txt");
    const auto thirdCommitHash = commits.createCommit("Third commit");


    const auto diffFiles = diff.getDiff(initialCommitHash, thirdCommitHash);
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

    EXPECT_EQ(test1DiffFile->isCombined, false);
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

    EXPECT_EQ(test2DiffFile->isCombined, false);
    EXPECT_EQ(test2DiffFile->diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(test2DiffFile->fileA, "/dev/null");
    EXPECT_EQ(test2DiffFile->fileB, "test2.txt");
    EXPECT_EQ(test2DiffFile->newMode, 100'644);
    ASSERT_EQ(test2DiffFile->hunkRangesBefore.size(), 0);
    ASSERT_EQ(test2DiffFile->hunkContent.size(), 0);
}

TEST_F(DiffTests, diffBetweenTwoCommitsGivenFile)
{
    const auto commits = repository->Commits();
    const auto index = repository->Index();
    const auto diff = repository->Diff();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test1.txt", "");
    index.add("test1.txt");
    const auto initialCommitHash = commits.createCommit("Initial commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test2.txt", "");
    index.add("test2.txt");
    commits.createCommit("Second commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "test1.txt", "Hello, World!");
    index.add("test1.txt");
    const auto thirdCommitHash = commits.createCommit("Third commit");


    const auto diffFiles = diff.getDiffFile(initialCommitHash, thirdCommitHash, std::filesystem::path{ "test2.txt" });
    ASSERT_EQ(diffFiles.size(), 1);
    const auto& diffFile = diffFiles[0];
    EXPECT_EQ(diffFile.isCombined, false);
    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile.fileA, "/dev/null");
    EXPECT_EQ(diffFile.fileB, "test2.txt");
    EXPECT_EQ(diffFile.newMode, 100'644);
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 0);
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}
