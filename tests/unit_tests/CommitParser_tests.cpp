#include "Parser/CommitParser.hpp"
#include "Signature.hpp"

#include <gtest/gtest.h>

TEST(CommitParserTests, parseCatfile_onlySingleLineMsg)
{
    std::string commit = R"(tree 4b825dc642cb6eb9a060e54bf8d69288fbee4904
author Firstauthor Secondauthor <author@email.com> 1722791195 +0200
committer Firstcommiter Secondcommiter <committer@email.com> 1722791195 +0200

msg msg msg)";

    CppGit::Commit parsedCommit = CppGit::CommitParser::parseCommit_CatFile(commit);
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

TEST(CommitParserTests, parseCatfile_multiLineMsg)
{
    std::string commit = R"(tree 4b825dc642cb6eb9a060e54bf8d69288fbee4904
author Firstauthor Secondauthor <author@email.com> 1722791195 +0200
committer Firstcommiter Secondcommiter <committer@email.com> 1722791195 +0200

msg
msg2)";

    CppGit::Commit parsedCommit = CppGit::CommitParser::parseCommit_CatFile(commit);
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

TEST(CommitParserTests, parseCatfile_singleLineMsg_singleLineDesc)
{
    std::string commit = R"(tree 4b825dc642cb6eb9a060e54bf8d69288fbee4904
author Firstauthor Secondauthor <author@email.com> 1722791195 +0200
committer Firstcommiter Secondcommiter <committer@email.com> 1722791195 +0200

msg

desc)";

    CppGit::Commit parsedCommit = CppGit::CommitParser::parseCommit_CatFile(commit);
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

TEST(CommitParserTests, parseCatfile_multiLineMsg_multiLineDesc)
{
    std::string commit = R"(tree 4b825dc642cb6eb9a060e54bf8d69288fbee4904
author Firstauthor Secondauthor <author@email.com> 1722791195 +0200
committer Firstcommiter Secondcommiter <committer@email.com> 1722791195 +0200

msg
msg2

desc
desc2)";

    CppGit::Commit parsedCommit = CppGit::CommitParser::parseCommit_CatFile(commit);
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

TEST(CommitParserTests, parseCatfile_withSingleParent)
{
    std::string commit = R"(tree 4b825dc642cb6eb9a060e54bf8d69288fbee4904
parent 8120cca3edbd848e900b41d3d217ca2803dd0e74
author Firstauthor Secondauthor <author@email.com> 1722791195 +0200
committer Firstcommiter Secondcommiter <committer@email.com> 1722791195 +0200

msg msg msg)";

    CppGit::Commit parsedCommit = CppGit::CommitParser::parseCommit_CatFile(commit);
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

TEST(CommitParserTests, parseCatfile_withMultiParent)
{
    std::string commit = R"(tree 4b825dc642cb6eb9a060e54bf8d69288fbee4904
parent 8120cca3edbd848e900b41d3d217ca2803dd0e74
parent 8120cca3edbd848e900b41d3d217ca2803dd0e75
author Firstauthor Secondauthor <author@email.com> 1722791195 +0200
committer Firstcommiter Secondcommiter <committer@email.com> 1722791195 +0200

msg msg msg)";

    CppGit::Commit parsedCommit = CppGit::CommitParser::parseCommit_CatFile(commit);
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


TEST(CommitParserTests, parseFormat_defaultFormat_fullCommit)
{
    const std::string commitLog = "hash;parent1 parent2;authorName;authorEmail;authorDate;committerName;committerEmail;committerDate;message;description";
    const CppGit::Commit commit = CppGit::CommitParser::parseCommit_PrettyFormat(commitLog);
    const auto& authorSignature = commit.getAuthor();
    const auto& committerSignature = commit.getCommitter();

    EXPECT_EQ(commit.getHash(), "hash");
    EXPECT_EQ(commit.getParents().size(), 2);
    EXPECT_EQ(commit.getParents()[0], "parent1");
    EXPECT_EQ(commit.getParents()[1], "parent2");
    EXPECT_EQ(authorSignature.name, "authorName");
    EXPECT_EQ(authorSignature.email, "authorEmail");
    EXPECT_EQ(commit.getAuthorDate(), "authorDate");
    EXPECT_EQ(committerSignature.name, "committerName");
    EXPECT_EQ(committerSignature.email, "committerEmail");
    EXPECT_EQ(commit.getCommitterDate(), "committerDate");
    EXPECT_EQ(commit.getMessage(), "message");
    EXPECT_EQ(commit.getDescription(), "description");
}
TEST(CommitParserTests, parseFormat_defaultFormat_fullCommit_multiLineDescription)
{
    const std::string commitLog = "hash;parent1 parent2;authorName;authorEmail;authorDate;committerName;committerEmail;committerDate;message;description\nline1\nline2";
    const CppGit::Commit commit = CppGit::CommitParser::parseCommit_PrettyFormat(commitLog);
    const auto& authorSignature = commit.getAuthor();
    const auto& committerSignature = commit.getCommitter();

    EXPECT_EQ(commit.getHash(), "hash");
    EXPECT_EQ(commit.getParents().size(), 2);
    EXPECT_EQ(commit.getParents()[0], "parent1");
    EXPECT_EQ(commit.getParents()[1], "parent2");
    EXPECT_EQ(authorSignature.name, "authorName");
    EXPECT_EQ(authorSignature.email, "authorEmail");
    EXPECT_EQ(commit.getAuthorDate(), "authorDate");
    EXPECT_EQ(committerSignature.name, "committerName");
    EXPECT_EQ(committerSignature.email, "committerEmail");
    EXPECT_EQ(commit.getCommitterDate(), "committerDate");
    EXPECT_EQ(commit.getMessage(), "message");
    EXPECT_EQ(commit.getDescription(), "description\nline1\nline2");
}

