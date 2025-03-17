#include "BaseRepositoryFixture.hpp"

#include <CppGit/CommitsManager.hpp>
#include <CppGit/IndexManager.hpp>
#include <CppGit/_details/FileUtility.hpp>
#include <gtest/gtest.h>
#include <vector>

class IndexTests : public BaseRepositoryFixture
{
};

TEST_F(IndexTests, addRegularFile)
{
    // For staged we need at least one commit
    // TODO: getStagedFileList should do ls-files if no commit

    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    auto stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    indexManager.add("file.txt");

    indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "file.txt");
    stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "file.txt");
}

TEST_F(IndexTests, addRegularFileInDir)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    auto stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    indexManager.add("dir/file.txt");


    indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");
    stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "dir/file.txt");
}

TEST_F(IndexTests, addRegularFileInDir_providedDirAsPattern)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();

    commitsManager.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    auto stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    indexManager.add("dir");


    indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");
    stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "dir/file.txt");
}

TEST_F(IndexTests, addExecutableFile)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.sh", "echo Hello, World!");
    std::filesystem::permissions(repositoryPath / "file.sh", std::filesystem::perms::owner_exec | std::filesystem::perms::owner_read, std::filesystem::perm_options::add);
    auto stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = indexManager.getFilesInIndexDetailedList();
    ASSERT_EQ(indexFiles.size(), 0);

    indexManager.add("file.sh");


    indexFiles = indexManager.getFilesInIndexDetailedList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0].fileMode, 100'755);
    EXPECT_EQ(indexFiles[0].path, "file.sh");
    stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "file.sh");
}

TEST_F(IndexTests, addSymlink)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    std::filesystem::create_symlink(repositoryPath / "file.txt", repositoryPath / "file-symlink.txt");
    auto stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = indexManager.getFilesInIndexDetailedList();
    ASSERT_EQ(indexFiles.size(), 0);

    indexManager.add("file-symlink.txt");


    indexFiles = indexManager.getFilesInIndexDetailedList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0].fileMode, 120'000);
    EXPECT_EQ(indexFiles[0].path, "file-symlink.txt");
    stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "file-symlink.txt");
}

TEST_F(IndexTests, addOnDeletedFile)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();

    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Second commit");
    auto indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "file.txt");
    std::filesystem::remove(repositoryPath / "file.txt");

    indexManager.add("file.txt");


    indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);
    const auto stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "file.txt");
}

TEST_F(IndexTests, addFilesWithAsteriskPattern)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");
    auto stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    indexManager.add("*.txt");

    indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 2);
    EXPECT_EQ(indexFiles[0], "file1.txt");
    EXPECT_EQ(indexFiles[1], "file2.txt");
    stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 2);
    EXPECT_EQ(stagedFiles[0], "file1.txt");
    EXPECT_EQ(stagedFiles[1], "file2.txt");
}

TEST_F(IndexTests, addFilesWithAsteriskPatternInDirectories)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir1");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir1" / "file.txt", "Hello, World!");
    std::filesystem::create_directory(repositoryPath / "dir2");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir2" / "file.txt", "Hello, World!");
    auto stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    indexManager.add("dir*/*.txt");


    indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 2);
    EXPECT_EQ(indexFiles[0], "dir1/file.txt");
    EXPECT_EQ(indexFiles[1], "dir2/file.txt");
    stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 2);
    EXPECT_EQ(stagedFiles[0], "dir1/file.txt");
    EXPECT_EQ(stagedFiles[1], "dir2/file.txt");
}

TEST_F(IndexTests, removeRegularFile_fileNotDeletedFromWorkinDirectory)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    auto indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "file.txt");

    indexManager.remove("file.txt");


    indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1); // file still exist in the working directory
}

TEST_F(IndexTests, removeRegularFile_fileDeletedFromWorkinDirectory)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    auto indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "file.txt");
    std::filesystem::remove(repositoryPath / "file.txt");

    indexManager.remove("file.txt");


    indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);
}

TEST_F(IndexTests, removeRegularFile_force)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    auto indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "file.txt");

    indexManager.remove("file.txt", true);


    indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);
}

TEST_F(IndexTests, removeRegularFileInDir)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    indexManager.add("dir/file.txt");
    auto indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");

    indexManager.remove("dir/file.txt");


    indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1); // file still exist in the working directory
}

TEST_F(IndexTests, removeRegularFileInDir_providedDirAsPattern)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    indexManager.add("dir/file.txt");
    auto indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");

    indexManager.remove("dir");


    indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1); // file still exist in the working directory
}

TEST_F(IndexTests, removeRegularFileInDir_force)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    indexManager.add("dir/file.txt");
    auto indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");

    indexManager.remove("dir", true);


    indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);
}

TEST_F(IndexTests, removeRegularFileInDir_removedFile)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    indexManager.add("dir/file.txt");
    auto indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");
    std::filesystem::remove(repositoryPath / "dir" / "file.txt");

    indexManager.remove("dir/file.txt");


    indexFiles = indexManager.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);
}


