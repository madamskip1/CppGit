#include <CppGit/_details/Parser/BranchesParser.hpp>
#include <gtest/gtest.h>

TEST(BranchesParser, isLocalBranch_heads)
{
    EXPECT_TRUE(CppGit::BranchesParser::isLocalBranch("refs/heads/main"));
}

TEST(BranchesParser, isLocalBranch_remotes)
{
    EXPECT_FALSE(CppGit::BranchesParser::isLocalBranch("refs/remotes/origin/main"));
}

TEST(BranchesParser, isLocalBranch_tags)
{
    EXPECT_FALSE(CppGit::BranchesParser::isLocalBranch("refs/tags/v1.0"));
}

TEST(BranchesParser, parseBranch_simpleLocalBranch)
{
    const auto branch = CppGit::BranchesParser::parseBranch("refs/heads/main;;");

    EXPECT_EQ(branch.getRefName(), "refs/heads/main");
    EXPECT_EQ(branch.getUpstreamPull(), "");
    EXPECT_EQ(branch.getUpstreamPush(), "");
    EXPECT_TRUE(branch.isLocalBranch());
}

TEST(BranchesParser, parseBranch_localBranchWithUpstream)
{
    const auto branch = CppGit::BranchesParser::parseBranch("refs/heads/main;refs/remotes/origin/main;");

    EXPECT_EQ(branch.getRefName(), "refs/heads/main");
    EXPECT_EQ(branch.getUpstreamPull(), "refs/remotes/origin/main");
    EXPECT_EQ(branch.getUpstreamPush(), "");
    EXPECT_TRUE(branch.isLocalBranch());
}

TEST(BranchesParser, parseBranch_localBranchWithPush)
{
    const auto branch = CppGit::BranchesParser::parseBranch("refs/heads/main;;refs/remotes/origin/main");

    EXPECT_EQ(branch.getRefName(), "refs/heads/main");
    EXPECT_EQ(branch.getUpstreamPull(), "");
    EXPECT_EQ(branch.getUpstreamPush(), "refs/remotes/origin/main");
    EXPECT_TRUE(branch.isLocalBranch());
}

TEST(BranchesParser, parseBranch_localBranchWithUpstreamAndPush)
{
    const auto branch = CppGit::BranchesParser::parseBranch("refs/heads/main;refs/remotes/origin/main;refs/remotes/origin/main;");

    EXPECT_EQ(branch.getRefName(), "refs/heads/main");
    EXPECT_EQ(branch.getUpstreamPull(), "refs/remotes/origin/main");
    EXPECT_EQ(branch.getUpstreamPush(), "refs/remotes/origin/main");
    EXPECT_TRUE(branch.isLocalBranch());
}

TEST(BranchesParser, parseBranch_simpleRemoteBranch)
{
    const auto branch = CppGit::BranchesParser::parseBranch("refs/remotes/origin/main;;");

    EXPECT_EQ(branch.getRefName(), "refs/remotes/origin/main");
    EXPECT_EQ(branch.getUpstreamPull(), "");
    EXPECT_EQ(branch.getUpstreamPush(), "");
    EXPECT_FALSE(branch.isLocalBranch());
}

TEST(BranchesParser, parseBranch_remoteHead)
{
    const auto branch = CppGit::BranchesParser::parseBranch("refs/remotes/origin/HEAD;;");

    EXPECT_EQ(branch.getRefName(), "refs/remotes/origin/HEAD");
    EXPECT_EQ(branch.getUpstreamPull(), "");
    EXPECT_EQ(branch.getUpstreamPush(), "");
    EXPECT_FALSE(branch.isLocalBranch());
}
