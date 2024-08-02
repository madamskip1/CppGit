#include "BaseRepositoryFixture.hpp"
#include "Index.hpp"

#include <fstream>
#include <gtest/gtest.h>

class IndexTests : public BaseRepositoryFixture
{
};

TEST_F(IndexTests, addAndRemoveRegularFile)
{
    CppGit::Index index(*repository);
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
    CppGit::Index index(*repository);
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
    CppGit::Index index(*repository);
    ASSERT_THROW(index.add(repositoryPath / "file.txt"), std::runtime_error);
}

TEST_F(IndexTests, addAndRemoveExecutableFile)
{
    CppGit::Index index(*repository);
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
    CppGit::Index index(*repository);
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
    CppGit::Index index(*repository);
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
    CppGit::Index index(*repository);
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
    CppGit::Index index(*repository);
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
    CppGit::Index index(*repository);
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
    CppGit::Index index(*repository);
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
    CppGit::Index index(*repository);
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
    CppGit::Index index(*repository);
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
    CppGit::Index index(*repository);
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
    CppGit::Index index(*repository);
    std::ofstream file(repositoryPath / ".git" / "file.txt");
    file << "Hello, World!";
    file.close();

    ASSERT_THROW(index.add(repositoryPath / ".git" / "file.txt"), std::runtime_error);
}

TEST_F(IndexTests, tryAddFileFromGitDirectory_repoPath)
{
    CppGit::Index index(*repository);
    std::ofstream file(repositoryPath / ".git" / "file.txt");
    file << "Hello, World!";
    file.close();

    ASSERT_THROW(index.add(".git/file.txt"), std::runtime_error);
}
