#include "Parser/CommitParser.hpp"
#include "Signature.hpp"

#include <gtest/gtest.h>

TEST(CommitParserTests, onlySingleLineMsg)
{
    std::string commit = R"(tree 4b825dc642cb6eb9a060e54bf8d69288fbee4904
author Firstauthor Secondauthor <author@email.com> 1722791195 +0200
committer Firstcommiter Secondcommiter <committer@email.com> 1722791195 +0200

msg msg msg)";

    CppGit::Commit parsedCommit = CppGit::CommitParser::parseCommit(commit);
    CppGit::Signature author = parsedCommit.getAuthor();
    CppGit::Signature committer = parsedCommit.getCommitter();

    EXPECT_EQ(parsedCommit.getTreeHash(), "4b825dc642cb6eb9a060e54bf8d69288fbee4904");
    EXPECT_EQ(author.name, "Firstauthor Secondauthor");
    EXPECT_EQ(author.email, "author@email.com");
    EXPECT_EQ(parsedCommit.getAuthorDate(), "1722791195 +0200");
    EXPECT_EQ(committer.name, "Firstcommiter Secondcommiter");
    EXPECT_EQ(committer.email, "committer@email.com");
    EXPECT_EQ(parsedCommit.getCommitterDate(), "1722791195 +0200");
    EXPECT_EQ(parsedCommit.getMessage(), "msg msg msg");
    EXPECT_EQ(parsedCommit.getDescription(), "");
}

TEST(CommitParserTests, multiLineMsg)
{
    std::string commit = R"(tree 4b825dc642cb6eb9a060e54bf8d69288fbee4904
author Firstauthor Secondauthor <author@email.com> 1722791195 +0200
committer Firstcommiter Secondcommiter <committer@email.com> 1722791195 +0200

msg
msg2)";

    CppGit::Commit parsedCommit = CppGit::CommitParser::parseCommit(commit);
    CppGit::Signature author = parsedCommit.getAuthor();
    CppGit::Signature committer = parsedCommit.getCommitter();

    EXPECT_EQ(parsedCommit.getTreeHash(), "4b825dc642cb6eb9a060e54bf8d69288fbee4904");
    EXPECT_EQ(author.name, "Firstauthor Secondauthor");
    EXPECT_EQ(author.email, "author@email.com");
    EXPECT_EQ(parsedCommit.getAuthorDate(), "1722791195 +0200");
    EXPECT_EQ(committer.name, "Firstcommiter Secondcommiter");
    EXPECT_EQ(committer.email, "committer@email.com");
    EXPECT_EQ(parsedCommit.getCommitterDate(), "1722791195 +0200");
    EXPECT_EQ(parsedCommit.getMessage(), "msg\nmsg2");
    EXPECT_EQ(parsedCommit.getDescription(), "");
}

TEST(CommitParserTests, singleLineMsg_singleLineDesc)
{
    std::string commit = R"(tree 4b825dc642cb6eb9a060e54bf8d69288fbee4904
author Firstauthor Secondauthor <author@email.com> 1722791195 +0200
committer Firstcommiter Secondcommiter <committer@email.com> 1722791195 +0200

msg

desc)";

    CppGit::Commit parsedCommit = CppGit::CommitParser::parseCommit(commit);
    CppGit::Signature author = parsedCommit.getAuthor();
    CppGit::Signature committer = parsedCommit.getCommitter();

    EXPECT_EQ(parsedCommit.getTreeHash(), "4b825dc642cb6eb9a060e54bf8d69288fbee4904");
    EXPECT_EQ(author.name, "Firstauthor Secondauthor");
    EXPECT_EQ(author.email, "author@email.com");
    EXPECT_EQ(parsedCommit.getAuthorDate(), "1722791195 +0200");
    EXPECT_EQ(committer.name, "Firstcommiter Secondcommiter");
    EXPECT_EQ(committer.email, "committer@email.com");
    EXPECT_EQ(parsedCommit.getCommitterDate(), "1722791195 +0200");
    EXPECT_EQ(parsedCommit.getMessage(), "msg");
    EXPECT_EQ(parsedCommit.getDescription(), "desc");
}

