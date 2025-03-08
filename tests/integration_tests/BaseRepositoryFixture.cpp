#include "BaseRepositoryFixture.hpp"

#include <CppGit/_details/CreateCommit.hpp>
#include <CppGit/_details/Refs.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

void BaseRepositoryFixture::SetUp()
{
    repository = std::make_unique<CppGit::Repository>(repositoryPath);
    ASSERT_TRUE(repository->initRepository());
    ASSERT_TRUE(repository->isValidGitRepository());
}

void BaseRepositoryFixture::TearDown()
{
    std::filesystem::remove_all(repositoryPath);
    ASSERT_FALSE(repository->isValidGitRepository());
}

auto BaseRepositoryFixture::prepareCommitAuthorCommiterTestEnvp() -> std::vector<std::string>
{
    const auto envp = std::vector<std::string>{ std::string{ "GIT_AUTHOR_NAME=" } + AUTHOR_NAME, std::string{ "GIT_AUTHOR_EMAIL=" } + AUTHOR_EMAIL, std::string{ "GIT_AUTHOR_DATE=" } + AUTHOR_DATE, std::string{ "GIT_COMMITTER_NAME=" } + AUTHOR_NAME, std::string{ "GIT_COMMITTER_EMAIL=" } + AUTHOR_EMAIL, std::string{ "GIT_COMMITTER_DATE=" } + AUTHOR_DATE };

    return envp;
}

auto BaseRepositoryFixture::createCommitWithTestAuthorCommiter(const std::string_view& message, const std::string_view description, std::string parentHash) const -> std::string
{
    const auto newCommitHash = CppGit::_details::CreateCommit{ *repository }.createCommit(message, description, { std::move(parentHash) }, prepareCommitAuthorCommiterTestEnvp());
    CppGit::_details::Refs{ *repository }.updateRefHash("HEAD", newCommitHash);

    return newCommitHash;
}

auto BaseRepositoryFixture::createCommitWithTestAuthorCommiter(const std::string_view& message, std::string parentHash) const -> std::string
{
    return createCommitWithTestAuthorCommiter(message, "", std::move(parentHash));
}

auto BaseRepositoryFixture::createCommitWithTestAuthorCommiterWithoutParent(const std::string_view& message, const std::string_view description) const -> std::string
{
    return createCommitWithTestAuthorCommiter(message, description, "");
}

auto BaseRepositoryFixture::checkCommitAuthorEqualTest(const CppGit::Commit& commit) -> void
{
    EXPECT_EQ(commit.getAuthor().name, AUTHOR_NAME);
    EXPECT_EQ(commit.getAuthor().email, AUTHOR_EMAIL);
    EXPECT_EQ(commit.getAuthorDate(), AUTHOR_DATE);
}

auto BaseRepositoryFixture::checkCommitAuthorNotEqualTest(const CppGit::Commit& commit) -> void
{
    EXPECT_NE(commit.getAuthor().name, AUTHOR_NAME);
    EXPECT_NE(commit.getAuthor().email, AUTHOR_EMAIL);
    EXPECT_NE(commit.getAuthorDate(), AUTHOR_DATE);
}

auto BaseRepositoryFixture::checkCommitCommiterEqualTest(const CppGit::Commit& commit) -> void
{
    EXPECT_EQ(commit.getCommitter().name, AUTHOR_NAME);
    EXPECT_EQ(commit.getCommitter().email, AUTHOR_EMAIL);
    EXPECT_EQ(commit.getCommitterDate(), AUTHOR_DATE);
}

auto BaseRepositoryFixture::checkCommitCommiterNotEqualTest(const CppGit::Commit& commit) -> void
{
    EXPECT_NE(commit.getCommitter().name, AUTHOR_NAME);
    EXPECT_NE(commit.getCommitter().email, AUTHOR_EMAIL);
    EXPECT_NE(commit.getCommitterDate(), AUTHOR_DATE);
}

auto BaseRepositoryFixture::checkTestAuthorAndCommiterPreserved(const CppGit::Commit& commit) -> void
{
    checkCommitAuthorEqualTest(commit);
    checkCommitCommiterEqualTest(commit);
}

auto BaseRepositoryFixture::checkTestAuthorPreservedCommitterModified(const CppGit::Commit& commit) -> void
{
    checkCommitAuthorEqualTest(commit);
    checkCommitCommiterNotEqualTest(commit);
}

auto BaseRepositoryFixture::checkTestAuthorAndCommiterModdified(const CppGit::Commit& commit) -> void
{
    checkCommitAuthorNotEqualTest(commit);
    checkCommitCommiterNotEqualTest(commit);
}
