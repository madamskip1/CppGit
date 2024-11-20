#include "BaseRepositoryFixture.hpp"
#include "Commits.hpp"
#include "Index.hpp"

#include <gtest/gtest.h>
#include <vector>

class IndexTests : public BaseRepositoryFixture
{
};

TEST_F(IndexTests, addRegularFile)
{
    // For staged we need at least one commit
    // TODO: getStagedFileList should do ls-files if no commit

    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");


    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    index.add("file.txt");

    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "file.txt");

    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "file.txt");
}

TEST_F(IndexTests, addRegularFile_FileDoesNotExist)
{
    auto index = repository->Index();

    ASSERT_THROW(index.add("file.txt"), std::runtime_error);
}

TEST_F(IndexTests, addRegularFileInDir)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");


    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    index.add("dir/file.txt");

    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");

    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "dir/file.txt");
}

TEST_F(IndexTests, addRegularFileInDir_providedDirAsPattern)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");


    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    index.add("dir");

    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "dir/file.txt");
}

TEST_F(IndexTests, addExecutableFile)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.sh", "echo Hello, World!");
    std::filesystem::permissions(repositoryPath / "file.sh", std::filesystem::perms::owner_exec | std::filesystem::perms::owner_read, std::filesystem::perm_options::add);


    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = index.getFilesInIndexListWithDetails();
    ASSERT_EQ(indexFiles.size(), 0);

    index.add("file.sh");

    indexFiles = index.getFilesInIndexListWithDetails();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0].fileMode, "100755");
    EXPECT_EQ(indexFiles[0].path, "file.sh");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "file.sh");
}

TEST_F(IndexTests, addSymlink)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    std::filesystem::create_symlink(repositoryPath / "file.txt", repositoryPath / "file-symlink.txt");


    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = index.getFilesInIndexListWithDetails();
    ASSERT_EQ(indexFiles.size(), 0);

    index.add("file-symlink.txt");

    indexFiles = index.getFilesInIndexListWithDetails();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0].fileMode, "120000");
    EXPECT_EQ(indexFiles[0].path, "file-symlink.txt");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "file-symlink.txt");
}

TEST_F(IndexTests, addOnDeletedFile)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Second commit");


    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "file.txt");

    std::filesystem::remove(repositoryPath / "file.txt");
    index.add("file.txt");

    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);
    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "file.txt");
}

TEST_F(IndexTests, addFilesWithAsteriskPattern)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World!");
    createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");


    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    index.add("*.txt");

    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 2);
    EXPECT_EQ(indexFiles[0], "file1.txt");
    EXPECT_EQ(indexFiles[1], "file2.txt");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 2);
    EXPECT_EQ(stagedFiles[0], "file1.txt");
    EXPECT_EQ(stagedFiles[1], "file2.txt");
}

TEST_F(IndexTests, addFilesWithAsteriskPatternInDirectories)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir1");
    createOrOverwriteFile(repositoryPath / "dir1" / "file.txt", "Hello, World!");
    std::filesystem::create_directory(repositoryPath / "dir2");
    createOrOverwriteFile(repositoryPath / "dir2" / "file.txt", "Hello, World!");


    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    index.add("dir*/*.txt");

    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 2);
    EXPECT_EQ(indexFiles[0], "dir1/file.txt");
    EXPECT_EQ(indexFiles[1], "dir2/file.txt");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 2);
    EXPECT_EQ(stagedFiles[0], "dir1/file.txt");
    EXPECT_EQ(stagedFiles[1], "dir2/file.txt");
}

TEST_F(IndexTests, addFile_fileNotExist)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    EXPECT_THROW(index.add("file.txt"), std::runtime_error);
}

TEST_F(IndexTests, removeRegularFile_fileNotDeletedFromWorkinDirectory)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");


    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "file.txt");

    index.remove("file.txt");

    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1); // file still exist in the working directory
}

TEST_F(IndexTests, removeRegularFile_fileDeletedFromWorkinDirectory)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");


    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "file.txt");

    std::filesystem::remove(repositoryPath / "file.txt");
    index.remove("file.txt");

    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);
}

TEST_F(IndexTests, removeRegularFile_force)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");


    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "file.txt");

    index.remove("file.txt", true);

    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);
}

TEST_F(IndexTests, removeRegularFile_notInIndex)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");


    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);
    EXPECT_THROW(index.remove("file.txt"), std::runtime_error);
}

TEST_F(IndexTests, removeRegularFile_notInIndex_force)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");


    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);
    EXPECT_THROW(index.remove("file.txt", true), std::runtime_error);
}

TEST_F(IndexTests, removeRegularFile_norInIndexNeitherInWorkingDirectory)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    EXPECT_THROW(index.remove("file.txt", true), std::runtime_error);
}

TEST_F(IndexTests, removeRegularFileInDir)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    index.add("dir/file.txt");


    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");

    index.remove("dir/file.txt");

    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1); // file still exist in the working directory
}

TEST_F(IndexTests, removeRegularFileInDir_providedDirAsPattern)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    index.add("dir/file.txt");


    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");

    index.remove("dir");

    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1); // file still exist in the working directory
}

TEST_F(IndexTests, removeRegularFileInDir_force)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    index.add("dir/file.txt");


    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");

    index.remove("dir", true);

    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);
}

TEST_F(IndexTests, removeRegularFileInDir_notInIndex)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");


    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    EXPECT_THROW(index.remove("dir"), std::runtime_error);
}

TEST_F(IndexTests, removeRegularFileInDir_notInIndex_force)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");


    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    EXPECT_THROW(index.remove("dir", true), std::runtime_error);
}

