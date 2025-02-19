#include "BaseRepositoryFixture.hpp"
#include "Commits.hpp"
#include "Index.hpp"
#include "_details/FileUtility.hpp"

#include <gtest/gtest.h>
#include <vector>

class IndexTests : public BaseRepositoryFixture
{
};

TEST_F(IndexTests, addRegularFile)
{
    // For staged we need at least one commit
    // TODO: getStagedFileList should do ls-files if no commit

    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    const auto addResult = index.add("file.txt");


    ASSERT_EQ(addResult, CppGit::Error::NO_ERROR);
    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "file.txt");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "file.txt");
}

TEST_F(IndexTests, addRegularFile_fileNotExist)
{
    const auto index = repository->Index();

    const auto addResult = index.add("file.txt");

    ASSERT_EQ(addResult, CppGit::Error::PATTERN_NOT_MATCHING_ANY_FILES);
}

TEST_F(IndexTests, addRegularFileInDir)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    const auto addResult = index.add("dir/file.txt");


    ASSERT_EQ(addResult, CppGit::Error::NO_ERROR);
    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "dir/file.txt");
}

TEST_F(IndexTests, addRegularFileInDir_providedDirAsPattern)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();

    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    const auto addResult = index.add("dir");


    ASSERT_EQ(addResult, CppGit::Error::NO_ERROR);
    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "dir/file.txt");
}

TEST_F(IndexTests, addExecutableFile)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.sh", "echo Hello, World!");
    std::filesystem::permissions(repositoryPath / "file.sh", std::filesystem::perms::owner_exec | std::filesystem::perms::owner_read, std::filesystem::perm_options::add);
    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = index.getFilesInIndexListWithDetails();
    ASSERT_EQ(indexFiles.size(), 0);

    const auto addResult = index.add("file.sh");


    ASSERT_EQ(addResult, CppGit::Error::NO_ERROR);
    indexFiles = index.getFilesInIndexListWithDetails();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0].fileMode, 100'755);
    EXPECT_EQ(indexFiles[0].path, "file.sh");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "file.sh");
}

TEST_F(IndexTests, addSymlink)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    std::filesystem::create_symlink(repositoryPath / "file.txt", repositoryPath / "file-symlink.txt");
    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = index.getFilesInIndexListWithDetails();
    ASSERT_EQ(indexFiles.size(), 0);

    const auto addResult = index.add("file-symlink.txt");


    ASSERT_EQ(addResult, CppGit::Error::NO_ERROR);
    indexFiles = index.getFilesInIndexListWithDetails();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0].fileMode, 120'000);
    EXPECT_EQ(indexFiles[0].path, "file-symlink.txt");
    stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "file-symlink.txt");
}

TEST_F(IndexTests, addOnDeletedFile)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();

    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Second commit");
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "file.txt");
    std::filesystem::remove(repositoryPath / "file.txt");

    const auto addResult = index.add("file.txt");


    ASSERT_EQ(addResult, CppGit::Error::NO_ERROR);
    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);
    const auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "file.txt");
}

TEST_F(IndexTests, addFilesWithAsteriskPattern)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World!");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");
    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    const auto addResult = index.add("*.txt");


    ASSERT_EQ(addResult, CppGit::Error::NO_ERROR);
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
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir1");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir1" / "file.txt", "Hello, World!");
    std::filesystem::create_directory(repositoryPath / "dir2");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir2" / "file.txt", "Hello, World!");
    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    const auto addResult = index.add("dir*/*.txt");


    ASSERT_EQ(addResult, CppGit::Error::NO_ERROR);
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
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    const auto addResult = index.add("file.txt");


    ASSERT_EQ(addResult, CppGit::Error::PATTERN_NOT_MATCHING_ANY_FILES);
}

TEST_F(IndexTests, removeRegularFile_fileNotDeletedFromWorkinDirectory)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "file.txt");

    const auto removeResult = index.remove("file.txt");


    EXPECT_EQ(removeResult, CppGit::Error::NO_ERROR);
    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1); // file still exist in the working directory
}

TEST_F(IndexTests, removeRegularFile_fileDeletedFromWorkinDirectory)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "file.txt");
    std::filesystem::remove(repositoryPath / "file.txt");

    const auto removeResult = index.remove("file.txt");


    EXPECT_EQ(removeResult, CppGit::Error::NO_ERROR);
    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);
}

TEST_F(IndexTests, removeRegularFile_force)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "file.txt");

    const auto removeResult = index.remove("file.txt", true);


    EXPECT_EQ(removeResult, CppGit::Error::NO_ERROR);
    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);
}

