#include "_details/Parser/Parser.hpp"

#include <gtest/gtest.h>

TEST(ParserTests, splitToStringViewByNewLine_withNewLineAtTheEnd)
{
    constexpr auto line = "line1\nline2\nline3\n";
    const auto result = CppGit::Parser::splitToStringViewsVector(line, '\n');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "line2");
    EXPECT_EQ(result[2], "line3");
    EXPECT_EQ(result[3], "");
}

TEST(ParserTests, splitToStringViewByNewLine_withoutNewLineAtTheEnd)
{
    constexpr auto line = "line1\nline2\nline3";
    const auto result = CppGit::Parser::splitToStringViewsVector(line, '\n');

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "line2");
    EXPECT_EQ(result[2], "line3");
}

TEST(ParserTests, splitToStringViewByNewLine_emptyLineWithNewLine)
{
    constexpr auto line = "\n";
    const auto result = CppGit::Parser::splitToStringViewsVector(line, '\n');

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "");
    EXPECT_EQ(result[1], "");
}

TEST(ParserTests, splitToStringViewByNewLine_emptyLineWithoutNewLine)
{
    constexpr auto line = "";
    const auto result = CppGit::Parser::splitToStringViewsVector(line, '\n');

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "");
}

TEST(ParserTests, splitToStringViewByNewLine_emptyLineWithTwoNewLines)
{
    constexpr auto line = "\n\n";
    const auto result = CppGit::Parser::splitToStringViewsVector(line, '\n');

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "");
}

TEST(ParserTests, splitToStringViewBySemicolon_withTwoNewLineInRow)
{
    constexpr auto line = "line1\n\nline2\nline3";
    const auto result = CppGit::Parser::splitToStringViewsVector(line, '\n');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "line2");
    EXPECT_EQ(result[3], "line3");
}

TEST(ParserTests, splitToStringViewBySemicolon_withSemicolonAtTheEnd)
{
    constexpr auto line = "line1;line2;line3;";
    const auto result = CppGit::Parser::splitToStringViewsVector(line, ';');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "line2");
    EXPECT_EQ(result[2], "line3");
    EXPECT_EQ(result[3], "");
}

TEST(ParserTests, splitToStringViewBySemicolon_withTwoSemicolonInRow)
{
    constexpr auto line = "line1;;line2;line3";
    const auto result = CppGit::Parser::splitToStringViewsVector(line, ';');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "line2");
    EXPECT_EQ(result[3], "line3");
}


TEST(ParserTests, splitToStringByNewLine_withNewLineAtTheEnd)
{
    constexpr auto line = "line1\nline2\nline3\n";
    const auto result = CppGit::Parser::splitToStringsVector(line, '\n');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "line2");
    EXPECT_EQ(result[2], "line3");
    EXPECT_EQ(result[3], "");
}

TEST(ParserTests, splitToStringByNewLine_withoutNewLineAtTheEnd)
{
    constexpr auto line = "line1\nline2\nline3";
    const auto result = CppGit::Parser::splitToStringsVector(line, '\n');

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "line2");
    EXPECT_EQ(result[2], "line3");
}

TEST(ParserTests, splitToStringByNewLine_emptyLineWithNewLine)
{
    constexpr auto line = "\n";
    const auto result = CppGit::Parser::splitToStringsVector(line, '\n');

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "");
    EXPECT_EQ(result[1], "");
}

TEST(ParserTests, splitToStringByNewLine_emptyLineWithoutNewLine)
{
    constexpr auto line = "";
    const auto result = CppGit::Parser::splitToStringsVector(line, '\n');

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "");
}

TEST(ParserTests, splitToStringByNewLine_emptyLineWithTwoNewLines)
{
    constexpr auto line = "\n\n";
    const auto result = CppGit::Parser::splitToStringsVector(line, '\n');

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "");
}

TEST(ParserTests, splitToStringBySemicolon_withTwoNewLineInRow)
{
    constexpr auto line = "line1\n\nline2\nline3";
    const auto result = CppGit::Parser::splitToStringsVector(line, '\n');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "line2");
    EXPECT_EQ(result[3], "line3");
}

TEST(ParserTests, splitToStringBySemicolon_withSemicolonAtTheEnd)
{
    constexpr auto line = "line1;line2;line3;";
    const auto result = CppGit::Parser::splitToStringsVector(line, ';');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "line2");
    EXPECT_EQ(result[2], "line3");
    EXPECT_EQ(result[3], "");
}

TEST(ParserTests, splitToStringBySemicolon_withTwoSemicolonInRow)
{
    constexpr auto line = "line1;;line2;line3";
    const auto result = CppGit::Parser::splitToStringsVector(line, ';');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "line2");
    EXPECT_EQ(result[3], "line3");
}
