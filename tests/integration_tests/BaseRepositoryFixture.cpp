#include "BaseRepositoryFixture.hpp"

#include "_details/CreateCommit.hpp"


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
    auto envp = std::vector<std::string>{ std::string{ "GIT_AUTHOR_NAME=" } + AUTHOR_NAME, std::string{ "GIT_AUTHOR_EMAIL=" } + AUTHOR_EMAIL, std::string{ "GIT_AUTHOR_DATE=" } + AUTHOR_DATE, std::string{ "GIT_COMMITTER_NAME=" } + AUTHOR_NAME, std::string{ "GIT_COMMITTER_EMAIL=" } + AUTHOR_EMAIL, std::string{ "GIT_COMMITTER_DATE=" } + AUTHOR_DATE };

    return envp;
}

auto BaseRepositoryFixture::createCommitWithTestAuthorCommiter(const std::string_view& message, const std::string_view description, std::string parentHash) const -> std::string
{
    return CppGit::_details::CreateCommit{ *repository }.createCommit(message, description, { std::move(parentHash) }, prepareCommitAuthorCommiterTestEnvp());
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