TEST(CommitParserTests, multiLineMsg_multiLineDesc)
{
    std::string commit = R"(tree 4b825dc642cb6eb9a060e54bf8d69288fbee4904
author Firstauthor Secondauthor <author@email.com> 1722791195 +0200
committer Firstcommiter Secondcommiter <committer@email.com> 1722791195 +0200

msg
msg2

desc
desc2)";

    CppGit::Commit parsedCommit = CppGit::CommitParser::parseCommit(commit);
    CppGit::Signature author = parsedCommit.getAuthor();
    CppGit::Signature committer = parsedCommit.getCommitter();

    EXPECT_EQ(parsedCommit.getTreeHash(), "4b825dc642cb6eb9a060e54bf8d69288fbee4904");
    EXPECT_EQ(author.name, "Firstauthor Secondauthor");
    EXPECT_EQ(author.email, "author@email.com");
    EXPECT_EQ(parsedCommit.getAuthorDate(), "1722791195 +0200");
    EXPECT_EQ(committer.name, "Firstcommiter Secondcommiter");
    EXPECT_EQ(committer.email, "committer@email.com");
    EXPECT_EQ(parsedCommit.getCommitterDate(), "1722791195 +0200");
    EXPECT_EQ(parsedCommit.getMessage(), "msg\nmsg2");
    EXPECT_EQ(parsedCommit.getDescription(), "desc\ndesc2");
}

TEST(CommitParserTests, withSingleParent)
{
    std::string commit = R"(tree 4b825dc642cb6eb9a060e54bf8d69288fbee4904
parent 8120cca3edbd848e900b41d3d217ca2803dd0e74
author Firstauthor Secondauthor <author@email.com> 1722791195 +0200
committer Firstcommiter Secondcommiter <committer@email.com> 1722791195 +0200

msg msg msg)";

    CppGit::Commit parsedCommit = CppGit::CommitParser::parseCommit(commit);
    CppGit::Signature author = parsedCommit.getAuthor();
    CppGit::Signature committer = parsedCommit.getCommitter();

    EXPECT_EQ(parsedCommit.getTreeHash(), "4b825dc642cb6eb9a060e54bf8d69288fbee4904");
    ASSERT_EQ(parsedCommit.getParents().size(), 1);
    EXPECT_EQ(parsedCommit.getParents()[0], "8120cca3edbd848e900b41d3d217ca2803dd0e74");
    EXPECT_EQ(author.name, "Firstauthor Secondauthor");
    EXPECT_EQ(author.email, "author@email.com");
    EXPECT_EQ(parsedCommit.getAuthorDate(), "1722791195 +0200");
    EXPECT_EQ(committer.name, "Firstcommiter Secondcommiter");
    EXPECT_EQ(committer.email, "committer@email.com");
    EXPECT_EQ(parsedCommit.getCommitterDate(), "1722791195 +0200");
    EXPECT_EQ(parsedCommit.getMessage(), "msg msg msg");
    EXPECT_EQ(parsedCommit.getDescription(), "");
}

TEST(CommitParserTests, withMultiParent)
{
    std::string commit = R"(tree 4b825dc642cb6eb9a060e54bf8d69288fbee4904
parent 8120cca3edbd848e900b41d3d217ca2803dd0e74
parent 8120cca3edbd848e900b41d3d217ca2803dd0e75
author Firstauthor Secondauthor <author@email.com> 1722791195 +0200
committer Firstcommiter Secondcommiter <committer@email.com> 1722791195 +0200

msg msg msg)";

    CppGit::Commit parsedCommit = CppGit::CommitParser::parseCommit(commit);
    CppGit::Signature author = parsedCommit.getAuthor();
    CppGit::Signature committer = parsedCommit.getCommitter();

    EXPECT_EQ(parsedCommit.getTreeHash(), "4b825dc642cb6eb9a060e54bf8d69288fbee4904");
    ASSERT_EQ(parsedCommit.getParents().size(), 2);
    EXPECT_EQ(parsedCommit.getParents()[0], "8120cca3edbd848e900b41d3d217ca2803dd0e74");
    EXPECT_EQ(parsedCommit.getParents()[1], "8120cca3edbd848e900b41d3d217ca2803dd0e75");
    EXPECT_EQ(author.name, "Firstauthor Secondauthor");
    EXPECT_EQ(author.email, "author@email.com");
    EXPECT_EQ(parsedCommit.getAuthorDate(), "1722791195 +0200");
    EXPECT_EQ(committer.name, "Firstcommiter Secondcommiter");
    EXPECT_EQ(committer.email, "committer@email.com");
    EXPECT_EQ(parsedCommit.getCommitterDate(), "1722791195 +0200");
    EXPECT_EQ(parsedCommit.getMessage(), "msg msg msg");
    EXPECT_EQ(parsedCommit.getDescription(), "");
}