TEST_F(IndexTests, restoreAllStaged_noChanges)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");
    auto stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);

    indexManager.restoreAllStaged();


    stagedFiles = indexManager.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, restoreAllStaged_notStagedChanges)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    auto stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);

    indexManager.restoreAllStaged();


    stagedFiles = indexManager.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, restoreAllStaged_stagedNewFile)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    auto stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);

    indexManager.restoreAllStaged();


    stagedFiles = indexManager.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, restoreAllStaged_stagedChanges)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    indexManager.add("file.txt");
    auto stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);

    indexManager.restoreAllStaged();


    stagedFiles = indexManager.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, restoreAllStaged_stagedChangesInDir)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    indexManager.add("dir/file.txt");
    auto stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);

    indexManager.restoreAllStaged();


    stagedFiles = indexManager.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, restoreAllStaged_multipleFile)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! 1");
    indexManager.add("file1.txt");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World! 2");
    indexManager.add("file2.txt");
    auto stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 2);

    indexManager.restoreAllStaged();


    stagedFiles = indexManager.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, restoreAllStaged_multipleFile_notAllStaged)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World!");
    indexManager.add("file1.txt");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");
    auto stagedFiles = indexManager.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);

    indexManager.restoreAllStaged();


    stagedFiles = indexManager.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, notDirty)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");


    EXPECT_FALSE(indexManager.isDirty());
}


TEST_F(IndexTests, dirty_changesInCachedNotAdded)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");


    EXPECT_TRUE(indexManager.isDirty());
}

TEST_F(IndexTests, dirty_changesInCachedAdded)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    indexManager.add("file.txt");


    EXPECT_TRUE(indexManager.isDirty());
}

TEST_F(IndexTests, notdirty_untrackedFile)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");


    EXPECT_FALSE(indexManager.isDirty());
}

TEST_F(IndexTests, dirty_untrackedFileAdded)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");
    indexManager.add("file2.txt");


    EXPECT_TRUE(indexManager.isDirty());
}

TEST_F(IndexTests, getUntrackedFileList_emptyRepo)
{
    const auto indexManager = repository->IndexManager();

    const auto untrackedFiles = indexManager.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getUntrackedFileList_notStagedFile)
{
    const auto indexManager = repository->IndexManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");


    const auto untrackedFiles = indexManager.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 1);
    ASSERT_EQ(untrackedFiles[0], "file.txt");
}

TEST_F(IndexTests, getUntrackedFileList_stagedFile)
{
    const auto indexManager = repository->IndexManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");


    const auto untrackedFiles = indexManager.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getUntrackedFileList_untrackedFileInDir)
{
    const auto indexManager = repository->IndexManager();


    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");


    const auto untrackedFiles = indexManager.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 1);
    ASSERT_EQ(untrackedFiles[0], "dir/file.txt");
}

TEST_F(IndexTests, getUntrackedFileList_untrackedFileInDirStaged)
{
    const auto indexManager = repository->IndexManager();


    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    indexManager.add("dir/file.txt");


    const auto untrackedFiles = indexManager.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getUntrackedFileList_trackedModified)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");


    const auto untrackedFiles = indexManager.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getUntrackedFileList_trackedDeleted)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");
    std::filesystem::remove(repositoryPath / "file.txt");


    const auto untrackedFiles = indexManager.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}


TEST_F(IndexTests, getStagedFilesList_commitedFile)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");


    const auto stagedFiles = indexManager.getStagedFilesListWithStatus();
    ASSERT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, getStagedFilesList_stagedFile)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");


    const auto stagedFiles = indexManager.getStagedFilesListWithStatus();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0].path, "file.txt");
    EXPECT_EQ(stagedFiles[0].status, CppGit::DiffIndexStatus::ADDED);
}

TEST_F(IndexTests, getStagedFilesList_stagedFile_pattern)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "another_file.txt", "");
    indexManager.add("file.txt");
    indexManager.add("another_file.txt");


    const auto stagedFiles = indexManager.getStagedFilesList("file*");
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "file.txt");
}

TEST_F(IndexTests, isFileStaged_fileNotStaged)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");


    EXPECT_FALSE(indexManager.isFileStaged("file.txt"));
}

TEST_F(IndexTests, isFileStaged_fileStaged)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");


    EXPECT_TRUE(indexManager.isFileStaged("file.txt"));
}

TEST_F(IndexTests, isFileStaged_FileNotExist)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();

    commitsManager.createCommit("Initial commit");

    EXPECT_FALSE(indexManager.isFileStaged("file2.txt"));
}

TEST_F(IndexTests, getNotStagedFiles_noFiles)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");


    const auto notStagedFiles = indexManager.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 0);
}

TEST_F(IndexTests, getNotStagedFiles_untrackedNotStagedFile)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");


    const auto notStagedFiles = indexManager.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 1);
    EXPECT_EQ(notStagedFiles[0], "file.txt");
}

TEST_F(IndexTests, getNotStagedFiles_untrackedStagedFile)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");


    const auto notStagedFiles = indexManager.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 0);
}

TEST_F(IndexTests, getNotStagedFiles_trackedNotModified)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");


    const auto notStagedFiles = indexManager.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 0);
}

