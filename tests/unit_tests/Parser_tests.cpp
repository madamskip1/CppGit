#include "_details/Parser/Parser.hpp"

#include <gtest/gtest.h>

TEST(ParserTests, splitToStringViewByNewLine_withNewLineAtTheEnd)
{
    auto line = "line1\nline2\nline3\n";
    auto result = CppGit::Parser::splitToStringViewsVector(line, '\n');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "line2");
    EXPECT_EQ(result[2], "line3");
    EXPECT_EQ(result[3], "");
}

TEST(ParserTests, splitToStringViewByNewLine_withoutNewLineAtTheEnd)
{
    auto line = "line1\nline2\nline3";
    auto result = CppGit::Parser::splitToStringViewsVector(line, '\n');

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "line2");
    EXPECT_EQ(result[2], "line3");
}

TEST(ParserTests, splitToStringViewByNewLine_emptyLineWithNewLine)
{
    auto line = "\n";
    auto result = CppGit::Parser::splitToStringViewsVector(line, '\n');

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "");
    EXPECT_EQ(result[1], "");
}

TEST(ParserTests, splitToStringViewByNewLine_emptyLineWithoutNewLine)
{
    auto line = "";
    auto result = CppGit::Parser::splitToStringViewsVector(line, '\n');

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "");
}

TEST(ParserTests, splitToStringViewByNewLine_emptyLineWithTwoNewLines)
{
    auto line = "\n\n";
    auto result = CppGit::Parser::splitToStringViewsVector(line, '\n');

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "");
}

TEST(ParserTests, splitToStringViewBySemicolon_withTwoNewLineInRow)
{
    auto line = "line1\n\nline2\nline3";
    auto result = CppGit::Parser::splitToStringViewsVector(line, '\n');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "line2");
    EXPECT_EQ(result[3], "line3");
}

TEST(ParserTests, splitToStringViewBySemicolon_withSemicolonAtTheEnd)
{
    auto line = "line1;line2;line3;";
    auto result = CppGit::Parser::splitToStringViewsVector(line, ';');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "line2");
    EXPECT_EQ(result[2], "line3");
    EXPECT_EQ(result[3], "");
}

TEST(ParserTests, splitToStringViewBySemicolon_withTwoSemicolonInRow)
{
    auto line = "line1;;line2;line3";
    auto result = CppGit::Parser::splitToStringViewsVector(line, ';');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "line2");
    EXPECT_EQ(result[3], "line3");
}


TEST(ParserTests, splitToStringByNewLine_withNewLineAtTheEnd)
{
    auto line = "line1\nline2\nline3\n";
    auto result = CppGit::Parser::splitToStringsVector(line, '\n');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "line2");
    EXPECT_EQ(result[2], "line3");
    EXPECT_EQ(result[3], "");
}

TEST(ParserTests, splitToStringByNewLine_withoutNewLineAtTheEnd)
{
    auto line = "line1\nline2\nline3";
    auto result = CppGit::Parser::splitToStringsVector(line, '\n');

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "line2");
    EXPECT_EQ(result[2], "line3");
}

TEST(ParserTests, splitToStringByNewLine_emptyLineWithNewLine)
{
    auto line = "\n";
    auto result = CppGit::Parser::splitToStringsVector(line, '\n');

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "");
    EXPECT_EQ(result[1], "");
}

TEST(ParserTests, splitToStringByNewLine_emptyLineWithoutNewLine)
{
    auto line = "";
    auto result = CppGit::Parser::splitToStringsVector(line, '\n');

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "");
}

TEST(ParserTests, splitToStringByNewLine_emptyLineWithTwoNewLines)
{
    auto line = "\n\n";
    auto result = CppGit::Parser::splitToStringsVector(line, '\n');

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "");
}

TEST(ParserTests, splitToStringBySemicolon_withTwoNewLineInRow)
{
    auto line = "line1\n\nline2\nline3";
    auto result = CppGit::Parser::splitToStringsVector(line, '\n');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "line2");
    EXPECT_EQ(result[3], "line3");
}

TEST(ParserTests, splitToStringBySemicolon_withSemicolonAtTheEnd)
{
    auto line = "line1;line2;line3;";
    auto result = CppGit::Parser::splitToStringsVector(line, ';');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "line2");
    EXPECT_EQ(result[2], "line3");
    EXPECT_EQ(result[3], "");
}

TEST(ParserTests, splitToStringBySemicolon_withTwoSemicolonInRow)
{
    auto line = "line1;;line2;line3";
    auto result = CppGit::Parser::splitToStringsVector(line, ';');

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "line1");
    EXPECT_EQ(result[1], "");
    EXPECT_EQ(result[2], "line2");
    EXPECT_EQ(result[3], "line3");
}
