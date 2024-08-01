#include "BaseRepositoryFixture.hpp"
#include "Index.hpp"

#include <fstream>
#include <gtest/gtest.h>

class IndexTests : public BaseRepositoryFixture
{
};

TEST_F(IndexTests, addFileRegularFile)
{
    CppGit::Index index(*repository);
    std::ofstream file(repositoryPath / "file.txt");
    file << "Hello, World!";
    file.close();
    index.add(repositoryPath / "file.txt");

    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    ASSERT_EQ(stagedFiles[0], "file.txt");
}

TEST_F(IndexTests, addFileRegularFile_StagedListWithDetail)
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
}

TEST_F(IndexTests, addFileRegularFile_FileDoesNotExist)
{
    CppGit::Index index(*repository);
    ASSERT_THROW(index.add(repositoryPath / "file.txt"), std::runtime_error);
}

TEST_F(IndexTests, addFileExecutableFile)
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
}

TEST_F(IndexTests, addFileSymlink)
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
}

TEST_F(IndexTests, addRegularFileInDir_DirectPath)
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
}
