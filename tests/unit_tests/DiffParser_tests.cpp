#include <Parser/DiffParser.hpp>
#include <gtest/gtest.h>

TEST(ParseHeaderLine, oldMode)
{
    const std::string line = "old mode 100644";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::NO_LINE);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::OLD_MODE);
    EXPECT_EQ(std::get<int>(headerLine.value), 100'644);
}

TEST(ParseHeaderLine, newModeAfterOld)
{
    const std::string line = "new mode 100644";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::OLD_MODE);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::NEW_MODE);
    EXPECT_EQ(std::get<int>(headerLine.value), 100'644);
}

TEST(ParseHeaderLine, newFileMode)
{
    const std::string line = "new file mode 100644";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::NO_LINE);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::NEW_FILE);
    EXPECT_EQ(std::get<int>(headerLine.value), 100'644);
}

TEST(ParseHeaderLine, deletedFileMode)
{
    const std::string line = "deleted file mode 100644";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::NO_LINE);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::DELETED_FILE);
    EXPECT_EQ(std::get<int>(headerLine.value), 100'644);
}

TEST(ParseHeaderLine, similarityIndex)
{
    const std::string line = "similarity index 90%";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::NO_LINE);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::SIMILARITY_INDEX);
    EXPECT_EQ(std::get<int>(headerLine.value), 90);
}

TEST(ParseHeaderLine, renameFromAfterSimilarityIndex)
{
    const std::string line = "rename from old_file.txt";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::SIMILARITY_INDEX);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::RENAME_FROM);
    EXPECT_EQ(std::get<std::string_view>(headerLine.value), "old_file.txt");
}

TEST(ParseHeaderLine, renameToAfterRenameFrom)
{
    const std::string line = "rename to new_file.txt";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::RENAME_FROM);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::RENAME_TO);
    EXPECT_EQ(std::get<std::string_view>(headerLine.value), "new_file.txt");
}

TEST(ParseHeaderLine, copyFromAfterSimilarityIndex)
{
    const std::string line = "copy from old_file.txt";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::SIMILARITY_INDEX);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::COPY_FROM);
    EXPECT_EQ(std::get<std::string_view>(headerLine.value), "old_file.txt");
}

TEST(ParseHeaderLine, copyToAfterCopyFrom)
{
    const std::string line = "copy to new_file.txt";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::COPY_FROM);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::COPY_TO);
    EXPECT_EQ(std::get<std::string_view>(headerLine.value), "new_file.txt");
}

TEST(ParseHeaderLine, indexSingleParentWithMode)
{
    const std::string line = "index 180cf8328022becee9aaa2577a8f84ea2b9f3827..1d89a1850b82787e2766aa3c724048fc74ea4fbc 100644";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::NO_LINE);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::INDEX);
    const auto [parentIndex, childIndex, mode] = std::get<std::tuple<std::string_view, std::string_view, int>>(headerLine.value);

    EXPECT_EQ(parentIndex, "180cf8328022becee9aaa2577a8f84ea2b9f3827");
    EXPECT_EQ(childIndex, "1d89a1850b82787e2766aa3c724048fc74ea4fbc");
    EXPECT_EQ(mode, 100'644);
}

TEST(ParseHeaderLine, indexSingleParentWithoutMode)
{
    const std::string line = "index 180cf8328022becee9aaa2577a8f84ea2b9f3827..1d89a1850b82787e2766aa3c724048fc74ea4fbc";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::NO_LINE);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::INDEX);
    const auto [parentIndex, childIndex, mode] = std::get<std::tuple<std::string_view, std::string_view, int>>(headerLine.value);

    EXPECT_EQ(parentIndex, "180cf8328022becee9aaa2577a8f84ea2b9f3827");
    EXPECT_EQ(childIndex, "1d89a1850b82787e2766aa3c724048fc74ea4fbc");
    EXPECT_EQ(mode, 0);
}

TEST(ParseHeaderLine, indexSingleParentWithShortHashesh)
{
    const std::string line = "index 180cf83..1d89a18 100644";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::NO_LINE);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::INDEX);
    const auto [parentIndex, childIndex, mode] = std::get<std::tuple<std::string_view, std::string_view, int>>(headerLine.value);

    EXPECT_EQ(parentIndex, "180cf83");
    EXPECT_EQ(childIndex, "1d89a18");
    EXPECT_EQ(mode, 100'644);
}

