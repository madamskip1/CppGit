#include "Parser/Parser.hpp"

#include <gtest/gtest.h>

TEST(ParserTests, splitByNewLine_withNewLineAtTheEnd)
{
    auto line = "line1\nline2\nline3\n";
    auto result = CppGit::Parser::split(line, '\n');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "line2");
    EXPECT_EQ(result[2], "line3");
    EXPECT_EQ(result[3], "");
}

TEST(ParserTests, splitByNewLine_withoutNewLineAtTheEnd)
{
    auto line = "line1\nline2\nline3";
    auto result = CppGit::Parser::split(line, '\n');

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "line2");
    EXPECT_EQ(result[2], "line3");
}

TEST(ParserTests, splitByNewLine_emptyLineWithNewLine)
{
    auto line = "\n";
    auto result = CppGit::Parser::split(line, '\n');

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "");
    EXPECT_EQ(result[1], "");
}

TEST(ParserTests, splitByNewLine_emptyLineWithoutNewLine)
{
    auto line = "";
    auto result = CppGit::Parser::split(line, '\n');

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "");
}

TEST(ParserTests, splitByNewLine_emptyLineWithTwoNewLines)
{
    auto line = "\n\n";
    auto result = CppGit::Parser::split(line, '\n');

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "");
}

TEST(ParserTests, splitBySemicolon_withTwoNewLineInRow)
{
    auto line = "line1\n\nline2\nline3";
    auto result = CppGit::Parser::split(line, '\n');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "line2");
    EXPECT_EQ(result[3], "line3");
}

TEST(ParserTests, splitBySemicolon_withSemicolonAtTheEnd)
{
    auto line = "line1;line2;line3;";
    auto result = CppGit::Parser::split(line, ';');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "line2");
    EXPECT_EQ(result[2], "line3");
    EXPECT_EQ(result[3], "");
}

TEST(ParserTests, splitBySemicolon_withTwoSemicolonInRow)
{
    auto line = "line1;;line2;line3";
    auto result = CppGit::Parser::split(line, ';');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "line2");
    EXPECT_EQ(result[3], "line3");
}