TEST_F(IndexTests, removeRegularFile_notInIndex)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    const auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    const auto removeResult = index.remove("file.txt");


    EXPECT_EQ(removeResult, CppGit::Error::PATTERN_NOT_MATCHING_ANY_FILES);
}

TEST_F(IndexTests, removeRegularFile_notInIndex_force)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    const auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    const auto removeResult = index.remove("file.txt", true);


    EXPECT_EQ(removeResult, CppGit::Error::PATTERN_NOT_MATCHING_ANY_FILES);
}

TEST_F(IndexTests, removeRegularFile_norInIndexNeitherInWorkingDirectory)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    const auto removeResult = index.remove("file.txt", true);


    EXPECT_EQ(removeResult, CppGit::Error::PATTERN_NOT_MATCHING_ANY_FILES);
}

TEST_F(IndexTests, removeRegularFileInDir)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    index.add("dir/file.txt");
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");

    const auto removeResult = index.remove("dir/file.txt");


    EXPECT_EQ(removeResult, CppGit::Error::NO_ERROR);
    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1); // file still exist in the working directory
}

TEST_F(IndexTests, removeRegularFileInDir_providedDirAsPattern)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    index.add("dir/file.txt");
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");

    const auto removeResult = index.remove("dir");


    EXPECT_EQ(removeResult, CppGit::Error::NO_ERROR);
    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1); // file still exist in the working directory
}

TEST_F(IndexTests, removeRegularFileInDir_force)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    index.add("dir/file.txt");
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");

    const auto removeResult = index.remove("dir", true);


    EXPECT_EQ(removeResult, CppGit::Error::NO_ERROR);
    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);
}

TEST_F(IndexTests, removeRegularFileInDir_notInIndex)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    const auto removeResult = index.remove("dir");


    EXPECT_EQ(removeResult, CppGit::Error::PATTERN_NOT_MATCHING_ANY_FILES);
}

TEST_F(IndexTests, removeRegularFileInDir_notInIndex_force)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);

    const auto removeResult = index.remove("dir", true);


    EXPECT_EQ(removeResult, CppGit::Error::PATTERN_NOT_MATCHING_ANY_FILES);
}

TEST_F(IndexTests, removeRegularFileInDir_removedFile)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    index.add("dir/file.txt");
    auto indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 1);
    EXPECT_EQ(indexFiles[0], "dir/file.txt");
    std::filesystem::remove(repositoryPath / "dir" / "file.txt");

    const auto removeResult = index.remove("dir/file.txt");


    EXPECT_EQ(removeResult, CppGit::Error::NO_ERROR);
    indexFiles = index.getFilesInIndexList();
    ASSERT_EQ(indexFiles.size(), 0);
}


TEST_F(IndexTests, restoreAllStaged_noChanges)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
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
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 0);

    index.restoreAllStaged();


    stagedFiles = index.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, restoreAllStaged_stagedNewFile)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);

    index.restoreAllStaged();


    stagedFiles = index.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, restoreAllStaged_stagedChanges)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    index.add("file.txt");
    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);

    index.restoreAllStaged();


    stagedFiles = index.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, restoreAllStaged_stagedChangesInDir)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    index.add("dir/file.txt");
    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);

    index.restoreAllStaged();


    stagedFiles = index.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, restoreAllStaged_multipleFile)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World! 1");
    index.add("file1.txt");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World! 2");
    index.add("file2.txt");
    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 2);

    index.restoreAllStaged();


    stagedFiles = index.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, restoreAllStaged_multipleFile_notAllStaged)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file1.txt", "Hello, World!");
    index.add("file1.txt");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");
    auto stagedFiles = index.getStagedFilesList();
    ASSERT_EQ(stagedFiles.size(), 1);

    index.restoreAllStaged();


    stagedFiles = index.getStagedFilesList();
    EXPECT_EQ(stagedFiles.size(), 0);
}


TEST_F(IndexTests, notDirty_noCommitsYet)
{
    const auto index = repository->Index();

    EXPECT_THROW(static_cast<void>(index.isDirty()), std::runtime_error); // Static cast to prevent warning from discard value
}

TEST_F(IndexTests, dirty_noCommitsYet)
{
    const auto index = repository->Index();

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");

    EXPECT_THROW(static_cast<void>(index.isDirty()), std::runtime_error); // Static cast to prevent warning from discard value
}

TEST_F(IndexTests, dirty_noCommitsYet_FileAddedToIndex)
{
    const auto index = repository->Index();

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");

    EXPECT_THROW(static_cast<void>(index.isDirty()), std::runtime_error); // Static cast to prevent warning from discard value
}

TEST_F(IndexTests, notDirty)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");


    EXPECT_FALSE(index.isDirty());
}


