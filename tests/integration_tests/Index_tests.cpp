#include "BaseRepositoryFixture.hpp"
#include "Index.hpp"

#include <fstream>
#include <gtest/gtest.h>

class IndexTests : public BaseRepositoryFixture
{
};

TEST_F(IndexTests, addAndRemoveRegularFile)
{
    auto index = repository->Index();
    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add(repositoryPath / "file.txt");

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    ASSERT_EQ(stagedFiles[0], "file.txt");

    index.remove(repositoryPath / "file.txt");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, addAndRemoveRegularFile_StagedListWithDetail)
{
    auto index = repository->Index();
    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add(repositoryPath / "file.txt");

    auto stagedFiles = index.getStagedFilesListWithDetails();
    ASSERT_EQ(stagedFiles.size(), 1);
    ASSERT_EQ(stagedFiles[0].fileMode, "100644");
    ASSERT_EQ(stagedFiles[0].path, "file.txt");

    index.remove(repositoryPath / "file.txt");
    stagedFiles = index.getStagedFilesListWithDetails();
    ASSERT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, addRegularFile_FileDoesNotExist)
{
    auto index = repository->Index();
    ASSERT_THROW(index.add(repositoryPath / "file.txt"), std::runtime_error);
}

TEST_F(IndexTests, addAndRemoveExecutableFile)
{
    auto index = repository->Index();
    std::ofstream file(repositoryPath / "file.sh");
    file << "echo Hello, World!";
    file.close();
    std::filesystem::permissions(repositoryPath / "file.sh", std::filesystem::perms::owner_exec | std::filesystem::perms::owner_read, std::filesystem::perm_options::add);
    index.add(repositoryPath / "file.sh");

    auto stagedFiles = index.getStagedFilesListWithDetails();
    ASSERT_EQ(stagedFiles.size(), 1);
    ASSERT_EQ(stagedFiles[0].fileMode, "100755");
    ASSERT_EQ(stagedFiles[0].path, "file.sh");

    index.remove(repositoryPath / "file.sh");
    stagedFiles = index.getStagedFilesListWithDetails();
    ASSERT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, addAndRemoveSymlink)
{
    auto index = repository->Index();
    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    std::filesystem::create_symlink(repositoryPath / "file.txt", repositoryPath / "file-symlink.txt");
    index.add(repositoryPath / "file-symlink.txt");

    auto stagedFiles = index.getStagedFilesListWithDetails();
    ASSERT_EQ(stagedFiles.size(), 1);
    ASSERT_EQ(stagedFiles[0].fileMode, "120000");
    ASSERT_EQ(stagedFiles[0].path, "file-symlink.txt");

    index.remove(repositoryPath / "file-symlink.txt");
    stagedFiles = index.getStagedFilesListWithDetails();
    ASSERT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, addAndRemoveRegularFileInDir_DirectPath)
{
    auto index = repository->Index();
    std::filesystem::create_directory(repositoryPath / "dir");
    std::ofstream file(repositoryPath / "dir" / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add(repositoryPath / "dir" / "file.txt");

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    ASSERT_EQ(stagedFiles[0], "dir/file.txt");

    index.remove(repositoryPath / "dir" / "file.txt");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, addAndRemoveRegularFileInDir_DirectoryPath)
{
    auto index = repository->Index();
    std::filesystem::create_directory(repositoryPath / "dir");
    std::ofstream file(repositoryPath / "dir" / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add(repositoryPath / "dir");

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    ASSERT_EQ(stagedFiles[0], "dir/file.txt");

    index.remove(repositoryPath / "dir");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, addAndRemoveReuglarFileInDir_DirectoryRelativePath)
{
    auto index = repository->Index();
    std::filesystem::create_directory(repositoryPath / "dir");
    std::ofstream file(repositoryPath / "dir" / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("dir");

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    ASSERT_EQ(stagedFiles[0], "dir/file.txt");

    index.remove("dir");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
}


TEST_F(IndexTests, addAndRemoveRegularFileInDir_RecursivePath)
{
    auto index = repository->Index();
    std::filesystem::create_directories(repositoryPath / "dir1" / "dir2");
    std::ofstream file1(repositoryPath / "dir1" / "file1.txt");
    file1 << "Hello, World!";
    file1.close();
    std::ofstream file2(repositoryPath / "dir1" / "dir2" / "file2.txt");
    file2 << "Hello, World!";
    file2.close();
    index.add(repositoryPath / "dir1");

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 2);
    ASSERT_TRUE(std::find(stagedFiles.begin(), stagedFiles.end(), "dir1/dir2/file2.txt") != stagedFiles.end());
    ASSERT_TRUE(std::find(stagedFiles.begin(), stagedFiles.end(), "dir1/file1.txt") != stagedFiles.end());

    index.remove(repositoryPath / "dir1");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, addAndRemoveRegularFile_RelativePath)
{
    auto index = repository->Index();
    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    ASSERT_EQ(stagedFiles[0], "file.txt");

    index.remove("file.txt");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, addAndRemoveRegularFile_RelativePathInDir)
{
    auto index = repository->Index();
    std::filesystem::create_directory(repositoryPath / "dir");
    std::ofstream file(repositoryPath / "dir" / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("dir/file.txt");

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    ASSERT_EQ(stagedFiles[0], "dir/file.txt");

    index.remove("dir/file.txt");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, addAndRemoveAll_RepositoryPath)
{
    auto index = repository->Index();
    std::filesystem::create_directory(repositoryPath / "dir");
    std::ofstream file(repositoryPath / "dir" / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add(repositoryPath);

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    ASSERT_EQ(stagedFiles[0], "dir/file.txt");

    index.remove(repositoryPath);
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, addAndRemoveAllDotPath)
{
    auto index = repository->Index();
    std::filesystem::create_directory(repositoryPath / "dir");
    std::ofstream file(repositoryPath / "dir" / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add(".");

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    ASSERT_EQ(stagedFiles[0], "dir/file.txt");

    index.remove(".");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, tryAddFileFromGitDirectory_absolutePath)
{
    auto index = repository->Index();
    std::ofstream file(repositoryPath / ".git" / "file.txt");
    file << "Hello, World!";
    file.close();

    ASSERT_THROW(index.add(repositoryPath / ".git" / "file.txt"), std::runtime_error);
}

TEST_F(IndexTests, tryAddFileFromGitDirectory_repoPath)
{
    auto index = repository->Index();
    std::ofstream file(repositoryPath / ".git" / "file.txt");
    file << "Hello, World!";
    file.close();

    ASSERT_THROW(index.add(".git/file.txt"), std::runtime_error);
}

TEST_F(IndexTests, resetIndex)
{
    auto index = repository->Index();
    std::ofstream file1(repositoryPath / "file1.txt");
    file1 << "Hello, World!";
    file1.close();
    index.add("file1.txt");
    std::filesystem::create_directory(repositoryPath / "dir");
    std::ofstream file2(repositoryPath / "dir" / "file2.txt");
    file2 << "Hello, World!";
    file2.close();
    index.add("dir");

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 2);
    ASSERT_TRUE(std::find(stagedFiles.begin(), stagedFiles.end(), "dir/file2.txt") != stagedFiles.end());
    ASSERT_TRUE(std::find(stagedFiles.begin(), stagedFiles.end(), "file1.txt") != stagedFiles.end());

    index.reset();
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, isFileStaged_EmptyIndex)
{
    auto index = repository->Index();
    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();

    EXPECT_FALSE(index.isFileStaged("file.txt"));
}

TEST_F(IndexTests, isFileStaged_FileStaged)
{
    auto index = repository->Index();
    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");

    EXPECT_TRUE(index.isFileStaged("file.txt"));
}

TEST_F(IndexTests, isFileStaged_FileNotStaged)
{
    auto index = repository->Index();
    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add("file.txt");

    EXPECT_FALSE(index.isFileStaged("file2.txt"));
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