TEST(CommitParserTests, parseFormat_defaultFormat_fullCommit_singleParent)
{
    const std::string commitLog = "hash;parent1;authorName;authorEmail;authorDate;committerName;committerEmail;committerDate;message;description";
    const CppGit::Commit commit = CppGit::CommitParser::parseCommit_PrettyFormat(commitLog);
    const auto& authorSignature = commit.getAuthor();
    const auto& committerSignature = commit.getCommitter();

    EXPECT_EQ(commit.getHash(), "hash");
    EXPECT_EQ(commit.getParents().size(), 1);
    EXPECT_EQ(commit.getParents()[0], "parent1");
    EXPECT_EQ(authorSignature.name, "authorName");
    EXPECT_EQ(authorSignature.email, "authorEmail");
    EXPECT_EQ(commit.getAuthorDate(), "authorDate");
    EXPECT_EQ(committerSignature.name, "committerName");
    EXPECT_EQ(committerSignature.email, "committerEmail");
    EXPECT_EQ(commit.getCommitterDate(), "committerDate");
    EXPECT_EQ(commit.getMessage(), "message");
    EXPECT_EQ(commit.getDescription(), "description");
}

TEST(CommitParserTests, parseFormat_defaultFormat_noParents)
{
    const std::string commitLog = "hash;;authorName;authorEmail;authorDate;committerName;committerEmail;committerDate;message;description";
    const CppGit::Commit commit = CppGit::CommitParser::parseCommit_PrettyFormat(commitLog);
    const auto& authorSignature = commit.getAuthor();
    const auto& committerSignature = commit.getCommitter();

    EXPECT_EQ(commit.getHash(), "hash");
    EXPECT_EQ(commit.getParents().size(), 0);
    EXPECT_EQ(authorSignature.name, "authorName");
    EXPECT_EQ(authorSignature.email, "authorEmail");
    EXPECT_EQ(commit.getAuthorDate(), "authorDate");
    EXPECT_EQ(committerSignature.name, "committerName");
    EXPECT_EQ(committerSignature.email, "committerEmail");
    EXPECT_EQ(commit.getCommitterDate(), "committerDate");
    EXPECT_EQ(commit.getMessage(), "message");
    EXPECT_EQ(commit.getDescription(), "description");
}

TEST(CommitParserTests, parseFormat_defaultFormat_noDescription)
{
    const std::string commitLog = "hash;parent1 parent2;authorName;authorEmail;authorDate;committerName;committerEmail;committerDate;message;";
    const CppGit::Commit commit = CppGit::CommitParser::parseCommit_PrettyFormat(commitLog);
    const auto& authorSignature = commit.getAuthor();
    const auto& committerSignature = commit.getCommitter();

    EXPECT_EQ(commit.getHash(), "hash");
    EXPECT_EQ(commit.getParents().size(), 2);
    EXPECT_EQ(commit.getParents()[0], "parent1");
    EXPECT_EQ(commit.getParents()[1], "parent2");
    EXPECT_EQ(authorSignature.name, "authorName");
    EXPECT_EQ(authorSignature.email, "authorEmail");
    EXPECT_EQ(commit.getAuthorDate(), "authorDate");
    EXPECT_EQ(committerSignature.name, "committerName");
    EXPECT_EQ(committerSignature.email, "committerEmail");
    EXPECT_EQ(commit.getCommitterDate(), "committerDate");
    EXPECT_EQ(commit.getMessage(), "message");
    EXPECT_EQ(commit.getDescription(), "");
}

TEST(CommitParserTests, parseFormat_defaultFormat_emptyCommit)
{
    const std::string commitLog = ";;;;;;;;;";
    const CppGit::Commit commit = CppGit::CommitParser::parseCommit_PrettyFormat(commitLog);
    const auto& authorSignature = commit.getAuthor();
    const auto& committerSignature = commit.getCommitter();

    EXPECT_EQ(commit.getHash(), "");
    EXPECT_EQ(commit.getParents().size(), 0);
    EXPECT_EQ(authorSignature.name, "");
    EXPECT_EQ(authorSignature.email, "");
    EXPECT_EQ(commit.getAuthorDate(), "");
    EXPECT_EQ(committerSignature.name, "");
    EXPECT_EQ(committerSignature.email, "");
    EXPECT_EQ(commit.getCommitterDate(), "");
    EXPECT_EQ(commit.getMessage(), "");
    EXPECT_EQ(commit.getDescription(), "");
}

TEST(CommitParserTests, parseFormat_customFormat)
{
    const std::string commitLog = "ad34f5b|message";
    const std::string format = "%h|%s";
    const auto* const delimiter = "|";
    const CppGit::Commit commit = CppGit::CommitParser::parseCommit_PrettyFormat(commitLog, format, delimiter);
    const auto& authorSignature = commit.getAuthor();
    const auto& committerSignature = commit.getCommitter();

    EXPECT_EQ(commit.getHash(), "ad34f5b");
    EXPECT_EQ(commit.getParents().size(), 0);
    EXPECT_EQ(authorSignature.name, "");
    EXPECT_EQ(authorSignature.email, "");
    EXPECT_EQ(commit.getAuthorDate(), "");
    EXPECT_EQ(committerSignature.name, "");
    EXPECT_EQ(committerSignature.email, "");
    EXPECT_EQ(commit.getCommitterDate(), "");
    EXPECT_EQ(commit.getMessage(), "message");
    EXPECT_EQ(commit.getDescription(), "");
}