TEST_F(IndexTests, dirty_changesInCachedNotAdded)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");


    EXPECT_TRUE(index.isDirty());
}

TEST_F(IndexTests, dirty_changesInCachedAdded)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    index.add("file.txt");


    EXPECT_TRUE(index.isDirty());
}

TEST_F(IndexTests, notdirty_untrackedFile)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");


    EXPECT_FALSE(index.isDirty());
}

TEST_F(IndexTests, dirty_untrackedFileAdded)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file2.txt", "Hello, World!");
    index.add("file2.txt");


    EXPECT_TRUE(index.isDirty());
}

TEST_F(IndexTests, getUntrackedFileList_emptyRepo)
{
    const auto index = repository->Index();

    const auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getUntrackedFileList_notStagedFile)
{
    const auto index = repository->Index();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");


    const auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 1);
    ASSERT_EQ(untrackedFiles[0], "file.txt");
}

TEST_F(IndexTests, getUntrackedFileList_stagedFile)
{
    const auto index = repository->Index();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");


    const auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getUntrackedFileList_untrackedFileInDir)
{
    const auto index = repository->Index();


    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");


    const auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 1);
    ASSERT_EQ(untrackedFiles[0], "dir/file.txt");
}

TEST_F(IndexTests, getUntrackedFileList_untrackedFileInDirStaged)
{
    const auto index = repository->Index();


    std::filesystem::create_directory(repositoryPath / "dir");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "dir" / "file.txt", "Hello, World!");
    index.add("dir/file.txt");


    const auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getUntrackedFileList_trackedModified)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");


    const auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getUntrackedFileList_trackedDeleted)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    std::filesystem::remove(repositoryPath / "file.txt");


    const auto untrackedFiles = index.getUntrackedFilesList();
    ASSERT_EQ(untrackedFiles.size(), 0);
}

TEST_F(IndexTests, getStagedFilesList_emptyRepo_noCommitsYet)
{
    const auto index = repository->Index();

    EXPECT_THROW(index.getStagedFilesListWithStatus(), std::runtime_error);
}


TEST_F(IndexTests, getStagedFilesList_notStagedFile_noCommitsYet)
{
    const auto index = repository->Index();

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");

    EXPECT_THROW(index.getStagedFilesListWithStatus(), std::runtime_error);
}

TEST_F(IndexTests, getStagedFilesList_stagedFile_noCommitsYet)
{
    const auto index = repository->Index();

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");

    EXPECT_THROW(index.getStagedFilesListWithStatus(), std::runtime_error);
}

TEST_F(IndexTests, getStagedFilesList_commitedFile)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");


    const auto stagedFiles = index.getStagedFilesListWithStatus();
    ASSERT_EQ(stagedFiles.size(), 0);
}

TEST_F(IndexTests, getStagedFilesList_stagedFile)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");


    const auto stagedFiles = index.getStagedFilesListWithStatus();
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0].path, "file.txt");
    EXPECT_EQ(stagedFiles[0].status, CppGit::DiffIndexStatus::ADDED);
}

TEST_F(IndexTests, getStagedFilesList_stagedFile_pattern)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "another_file.txt", "");
    index.add("file.txt");
    index.add("another_file.txt");


    const auto stagedFiles = index.getStagedFilesList("file*");
    ASSERT_EQ(stagedFiles.size(), 1);
    EXPECT_EQ(stagedFiles[0], "file.txt");
}

TEST_F(IndexTests, isFileStaged_noCommitsYet)
{
    const auto index = repository->Index();

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");

    EXPECT_THROW(static_cast<void>(index.isFileStaged("file.txt")), std::runtime_error); // Static cast to prevent warning from discard value
}

TEST_F(IndexTests, isFileStaged_fileStaged_noCommitsYet)
{
    const auto index = repository->Index();

    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");

    EXPECT_THROW(static_cast<void>(index.isFileStaged("file.txt")), std::runtime_error); // Static cast to prevent warning from discard value
}

TEST_F(IndexTests, isFileStaged_fileNotStaged)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");


    EXPECT_FALSE(index.isFileStaged("file.txt"));
}

TEST_F(IndexTests, isFileStaged_fileStaged)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");


    EXPECT_TRUE(index.isFileStaged("file.txt"));
}

TEST_F(IndexTests, isFileStaged_FileNotExist)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    EXPECT_FALSE(index.isFileStaged("file2.txt"));
}

TEST_F(IndexTests, getNotStagedFiles_noFiles)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");


    const auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 0);
}

TEST_F(IndexTests, getNotStagedFiles_untrackedNotStagedFile)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");


    const auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 1);
    EXPECT_EQ(notStagedFiles[0], "file.txt");
}

TEST_F(IndexTests, getNotStagedFiles_untrackedStagedFile)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");


    const auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 0);
}

