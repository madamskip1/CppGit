#include "BaseRepositoryFixture.hpp"
#include "Commits.hpp"
#include "Index.hpp"

#include <fstream>
#include <gtest/gtest.h>
#include <vector>

class IndexTests : public BaseRepositoryFixture
{
};

// TEST_F(IndexTests, addAndRemoveRegularFile)
// {
//     auto index = repository->Index();
//     std::ofstream file(repositoryPath / "file.txt");
//     file << "Hello, World!";
//     file.close();
//     index.add(repositoryPath / "file.txt");

//     auto stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 1);
//     ASSERT_EQ(stagedFiles[0], "file.txt");

//     index.remove(repositoryPath / "file.txt");
//     stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 0);
// }

// TEST_F(IndexTests, addAndRemoveRegularFile_StagedListWithDetail)
// {
//     auto index = repository->Index();
//     std::ofstream file(repositoryPath / "file.txt");
//     file << "Hello, World!";
//     file.close();
//     index.add(repositoryPath / "file.txt");

//     auto stagedFiles = index.getFilesInIndexListWithDetails();
//     ASSERT_EQ(stagedFiles.size(), 1);
//     ASSERT_EQ(stagedFiles[0].fileMode, "100644");
//     ASSERT_EQ(stagedFiles[0].path, "file.txt");

//     index.remove(repositoryPath / "file.txt");
//     stagedFiles = index.getFilesInIndexListWithDetails();
//     ASSERT_EQ(stagedFiles.size(), 0);
// }

// TEST_F(IndexTests, addRegularFile_FileDoesNotExist)
// {
//     auto index = repository->Index();
//     ASSERT_THROW(index.add(repositoryPath / "file.txt"), std::runtime_error);
// }

// TEST_F(IndexTests, addAndRemoveExecutableFile)
// {
//     auto index = repository->Index();
//     std::ofstream file(repositoryPath / "file.sh");
//     file << "echo Hello, World!";
//     file.close();
//     std::filesystem::permissions(repositoryPath / "file.sh", std::filesystem::perms::owner_exec | std::filesystem::perms::owner_read, std::filesystem::perm_options::add);
//     index.add(repositoryPath / "file.sh");

//     auto stagedFiles = index.getFilesInIndexListWithDetails();
//     ASSERT_EQ(stagedFiles.size(), 1);
//     ASSERT_EQ(stagedFiles[0].fileMode, "100755");
//     ASSERT_EQ(stagedFiles[0].path, "file.sh");

//     index.remove(repositoryPath / "file.sh");
//     stagedFiles = index.getFilesInIndexListWithDetails();
//     ASSERT_EQ(stagedFiles.size(), 0);
// }

// TEST_F(IndexTests, addAndRemoveSymlink)
// {
//     auto index = repository->Index();
//     std::ofstream file(repositoryPath / "file.txt");
//     file << "Hello, World!";
//     file.close();
//     std::filesystem::create_symlink(repositoryPath / "file.txt", repositoryPath / "file-symlink.txt");
//     index.add(repositoryPath / "file-symlink.txt");

//     auto stagedFiles = index.getFilesInIndexListWithDetails();
//     ASSERT_EQ(stagedFiles.size(), 1);
//     ASSERT_EQ(stagedFiles[0].fileMode, "120000");
//     ASSERT_EQ(stagedFiles[0].path, "file-symlink.txt");

//     index.remove(repositoryPath / "file-symlink.txt");
//     stagedFiles = index.getFilesInIndexListWithDetails();
//     ASSERT_EQ(stagedFiles.size(), 0);
// }

// TEST_F(IndexTests, addAndRemoveRegularFileInDir_DirectPath)
// {
//     auto index = repository->Index();
//     std::filesystem::create_directory(repositoryPath / "dir");
//     std::ofstream file(repositoryPath / "dir" / "file.txt");
//     file << "Hello, World!";
//     file.close();
//     index.add(repositoryPath / "dir" / "file.txt");

//     auto stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 1);
//     ASSERT_EQ(stagedFiles[0], "dir/file.txt");

//     index.remove(repositoryPath / "dir" / "file.txt");
//     stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 0);
// }

// TEST_F(IndexTests, addAndRemoveRegularFileInDir_DirectoryPath)
// {
//     auto index = repository->Index();
//     std::filesystem::create_directory(repositoryPath / "dir");
//     std::ofstream file(repositoryPath / "dir" / "file.txt");
//     file << "Hello, World!";
//     file.close();
//     index.add(repositoryPath / "dir");

