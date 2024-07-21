#include <gtest/gtest.h>
#include "Parser/CommitParser.hpp"
#include "Signature.hpp"

TEST(CommitParserTests, defaultFormat_fullCommit)
{
    const std::string commitLog = "hash;parent1 parent2;authorName;authorEmail;authorDate;committerName;committerEmail;committerDate;message;description";
    const CppGit::Commit commit = CppGit::CommitParser::parseCommit(commitLog);
    auto authorSignature = commit.getAuthor();
    auto committerSignature = commit.getCommitter();

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
TEST(CommitParserTests, defaultFormat_fullCommit_multiLineDescription)
{
    const std::string commitLog = "hash;parent1 parent2;authorName;authorEmail;authorDate;committerName;committerEmail;committerDate;message;description\nline1\nline2";
    const CppGit::Commit commit = CppGit::CommitParser::parseCommit(commitLog);
    auto authorSignature = commit.getAuthor();
    auto committerSignature = commit.getCommitter();

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

TEST(CommitParserTests, defaultFormat_fullCommit_singleParent)
{
    const std::string commitLog = "hash;parent1;authorName;authorEmail;authorDate;committerName;committerEmail;committerDate;message;description";
    const CppGit::Commit commit = CppGit::CommitParser::parseCommit(commitLog);
    auto authorSignature = commit.getAuthor();
    auto committerSignature = commit.getCommitter();

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

TEST(CommitParserTests, defaultFormat_noParents)
{
    const std::string commitLog = "hash;;authorName;authorEmail;authorDate;committerName;committerEmail;committerDate;message;description";
    const CppGit::Commit commit = CppGit::CommitParser::parseCommit(commitLog);
    auto authorSignature = commit.getAuthor();
    auto committerSignature = commit.getCommitter();

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

TEST(CommitParserTests, defaultFormat_noDescription)
{
    const std::string commitLog = "hash;parent1 parent2;authorName;authorEmail;authorDate;committerName;committerEmail;committerDate;message;";
    const CppGit::Commit commit = CppGit::CommitParser::parseCommit(commitLog);
    auto authorSignature = commit.getAuthor();
    auto committerSignature = commit.getCommitter();

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

TEST(CommitParserTests, defaultFormat_emptyCommit)
{
    const std::string commitLog = ";;;;;;;;;";
    const CppGit::Commit commit = CppGit::CommitParser::parseCommit(commitLog);
    auto authorSignature = commit.getAuthor();
    auto committerSignature = commit.getCommitter();

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

TEST(CommitParserTests, customFormat)
{
    const std::string commitLog = "ad34f5b|message";
    const std::string format = "%h|%s";
    const char delimiter = '|';
    const CppGit::Commit commit = CppGit::CommitParser::parseCommit(commitLog, format, delimiter);
    auto authorSignature = commit.getAuthor();
    auto committerSignature = commit.getCommitter();

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