TEST_F(IndexTests, getNotStagedFiles_trackedNotModified)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");


    const auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 0);
}

TEST_F(IndexTests, getNotStagedFiles_trackedModifiedNotStaged)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");


    const auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 1);
    EXPECT_EQ(notStagedFiles[0], "file.txt");
}

TEST_F(IndexTests, getNotStagedFiles_trackedModifiedStaged)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World! Modified");
    index.add("file.txt");


    const auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 0);
}


TEST_F(IndexTests, getNotStagedFiles_trackedDeletedNotStaged)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Hello, World!");
    index.add("file.txt");
    commits.createCommit("Initial commit");
    std::filesystem::remove(repositoryPath / "file.txt");


    const auto notStagedFiles = index.getNotStagedFilesList();
    ASSERT_EQ(notStagedFiles.size(), 1);
    EXPECT_EQ(notStagedFiles[0], "file.txt");
}

TEST_F(IndexTests, areAnyNotStagedTrackedFiles_justFirstCommit)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    EXPECT_FALSE(index.areAnyNotStagedTrackedFiles());
}

TEST_F(IndexTests, areAnyNotStagedTrackedFiles_notTrackedFile)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");


    EXPECT_FALSE(index.areAnyNotStagedTrackedFiles());
}

TEST_F(IndexTests, areAnyNotStagedTrackedFiles_stagedNotTrackedBefore)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");


    EXPECT_FALSE(index.areAnyNotStagedTrackedFiles());
}

TEST_F(IndexTests, areAnyNotStagedTrackedFiles_trackedNotChanged)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    commits.createCommit("Second commit");


    EXPECT_FALSE(index.areAnyNotStagedTrackedFiles());
}

TEST_F(IndexTests, areAnyNotStagedTrackedFiles_trackedChangedNotStagged)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Modified");


    EXPECT_TRUE(index.areAnyNotStagedTrackedFiles());
}

TEST_F(IndexTests, areAnyNotStagedTrackedFiles_trackedChangedStagged)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Modified");
    index.add("file.txt");


    EXPECT_FALSE(index.areAnyNotStagedTrackedFiles());
}

TEST_F(IndexTests, areAnyNotStagedTrackedFiles_trackedDeletedNotStaged)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    commits.createCommit("Second commit");
    std::filesystem::remove(repositoryPath / "file.txt");


    EXPECT_TRUE(index.areAnyNotStagedTrackedFiles());
}

TEST_F(IndexTests, areAnyNotStagedTrackedFiles_trackedDeletedStaged)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    commits.createCommit("Second commit");
    std::filesystem::remove(repositoryPath / "file.txt");
    index.add("file.txt");


    EXPECT_FALSE(index.areAnyNotStagedTrackedFiles());
}

TEST_F(IndexTests, areAnyStagedFiles_justFirstCommit)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();

    commits.createCommit("Initial commit");

    EXPECT_FALSE(index.areAnyStagedFiles());
}

TEST_F(IndexTests, areAnyStagedFiles_notTrackedFile)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");


    EXPECT_FALSE(index.areAnyStagedFiles());
}

TEST_F(IndexTests, areAnyStagedFiles_stagedNotTrackedBefore)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");


    EXPECT_TRUE(index.areAnyStagedFiles());
}

TEST_F(IndexTests, areAnyStagedFiles_trackedNotChanged)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    commits.createCommit("Second commit");


    EXPECT_FALSE(index.areAnyStagedFiles());
}

TEST_F(IndexTests, areAnyStagedFiles_trackedChangedNotStagged)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Modified");


    EXPECT_FALSE(index.areAnyStagedFiles());
}

TEST_F(IndexTests, areAnyStagedFiles_trackedChangedStagged)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    commits.createCommit("Second commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "Modified");
    index.add("file.txt");


    EXPECT_TRUE(index.areAnyStagedFiles());
}

TEST_F(IndexTests, areAnyStagedFiles_trackedDeletedNotStaged)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    commits.createCommit("Second commit");
    std::filesystem::remove(repositoryPath / "file.txt");


    EXPECT_FALSE(index.areAnyStagedFiles());
}

TEST_F(IndexTests, areAnyStagedFiles_trackedDeletedStaged)
{
    const auto index = repository->Index();
    const auto commits = repository->Commits();


    commits.createCommit("Initial commit");
    CppGit::_details::FileUtility::createOrOverwriteFile(repositoryPath / "file.txt", "");
    index.add("file.txt");
    commits.createCommit("Second commit");
    std::filesystem::remove(repositoryPath / "file.txt");
    index.add("file.txt");


    EXPECT_TRUE(index.areAnyStagedFiles());
}