//     auto stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 1);
//     ASSERT_EQ(stagedFiles[0], "dir/file.txt");

//     index.remove(repositoryPath / "dir");
//     stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 0);
// }

// TEST_F(IndexTests, addAndRemoveReuglarFileInDir_DirectoryRelativePath)
// {
//     auto index = repository->Index();
//     std::filesystem::create_directory(repositoryPath / "dir");
//     std::ofstream file(repositoryPath / "dir" / "file.txt");
//     file << "Hello, World!";
//     file.close();
//     index.add("dir");

//     auto stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 1);
//     ASSERT_EQ(stagedFiles[0], "dir/file.txt");

//     index.remove("dir");
//     stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 0);
// }


// TEST_F(IndexTests, addAndRemoveRegularFileInDir_RecursivePath)
// {
//     auto index = repository->Index();
//     std::filesystem::create_directories(repositoryPath / "dir1" / "dir2");
//     std::ofstream file1(repositoryPath / "dir1" / "file1.txt");
//     file1 << "Hello, World!";
//     file1.close();
//     std::ofstream file2(repositoryPath / "dir1" / "dir2" / "file2.txt");
//     file2 << "Hello, World!";
//     file2.close();
//     index.add(repositoryPath / "dir1");

//     auto stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 2);
//     ASSERT_TRUE(std::find(stagedFiles.begin(), stagedFiles.end(), "dir1/dir2/file2.txt") != stagedFiles.end());
//     ASSERT_TRUE(std::find(stagedFiles.begin(), stagedFiles.end(), "dir1/file1.txt") != stagedFiles.end());

//     index.remove(repositoryPath / "dir1");
//     stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 0);
// }

// TEST_F(IndexTests, addAndRemoveRegularFile_RelativePath)
// {
//     auto index = repository->Index();
//     std::ofstream file(repositoryPath / "file.txt");
//     file << "Hello, World!";
//     file.close();
//     index.add("file.txt");

//     auto stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 1);
//     ASSERT_EQ(stagedFiles[0], "file.txt");

//     index.remove("file.txt");
//     stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 0);
// }

// TEST_F(IndexTests, addAndRemoveRegularFile_RelativePathInDir)
// {
//     auto index = repository->Index();
//     std::filesystem::create_directory(repositoryPath / "dir");
//     std::ofstream file(repositoryPath / "dir" / "file.txt");
//     file << "Hello, World!";
//     file.close();
//     index.add("dir/file.txt");

//     auto stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 1);
//     ASSERT_EQ(stagedFiles[0], "dir/file.txt");

//     index.remove("dir/file.txt");
//     stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 0);
// }

// TEST_F(IndexTests, addAndRemoveAll_RepositoryPath)
// {
//     auto index = repository->Index();
//     std::filesystem::create_directory(repositoryPath / "dir");
//     std::ofstream file(repositoryPath / "dir" / "file.txt");
//     file << "Hello, World!";
//     file.close();
//     index.add(repositoryPath);

//     auto stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 1);
//     ASSERT_EQ(stagedFiles[0], "dir/file.txt");

//     index.remove(repositoryPath);
//     stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 0);
// }

// TEST_F(IndexTests, addAndRemoveAllDotPath)
// {
//     auto index = repository->Index();
//     std::filesystem::create_directory(repositoryPath / "dir");
//     std::ofstream file(repositoryPath / "dir" / "file.txt");
//     file << "Hello, World!";
//     file.close();
//     index.add(".");

//     auto stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 1);
//     ASSERT_EQ(stagedFiles[0], "dir/file.txt");

//     index.remove(".");
//     stagedFiles = index.getFilesInIndexList();
//     ASSERT_EQ(stagedFiles.size(), 0);
// }

// TEST_F(IndexTests, tryAddFileFromGitDirectory_absolutePath)
// {
//     auto index = repository->Index();
//     std::ofstream file(repositoryPath / ".git" / "file.txt");
//     file << "Hello, World!";
//     file.close();

//     ASSERT_THROW(index.add(repositoryPath / ".git" / "file.txt"), std::runtime_error);
// }

// TEST_F(IndexTests, tryAddFileFromGitDirectory_repoPath)
// {
//     auto index = repository->Index();
//     std::ofstream file(repositoryPath / ".git" / "file.txt");
//     file << "Hello, World!";
//     file.close();

