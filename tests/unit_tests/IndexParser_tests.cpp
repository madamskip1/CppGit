#include <CppGit/_details/Parser/IndexParser.hpp>
#include <gtest/gtest.h>

TEST(IndexParserTests, parseStageDetailedEntry)
{
    constexpr auto* indexEntryLine = "100644 1234567890abcdef1234567890abcdef12345678 0       file.txt";
    const auto indexEntry = CppGit::IndexParser::parseStageDetailedEntry(indexEntryLine);

    EXPECT_EQ(indexEntry.fileMode, 100'644);
    EXPECT_EQ(indexEntry.objectHash, "1234567890abcdef1234567890abcdef12345678");
    EXPECT_EQ(indexEntry.stageNumber, 0);
    EXPECT_EQ(indexEntry.path, "file.txt");
}

TEST(IndexParserTests, parseStageDetailedList)
{
    constexpr auto* indexContent = "100644 1234567890abcdef1234567890abcdef12345678 0\tfile.txt\n"
                                   "100755 1234567890abcdef1234567890abcdef12345679 3       file.exe";
    const auto indexEntries = CppGit::IndexParser::parseStageDetailedList(indexContent);

    EXPECT_EQ(indexEntries.size(), 2);
    EXPECT_EQ(indexEntries[0].fileMode, 100'644);
    EXPECT_EQ(indexEntries[0].objectHash, "1234567890abcdef1234567890abcdef12345678");
    EXPECT_EQ(indexEntries[0].stageNumber, 0);
    EXPECT_EQ(indexEntries[0].path, "file.txt");
    EXPECT_EQ(indexEntries[1].fileMode, 100'755);
    EXPECT_EQ(indexEntries[1].objectHash, "1234567890abcdef1234567890abcdef12345679");
    EXPECT_EQ(indexEntries[1].stageNumber, 3);
    EXPECT_EQ(indexEntries[1].path, "file.exe");
}

TEST(IndexParserTests, parseStagedCacheListOnlyFilenames)
{
    constexpr auto* indexContent = "file.txt\n"
                                   "file.exe";
    const auto indexEntries = CppGit::IndexParser::parseCacheFilenameList(indexContent);

    EXPECT_EQ(indexEntries.size(), 2);
    EXPECT_EQ(indexEntries[0], "file.txt");
    EXPECT_EQ(indexEntries[1], "file.exe");
}

TEST(IndexParserTests, parseStageDetailedList_Empty)
{
    constexpr auto* indexContent = "";
    const auto indexEntries = CppGit::IndexParser::parseStageDetailedList(indexContent);

    EXPECT_EQ(indexEntries.size(), 0);
}

TEST(IndexParserTests, parseStagedCacheListOnlyFilenames_Empty)
{
    constexpr auto* indexContent = "";
    const auto indexEntries = CppGit::IndexParser::parseCacheFilenameList(indexContent);

    EXPECT_EQ(indexEntries.size(), 0);
}

TEST(IndexParserTests, parseDiffIndexWithStatusEntry_Added)
{
    constexpr auto* diffIndexLine = "A file.txt";
    const auto diffIndexEntry = CppGit::IndexParser::parseDiffIndexWithStatusEntry(diffIndexLine);

    EXPECT_EQ(diffIndexEntry.path, "file.txt");
    EXPECT_EQ(diffIndexEntry.status, CppGit::DiffIndexStatus::ADDED);
}

TEST(IndexParserTests, parseDiffIndexWithStatusEntry_Deleted)
{
    constexpr auto* diffIndexLine = "D file.txt";
    const auto diffIndexEntry = CppGit::IndexParser::parseDiffIndexWithStatusEntry(diffIndexLine);

    EXPECT_EQ(diffIndexEntry.path, "file.txt");
    EXPECT_EQ(diffIndexEntry.status, CppGit::DiffIndexStatus::DELETED);
}

TEST(IndexParserTests, parseDiffIndexWithStatusEntry_Modified)
{
    constexpr auto* diffIndexLine = "M file.txt";
    const auto diffIndexEntry = CppGit::IndexParser::parseDiffIndexWithStatusEntry(diffIndexLine);

    EXPECT_EQ(diffIndexEntry.path, "file.txt");
    EXPECT_EQ(diffIndexEntry.status, CppGit::DiffIndexStatus::MODIFIED);
}

TEST(IndexParserTests, parseDiffIndexWithStatusEntry_Renamed)
{
    constexpr auto* diffIndexLine = "R file.txt";
    const auto diffIndexEntry = CppGit::IndexParser::parseDiffIndexWithStatusEntry(diffIndexLine);

    EXPECT_EQ(diffIndexEntry.path, "file.txt");
    EXPECT_EQ(diffIndexEntry.status, CppGit::DiffIndexStatus::RENAMED);
}

TEST(IndexParserTests, parseDiffIndexWithStatusEntry_Copied)
{
    constexpr auto* diffIndexLine = "C file.txt";
    const auto diffIndexEntry = CppGit::IndexParser::parseDiffIndexWithStatusEntry(diffIndexLine);

    EXPECT_EQ(diffIndexEntry.path, "file.txt");
    EXPECT_EQ(diffIndexEntry.status, CppGit::DiffIndexStatus::COPIED);
}

TEST(IndexParserTests, parseDiffIndexWithStatusEntry_TypeChanged)
{
    constexpr auto* diffIndexLine = "T file.txt";
    const auto diffIndexEntry = CppGit::IndexParser::parseDiffIndexWithStatusEntry(diffIndexLine);

    EXPECT_EQ(diffIndexEntry.path, "file.txt");
    EXPECT_EQ(diffIndexEntry.status, CppGit::DiffIndexStatus::TYPE_CHANGED);
}

TEST(IndexParserTests, parseDiffIndexWithStatusEntry_Unmerged)
{
    constexpr auto* diffIndexLine = "U file.txt";
    const auto diffIndexEntry = CppGit::IndexParser::parseDiffIndexWithStatusEntry(diffIndexLine);

    EXPECT_EQ(diffIndexEntry.path, "file.txt");
    EXPECT_EQ(diffIndexEntry.status, CppGit::DiffIndexStatus::UNMERGED);
}

TEST(IndexParserTests, parseDiffIndexWithStatusEntry_Unknown)
{
    constexpr auto* diffIndexLine = "X file.txt";
    const auto diffIndexEntry = CppGit::IndexParser::parseDiffIndexWithStatusEntry(diffIndexLine);

    EXPECT_EQ(diffIndexEntry.path, "file.txt");
    EXPECT_EQ(diffIndexEntry.status, CppGit::DiffIndexStatus::UNKNOWN);
}

TEST(IndexParserTests, parseDiffIndexWithStatusList_Empty)
{
    constexpr auto* diffIndexContent = "";
    const auto diffIndexEntries = CppGit::IndexParser::parseDiffIndexWithStatusList(diffIndexContent);

    EXPECT_EQ(diffIndexEntries.size(), 0);
}

TEST(IndexParserTests, parseDiffIndexWithStatusList)
{
    constexpr auto* diffIndexContent = "A file.txt\n"
                                       "D file.exe\n"
                                       "M file.cpp";
    const auto diffIndexEntries = CppGit::IndexParser::parseDiffIndexWithStatusList(diffIndexContent);

    EXPECT_EQ(diffIndexEntries.size(), 3);
    EXPECT_EQ(diffIndexEntries[0].path, "file.txt");
    EXPECT_EQ(diffIndexEntries[0].status, CppGit::DiffIndexStatus::ADDED);
    EXPECT_EQ(diffIndexEntries[1].path, "file.exe");
    EXPECT_EQ(diffIndexEntries[1].status, CppGit::DiffIndexStatus::DELETED);
    EXPECT_EQ(diffIndexEntries[2].path, "file.cpp");
    EXPECT_EQ(diffIndexEntries[2].status, CppGit::DiffIndexStatus::MODIFIED);
}

TEST(IndexParserTests, parseLsFilesEntry_TrackedNotUnmergedNotSkipWorktree)
{
    constexpr auto* lsFilesLine = "H file.txt";
    const auto lsFilesEntry = CppGit::IndexParser::parseLsFilesEntry(lsFilesLine);

    EXPECT_EQ(lsFilesEntry.path, "file.txt");
    EXPECT_EQ(lsFilesEntry.status, CppGit::LsFilesStatus::TRACKED_NOT_UNMERGED_NOT_SKIP_WORKTREE);
}

TEST(IndexParserTests, parseLsFilesEntry_TrackedSkipWorktree)
{
    constexpr auto* lsFilesLine = "S file.txt";
    const auto lsFilesEntry = CppGit::IndexParser::parseLsFilesEntry(lsFilesLine);

    EXPECT_EQ(lsFilesEntry.path, "file.txt");
    EXPECT_EQ(lsFilesEntry.status, CppGit::LsFilesStatus::TRACKED_SKIP_WORKTREE);
}

TEST(IndexParserTests, parseLsFilesEntry_TrackedUnmerged)
{
    constexpr auto* lsFilesLine = "M file.txt";
    const auto lsFilesEntry = CppGit::IndexParser::parseLsFilesEntry(lsFilesLine);

    EXPECT_EQ(lsFilesEntry.path, "file.txt");
    EXPECT_EQ(lsFilesEntry.status, CppGit::LsFilesStatus::TRACKED_UNMERGED);
}

TEST(IndexParserTests, parseLsFilesEntry_TrackedDeleted)
{
    constexpr auto* lsFilesLine = "R file.txt";
    const auto lsFilesEntry = CppGit::IndexParser::parseLsFilesEntry(lsFilesLine);

    EXPECT_EQ(lsFilesEntry.path, "file.txt");
    EXPECT_EQ(lsFilesEntry.status, CppGit::LsFilesStatus::TRACKED_DELETED);
}

TEST(IndexParserTests, parseLsFilesEntry_TrackedModified)
{
    constexpr auto* lsFilesLine = "C file.txt";
    const auto lsFilesEntry = CppGit::IndexParser::parseLsFilesEntry(lsFilesLine);

    EXPECT_EQ(lsFilesEntry.path, "file.txt");
    EXPECT_EQ(lsFilesEntry.status, CppGit::LsFilesStatus::TRACKED_MODIFIED);
}

TEST(IndexParserTests, parseLsFilesEntry_UntrackedConflicting)
{
    constexpr auto* lsFilesLine = "K file.txt";
    const auto lsFilesEntry = CppGit::IndexParser::parseLsFilesEntry(lsFilesLine);

    EXPECT_EQ(lsFilesEntry.path, "file.txt");
    EXPECT_EQ(lsFilesEntry.status, CppGit::LsFilesStatus::UNTRACKED_CONFLICTING);
}

TEST(IndexParserTests, parseLsFilesEntry_Untracked)
{
    constexpr auto* lsFilesLine = "? file.txt";
    const auto lsFilesEntry = CppGit::IndexParser::parseLsFilesEntry(lsFilesLine);

    EXPECT_EQ(lsFilesEntry.path, "file.txt");
    EXPECT_EQ(lsFilesEntry.status, CppGit::LsFilesStatus::UNTRACKED);
}

TEST(IndexParserTests, parseLsFilesEntry_ResolveUndo)
{
    constexpr auto* lsFilesLine = "U file.txt";
    const auto lsFilesEntry = CppGit::IndexParser::parseLsFilesEntry(lsFilesLine);

    EXPECT_EQ(lsFilesEntry.path, "file.txt");
    EXPECT_EQ(lsFilesEntry.status, CppGit::LsFilesStatus::RESOLVE_UNDO);
}

TEST(IndexParserTests, parseLsFilesList_Empty)
{
    constexpr auto* lsFilesContent = "";
    const auto lsFilesEntries = CppGit::IndexParser::parseLsFilesList(lsFilesContent);

    EXPECT_EQ(lsFilesEntries.size(), 0);
}

TEST(IndexParserTests, parseLsFilesList)
{
    constexpr auto* lsFilesContent = "H file.txt\n"
                                     "S file.exe\n"
                                     "M file.cpp";
    const auto lsFilesEntries = CppGit::IndexParser::parseLsFilesList(lsFilesContent);

    EXPECT_EQ(lsFilesEntries.size(), 3);
    EXPECT_EQ(lsFilesEntries[0].path, "file.txt");
    EXPECT_EQ(lsFilesEntries[0].status, CppGit::LsFilesStatus::TRACKED_NOT_UNMERGED_NOT_SKIP_WORKTREE);
    EXPECT_EQ(lsFilesEntries[1].path, "file.exe");
    EXPECT_EQ(lsFilesEntries[1].status, CppGit::LsFilesStatus::TRACKED_SKIP_WORKTREE);
    EXPECT_EQ(lsFilesEntries[2].path, "file.cpp");
    EXPECT_EQ(lsFilesEntries[2].status, CppGit::LsFilesStatus::TRACKED_UNMERGED);
}