TEST_F(IndexTests, getNotStagedFiles_trackedModifiedNotStaged)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");


    const auto notStagedFiles = indexManager.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 1);
    EXPECT_EQ(notStagedFiles[0], "file.txt");
}

TEST_F(IndexTests, getNotStagedFiles_trackedModifiedStaged)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    indexManager.add("file.txt");


    const auto notStagedFiles = indexManager.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 0);
}


TEST_F(IndexTests, getNotStagedFiles_trackedDeletedNotStaged)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    indexManager.add("file.txt");
    commitsManager.createCommit("Initial commit");
    std::filesystem::remove(repositoryPath / "file.txt");


    const auto notStagedFiles = indexManager.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 1);
    EXPECT_EQ(notStagedFiles[0], "file.txt");
}

TEST_F(IndexTests, areAnyNotStagedTrackedFiles_justFirstCommit)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();

    commitsManager.createCommit("Initial commit");

    EXPECT_FALSE(indexManager.areAnyNotStagedTrackedFiles());
}

TEST_F(IndexTests, areAnyNotStagedTrackedFiles_notTrackedFile)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");


    EXPECT_FALSE(indexManager.areAnyNotStagedTrackedFiles());
}

TEST_F(IndexTests, areAnyNotStagedTrackedFiles_stagedNotTrackedBefore)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    indexManager.add("file.txt");


    EXPECT_FALSE(indexManager.areAnyNotStagedTrackedFiles());
}

TEST_F(IndexTests, areAnyNotStagedTrackedFiles_trackedNotChanged)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    indexManager.add("file.txt");
    commitsManager.createCommit("Second commit");


    EXPECT_FALSE(indexManager.areAnyNotStagedTrackedFiles());
}

TEST_F(IndexTests, areAnyNotStagedTrackedFiles_trackedChangedNotStagged)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    indexManager.add("file.txt");
    commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Modified");


    EXPECT_TRUE(indexManager.areAnyNotStagedTrackedFiles());
}

TEST_F(IndexTests, areAnyNotStagedTrackedFiles_trackedChangedStagged)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    indexManager.add("file.txt");
    commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Modified");
    indexManager.add("file.txt");


    EXPECT_FALSE(indexManager.areAnyNotStagedTrackedFiles());
}

TEST_F(IndexTests, areAnyNotStagedTrackedFiles_trackedDeletedNotStaged)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    indexManager.add("file.txt");
    commitsManager.createCommit("Second commit");
    std::filesystem::remove(repositoryPath / "file.txt");


    EXPECT_TRUE(indexManager.areAnyNotStagedTrackedFiles());
}

TEST_F(IndexTests, areAnyNotStagedTrackedFiles_trackedDeletedStaged)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    indexManager.add("file.txt");
    commitsManager.createCommit("Second commit");
    std::filesystem::remove(repositoryPath / "file.txt");
    indexManager.add("file.txt");


    EXPECT_FALSE(indexManager.areAnyNotStagedTrackedFiles());
}

TEST_F(IndexTests, areAnyStagedFiles_justFirstCommit)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();

    commitsManager.createCommit("Initial commit");

    EXPECT_FALSE(indexManager.areAnyStagedFiles());
}

TEST_F(IndexTests, areAnyStagedFiles_notTrackedFile)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");


    EXPECT_FALSE(indexManager.areAnyStagedFiles());
}

TEST_F(IndexTests, areAnyStagedFiles_stagedNotTrackedBefore)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    indexManager.add("file.txt");


    EXPECT_TRUE(indexManager.areAnyStagedFiles());
}

TEST_F(IndexTests, areAnyStagedFiles_trackedNotChanged)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    indexManager.add("file.txt");
    commitsManager.createCommit("Second commit");


    EXPECT_FALSE(indexManager.areAnyStagedFiles());
}

TEST_F(IndexTests, areAnyStagedFiles_trackedChangedNotStagged)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    indexManager.add("file.txt");
    commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Modified");


    EXPECT_FALSE(indexManager.areAnyStagedFiles());
}

TEST_F(IndexTests, areAnyStagedFiles_trackedChangedStagged)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    indexManager.add("file.txt");
    commitsManager.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Modified");
    indexManager.add("file.txt");


    EXPECT_TRUE(indexManager.areAnyStagedFiles());
}

TEST_F(IndexTests, areAnyStagedFiles_trackedDeletedNotStaged)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    indexManager.add("file.txt");
    commitsManager.createCommit("Second commit");
    std::filesystem::remove(repositoryPath / "file.txt");


    EXPECT_FALSE(indexManager.areAnyStagedFiles());
}

TEST_F(IndexTests, areAnyStagedFiles_trackedDeletedStaged)
{
    const auto indexManager = repository->IndexManager();
    const auto commitsManager = repository->CommitsManager();


    commitsManager.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    indexManager.add("file.txt");
    commitsManager.createCommit("Second commit");
    std::filesystem::remove(repositoryPath / "file.txt");
    indexManager.add("file.txt");


    EXPECT_TRUE(indexManager.areAnyStagedFiles());
}