//     ASSERT_THROW(index.add(".git/file.txt"), std::runtime_error);
// }

TEST_F(IndexTests, restoreAllStaged_noChanges)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Initial commit");

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);

    index.restoreAllStaged();

    stagedFiles = index.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, restoreAllStaged_notStagedChanges)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Initial commit");

    file.open(repositoryPath / "file.txt");
    file << "Hello, World! Changed";
    file.close();

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);

    index.restoreAllStaged();

    stagedFiles = index.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, restoreAllStaged_stagedNewFile)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);

    index.restoreAllStaged();

    stagedFiles = index.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, restoreAllStaged_stagedChanges)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Initial commit");

    file.open(repositoryPath / "file.txt");
    file << "Hello, World! Changed";
    file.close();
    index.add("file.txt");

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);

    index.restoreAllStaged();

    stagedFiles = index.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, restoreAllStaged_stagedChangesInDir)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    std::filesystem::create_directory(repositoryPath / "dir");
    std::ofstream file(repositoryPath / "dir" / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("dir/file.txt");

    commits.createCommit("Initial commit");

    file.open(repositoryPath / "dir" / "file.txt");
    file << "Hello, World! Changed";
    file.close();
    index.add("dir/file.txt");

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);

    index.restoreAllStaged();

    stagedFiles = index.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, restoreAllStaged_multipleFile)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    std::ofstream file1(repositoryPath / "file1.txt");
    file1 << "Hello, World!";
    file1.close();
    index.add("file1.txt");

    commits.createCommit("Initial commit");

    file1.open(repositoryPath / "file1.txt");
    file1 << "Hello, World! Changed";
    file1.close();
    index.add("file1.txt");

    std::ofstream file2(repositoryPath / "file2.txt");
    file2 << "Hello, World!";
    file2.close();
    index.add("file2.txt");

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 2);

    index.restoreAllStaged();

    stagedFiles = index.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, restoreAllStaged_multipleFile_notAllStaged)
{
    auto index = repository->Index();
    auto commits = repository->Commits();
    commits.createCommit("Initial commit");

    std::ofstream file1(repositoryPath / "file1.txt");
    file1 << "Hello, World!";
    file1.close();
    index.add("file1.txt");

    std::ofstream file2(repositoryPath / "file2.txt");
    file2 << "Hello, World!";
    file2.close();

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);

    index.restoreAllStaged();

    stagedFiles = index.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}


TEST_F(IndexTests, notDirty_emptyRepo)
{
    auto index = repository->Index();

    EXPECT_THROW(index.isDirty(), std::runtime_error);
}

TEST_F(IndexTests, dirty_emptyRepo)
{
    auto index = repository->Index();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();

    EXPECT_THROW(index.isDirty(), std::runtime_error);
}

TEST_F(IndexTests, dirty_emptyRepo_FileAddedToIndex)
{
    auto index = repository->Index();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");

    EXPECT_THROW(index.isDirty(), std::runtime_error);
}

TEST_F(IndexTests, notDirty)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Initial commit");

    EXPECT_FALSE(index.isDirty());
}


TEST_F(IndexTests, dirty_trackedFile)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Initial commit");

    file.open(repositoryPath / "file.txt");
    file << "Hello, World! Changed";
    file.close();

    EXPECT_TRUE(index.isDirty());
}

TEST_F(IndexTests, dirty_cachedTrackedFile)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Initial commit");

    file.open(repositoryPath / "file.txt");
    file << "Hello, World! Changed";
    file.close();
    index.add("file.txt");

    EXPECT_TRUE(index.isDirty());
}

TEST_F(IndexTests, notdirty_untrackedFile)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Initial commit");

    std::ofstream utrackedFile(repositoryPath / "file2.txt");
    utrackedFile << "Hello, World!";
    utrackedFile.close();

    EXPECT_FALSE(index.isDirty());
}

TEST_F(IndexTests, dirty_untrackedFileAddedToIndex)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Initial commit");

    std::ofstream utrackedFile(repositoryPath / "file2.txt");
    utrackedFile << "Hello, World!";
    utrackedFile.close();
    index.add("file2.txt");

    EXPECT_TRUE(index.isDirty());
}

TEST_F(IndexTests, getUntrackedFileList_emptyRepo)
{
    auto index = repository->Index();

    auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getUntrackedFileList_notStagedFile)
{
    auto index = repository->Index();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();

    auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 1);
    ASSERT_EQ(untrackedFiles[0], "file.txt");
}

