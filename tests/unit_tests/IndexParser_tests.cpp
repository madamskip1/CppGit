#include "Index.hpp"
#include "Parser/IndexParser.hpp"

#include <gtest/gtest.h>

TEST(IndexParserTests, parseStageDetailedEntry)
{
    auto indexEntryLine = "100644 1234567890abcdef1234567890abcdef12345678 0       file.txt";
    auto indexEntry = CppGit::IndexParser::parseStageDetailedEntry(indexEntryLine);

    EXPECT_EQ(indexEntry.fileMode, "100644");
    EXPECT_EQ(indexEntry.objectHash, "1234567890abcdef1234567890abcdef12345678");
    EXPECT_EQ(indexEntry.stageNumber, 0);
    EXPECT_EQ(indexEntry.path, "file.txt");
}

TEST(IndexParserTests, parseStageDetailedList)
{
    auto indexContent = "100644 1234567890abcdef1234567890abcdef12345678 0\tfile.txt\n"
                        "100755 1234567890abcdef1234567890abcdef12345679 3       file.exe";
    auto indexEntries = CppGit::IndexParser::parseStageDetailedList(indexContent);

    EXPECT_EQ(indexEntries.size(), 2);
    EXPECT_EQ(indexEntries[0].fileMode, "100644");
    EXPECT_EQ(indexEntries[0].objectHash, "1234567890abcdef1234567890abcdef12345678");
    EXPECT_EQ(indexEntries[0].stageNumber, 0);
    EXPECT_EQ(indexEntries[0].path, "file.txt");
    EXPECT_EQ(indexEntries[1].fileMode, "100755");
    EXPECT_EQ(indexEntries[1].objectHash, "1234567890abcdef1234567890abcdef12345679");
    EXPECT_EQ(indexEntries[1].stageNumber, 3);
    EXPECT_EQ(indexEntries[1].path, "file.exe");
}