TEST_F(IndexTests, removeRegularFileInDir_removedFile)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    index.add("dir/file.txt");


    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");

    std::filesystem::remove(repositoryPath / "dir" / "file.txt");
    index.remove("dir/file.txt");

    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);
}


TEST_F(IndexTests, restoreAllStaged_noChanges)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
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


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");


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
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
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


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
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


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
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


    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! 1");
    index.add("file1.txt");
    createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World! 2");
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

    createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World!");
    index.add("file1.txt");
    createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");


    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);

    index.restoreAllStaged();

    stagedFiles = index.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}


TEST_F(IndexTests, notDirty_noCommitsYet)
{
    auto index = repository->Index();

    EXPECT_THROW(index.isDirty(), std::runtime_error);
}

TEST_F(IndexTests, dirty_noCommitsYet)
{
    auto index = repository->Index();

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");

    EXPECT_THROW(index.isDirty(), std::runtime_error);
}

TEST_F(IndexTests, dirty_noCommitsYet_FileAddedToIndex)
{
    auto index = repository->Index();

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");

    EXPECT_THROW(index.isDirty(), std::runtime_error);
}

TEST_F(IndexTests, notDirty)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");


    EXPECT_FALSE(index.isDirty());
}


TEST_F(IndexTests, dirty_changesInCachedNotAdded)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");


    EXPECT_TRUE(index.isDirty());
}

TEST_F(IndexTests, dirty_changesInCachedAdded)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    index.add("file.txt");


    EXPECT_TRUE(index.isDirty());
}

TEST_F(IndexTests, notdirty_untrackedFile)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");


    EXPECT_FALSE(index.isDirty());
}

TEST_F(IndexTests, dirty_untrackedFileAdded)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");
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


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");


    auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 1);
    ASSERT_EQ(untrackedFiles[0], "file.txt");
}

TEST_F(IndexTests, getUntrackedFileList_stagedFile)
{
    auto index = repository->Index();


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");


    auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getUntrackedFileList_untrackedFileInDir)
{
    auto index = repository->Index();


    std::filesystem::create_directory(repositoryPath / "dir");
    createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");


    auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 1);
    ASSERT_EQ(untrackedFiles[0], "dir/file.txt");
}

TEST_F(IndexTests, getUntrackedFileList_untrackedFileInDirStaged)
{
    auto index = repository->Index();


    std::filesystem::create_directory(repositoryPath / "dir");
    createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    index.add("dir/file.txt");


    auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getUntrackedFileList_trackedModified)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");


    auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getUntrackedFileList_trackedDeleted)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    std::filesystem::remove(repositoryPath / "file.txt");


    auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getStagedFilesList_emptyRepo_noCommitsYet)
{
    auto index = repository->Index();

    EXPECT_THROW(index.getStagedFilesListWithStatus(), std::runtime_error);
}


TEST_F(IndexTests, getStagedFilesList_notStagedFile_noCommitsYet)
{
    auto index = repository->Index();

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");

    EXPECT_THROW(index.getStagedFilesListWithStatus(), std::runtime_error);
}

TEST_F(IndexTests, getStagedFilesList_stagedFile_noCommitsYet)
{
    auto index = repository->Index();

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");

    EXPECT_THROW(index.getStagedFilesListWithStatus(), std::runtime_error);
}

TEST_F(IndexTests, getStagedFilesList_commitedFile)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
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
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");


    auto stagedFiles = index.getStagedFilesListWithStatus();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0].path, "file.txt");
    EXPECT_EQ(stagedFiles[0].status, CppGit::DiffIndexStatus::ADDED);
}

TEST_F(IndexTests, getStagedFilesList_stagedFile_pattern)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "");
    createOrOverwriteFile(repositoryPath / "another_file.txt", "");
    index.add("file.txt");
    index.add("another_file.txt");


    auto stagedFiles = index.getStagedFilesList("file*");
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "file.txt");
}

TEST_F(IndexTests, isFileStaged_noCommitsYet)
{
    auto index = repository->Index();

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");

    EXPECT_THROW(index.isFileStaged("file.txt"), std::runtime_error);
}

TEST_F(IndexTests, isFileStaged_fileStaged_noCommitsYet)
{
    auto index = repository->Index();

    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");

    EXPECT_THROW(index.isFileStaged("file.txt"), std::runtime_error);
}

TEST_F(IndexTests, isFileStaged_fileNotStaged)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");


    EXPECT_FALSE(index.isFileStaged("file.txt"));
}

TEST_F(IndexTests, isFileStaged_fileStaged)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");


    EXPECT_TRUE(index.isFileStaged("file.txt"));
}

TEST_F(IndexTests, isFileStaged_FileNotExist)
{
    auto index = repository->Index();
    auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    EXPECT_FALSE(index.isFileStaged("file2.txt"));
}

TEST_F(IndexTests, getNotStagedFiles_noFiles)
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
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");


    auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 1);
    EXPECT_EQ(notStagedFiles[0], "file.txt");
}

TEST_F(IndexTests, getNotStagedFiles_untrackedStagedFile)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");


    auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 0);
}

TEST_F(IndexTests, getNotStagedFiles_trackedNotModified)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");


    auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 0);
}

TEST_F(IndexTests, getNotStagedFiles_trackedModifiedNotStaged)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");


    auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 1);
    EXPECT_EQ(notStagedFiles[0], "file.txt");
}

TEST_F(IndexTests, getNotStagedFiles_trackedModifiedStaged)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    index.add("file.txt");


    auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 0);
}


TEST_F(IndexTests, getNotStagedFiles_trackedDeletedNotStaged)
{
    auto index = repository->Index();
    auto commits = repository->Commits();


    createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    std::filesystem::remove(repositoryPath / "file.txt");


    auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 1);
    EXPECT_EQ(notStagedFiles[0], "file.txt");
}