TEST(ParseHeaderLine, indexMultipleParents)
{
    const std::string line = "index 6fefc78134f4d22c90a778f555c4137feded408e,1d89a1850b82787e2766aa3c724048fc74ea4fbc..180cf8328022becee9aaa2577a8f84ea2b9f3827";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::NO_LINE);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::INDEX);
    const auto [parentIndexes, childIndex, mode] = std::get<std::tuple<std::string_view, std::string_view, int>>(headerLine.value);

    EXPECT_EQ(parentIndexes, "6fefc78134f4d22c90a778f555c4137feded408e,1d89a1850b82787e2766aa3c724048fc74ea4fbc");
    EXPECT_EQ(childIndex, "180cf8328022becee9aaa2577a8f84ea2b9f3827");
    EXPECT_EQ(mode, 0);
}

TEST(ParseHeaderLine, indexAfterNewMode)
{
    const std::string line = "index 180cf8328022becee9aaa2577a8f84ea2b9f3827..1d89a1850b82787e2766aa3c724048fc74ea4fbc 100644";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::NEW_MODE);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::INDEX);
    const auto [parentIndex, childIndex, mode] = std::get<std::tuple<std::string_view, std::string_view, int>>(headerLine.value);

    EXPECT_EQ(parentIndex, "180cf8328022becee9aaa2577a8f84ea2b9f3827");
    EXPECT_EQ(childIndex, "1d89a1850b82787e2766aa3c724048fc74ea4fbc");
    EXPECT_EQ(mode, 100'644);
}

TEST(ParseHeaderLine, indexAfterNewFileMode)
{
    const std::string line = "index 180cf8328022becee9aaa2577a8f84ea2b9f3827..1d89a1850b82787e2766aa3c724048fc74ea4fbc 100644";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::NEW_FILE);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::INDEX);
    const auto [parentIndex, childIndex, mode] = std::get<std::tuple<std::string_view, std::string_view, int>>(headerLine.value);

    EXPECT_EQ(parentIndex, "180cf8328022becee9aaa2577a8f84ea2b9f3827");
    EXPECT_EQ(childIndex, "1d89a1850b82787e2766aa3c724048fc74ea4fbc");
    EXPECT_EQ(mode, 100'644);
}

TEST(ParseHeaderLine, indexAfterDeletedFileMode)
{
    const std::string line = "index 180cf8328022becee9aaa2577a8f84ea2b9f3827..1d89a1850b82787e2766aa3c724048fc74ea4fbc 100644";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::DELETED_FILE);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::INDEX);
    const auto [parentIndex, childIndex, mode] = std::get<std::tuple<std::string_view, std::string_view, int>>(headerLine.value);

    EXPECT_EQ(parentIndex, "180cf8328022becee9aaa2577a8f84ea2b9f3827");
    EXPECT_EQ(childIndex, "1d89a1850b82787e2766aa3c724048fc74ea4fbc");
    EXPECT_EQ(mode, 100'644);
}

TEST(ParseHeaderLine, indexAfterRenameTo)
{
    const std::string line = "index 180cf8328022becee9aaa2577a8f84ea2b9f3827..1d89a1850b82787e2766aa3c724048fc74ea4fbc 100644";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::RENAME_TO);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::INDEX);
    const auto [parentIndex, childIndex, mode] = std::get<std::tuple<std::string_view, std::string_view, int>>(headerLine.value);

    EXPECT_EQ(parentIndex, "180cf8328022becee9aaa2577a8f84ea2b9f3827");
    EXPECT_EQ(childIndex, "1d89a1850b82787e2766aa3c724048fc74ea4fbc");
    EXPECT_EQ(mode, 100'644);
}

TEST(ParseHeaderLine, indexAfterCopyTo)
{
    const std::string line = "index 180cf8328022becee9aaa2577a8f84ea2b9f3827..1d89a1850b82787e2766aa3c724048fc74ea4fbc 100644";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::COPY_TO);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::INDEX);
    const auto [parentIndex, childIndex, mode] = std::get<std::tuple<std::string_view, std::string_view, int>>(headerLine.value);

    EXPECT_EQ(parentIndex, "180cf8328022becee9aaa2577a8f84ea2b9f3827");
    EXPECT_EQ(childIndex, "1d89a1850b82787e2766aa3c724048fc74ea4fbc");
    EXPECT_EQ(mode, 100'644);
}

TEST(ParseHeaderLine, endOfHeaderAfterIndex)
{
    const std::string line = "whatever";
    const auto headerLine = CppGit::DiffParser::parseHeaderLine(line, CppGit::DiffParser::HeaderLineType::INDEX);

    ASSERT_EQ(headerLine.type, CppGit::DiffParser::HeaderLineType::END_HEADER);
    EXPECT_EQ(std::get<int>(headerLine.value), 0);
}