TEST_F(IndexTests, getUntrackedFileList_stagedFile)
{
    auto index = repository->Index();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");

    auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getUntrackedFileList_untrackedFileInDir)
{
    auto index = repository->Index();

    std::filesystem::create_directory(repositoryPath / "dir");
    std::ofstream file(repositoryPath / "dir" / "file.txt");
    file << "Hello, World!";
    file.close();

    auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 1);
    ASSERT_EQ(untrackedFiles[0], "dir/file.txt");
}

TEST_F(IndexTests, getUntrackedFileList_untrackedFileInDirStaged)
{
    auto index = repository->Index();

    std::filesystem::create_directory(repositoryPath / "dir");
    std::ofstream file(repositoryPath / "dir" / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("dir/file.txt");

    auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getUntrackedFileList_trackedModified)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Initial commit");

    file.open(repositoryPath / "file.txt");
    file << "Hello, World! Changed";
    file.close();

    auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getUntrackedFileList_trackedDeleted)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Initial commit");

    std::filesystem::remove(repositoryPath / "file.txt");

    auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getStagedFilesList_emptyRepo)
{
    auto index = repository->Index();

    EXPECT_THROW(index.getStagedFilesListWithStatus(), std::runtime_error);
}


TEST_F(IndexTests, getStagedFilesList_notStagedFile_notCommitYet)
{
    auto index = repository->Index();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();

    EXPECT_THROW(index.getStagedFilesListWithStatus(), std::runtime_error);
}

TEST_F(IndexTests, getStagedFilesList_stagedFile_notCommitYet)
{
    auto index = repository->Index();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");

    EXPECT_THROW(index.getStagedFilesListWithStatus(), std::runtime_error);
}

TEST_F(IndexTests, getStagedFilesList_commitedFile)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Initial commit");

    auto stagedFiles = index.getStagedFilesListWithStatus();
    ASSERT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, getStagedFilesList_stagedFile)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");

    auto stagedFiles = index.getStagedFilesListWithStatus();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0].path, "file.txt");
    EXPECT_EQ(stagedFiles[0].status, CppGit::DiffIndexStatus::ADDED);
}

TEST_F(IndexTests, isFileStaged_EmptyIndex)
{
    auto index = repository->Index();
    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();

    EXPECT_THROW(index.isFileStaged("file.txt"), std::runtime_error);
}

TEST_F(IndexTests, isFileStaged_FileStaged_notCommitYet)
{
    auto index = repository->Index();
    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");

    EXPECT_THROW(index.isFileStaged("file.txt"), std::runtime_error);
}

TEST_F(IndexTests, isFileStaged_FileNotStaged_afterCommit)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();

    EXPECT_FALSE(index.isFileStaged("file.txt"));
}

TEST_F(IndexTests, isFileStaged_FileStaged_afterCommit)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");

    EXPECT_TRUE(index.isFileStaged("file.txt"));
}

TEST_F(IndexTests, isFileStaged_FileNotExist_afterCommit)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");

    EXPECT_FALSE(index.isFileStaged("file2.txt"));
}

TEST_F(IndexTests, getNotStagedFiles_emptyRepoAfterCommit)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 0);
}

TEST_F(IndexTests, getNotStagedFiles_untrackedNotStagedFile)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();

    auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 1);
    EXPECT_EQ(notStagedFiles[0], "file.txt");
}

TEST_F(IndexTests, getNotStagedFiles_untrackedStagedFile)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");

    auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 0);
}

TEST_F(IndexTests, getNotStagedFiles_trackedNotModified)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Initial commit");

    auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 0);
}

TEST_F(IndexTests, getNotStagedFiles_trackedModifiedNotStaged)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Initial commit");

    file.open(repositoryPath / "file.txt");
    file << "Hello, World! Changed";
    file.close();

    auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 1);
    EXPECT_EQ(notStagedFiles[0], "file.txt");
}

TEST_F(IndexTests, getNotStagedFiles_trackedModifiedStaged)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Initial commit");

    file.open(repositoryPath / "file.txt");
    file << "Hello, World! Changed";
    file.close();
    index.add("file.txt");

    auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 0);
}


TEST_F(IndexTests, getNotStagedFiles_trackedDeletedNotStaged)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");
    commits.createCommit("Initial commit");

    std::filesystem::remove(repositoryPath / "file.txt");

    auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 1);
    EXPECT_EQ(notStagedFiles[0], "file.txt");
}
