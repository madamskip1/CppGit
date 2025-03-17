#include "BaseRepositoryFixture.hpp"

#include <CppGit/Commits.hpp>
#include <CppGit/CommitsLog.hpp>
#include <gtest/gtest.h>

class CommitsLogTests : public BaseRepositoryFixture
{
public:
    void SetUp() override
    {
        BaseRepositoryFixture::SetUp();
        auto commits = repository->Commits();
        auto commit1Hash = commits.createCommit("Commit1");
        auto commit2Hash = commits.createCommit("Commit2");
        auto commit3Hash = commits.createCommit("Commit3");
        auto commit41Hash = commits.createCommit("Commit41", "Description41");
        auto commit42Hash = commits.createCommit("Commit42", "Description42");

        commitsHashes = { std::move(commit1Hash), std::move(commit2Hash), std::move(commit3Hash), std::move(commit41Hash), std::move(commit42Hash) };
    }

protected:
    std::vector<std::string> commitsHashes;
};

TEST_F(CommitsLogTests, getCommitsLogHashesOnly_NoFilters)
{
    const auto commitsLog = repository->CommitsLog();
    const auto log = commitsLog.getCommitsLogHashesOnly();

    ASSERT_EQ(log.size(), 5);
    EXPECT_EQ(log[0], commitsHashes[4]);
    EXPECT_EQ(log[1], commitsHashes[3]);
    EXPECT_EQ(log[2], commitsHashes[2]);
    EXPECT_EQ(log[3], commitsHashes[1]);
    EXPECT_EQ(log[4], commitsHashes[0]);
}

TEST_F(CommitsLogTests, getCommitsLogHashesOnly_MaxCount)
{
    const auto commitsLog = repository->CommitsLog().setMaxCount(3);
    const auto log = commitsLog.getCommitsLogHashesOnly();

    ASSERT_EQ(log.size(), 3);
    EXPECT_EQ(log[0], commitsHashes[4]);
    EXPECT_EQ(log[1], commitsHashes[3]);
    EXPECT_EQ(log[2], commitsHashes[2]);
}

TEST_F(CommitsLogTests, getCommitsLogHashesOnly_Skip)
{
    const auto commitsLog = repository->CommitsLog().setSkip(2);
    const auto log = commitsLog.getCommitsLogHashesOnly();

    ASSERT_EQ(log.size(), 3);
    EXPECT_EQ(log[0], commitsHashes[2]);
    EXPECT_EQ(log[1], commitsHashes[1]);
    EXPECT_EQ(log[2], commitsHashes[0]);
}

TEST_F(CommitsLogTests, getCommitsLogHashesOnly_MaxCountSkip)
{
    const auto commitsLog = repository->CommitsLog().setMaxCount(2).setSkip(1);
    const auto log = commitsLog.getCommitsLogHashesOnly();

    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0], commitsHashes[3]);
    EXPECT_EQ(log[1], commitsHashes[2]);
}

TEST_F(CommitsLogTests, getCommitsLogHashesOnly_ReverseOrder)
{
    const auto commitsLog = repository->CommitsLog().setOrder(CppGit::CommitsLog::Order::REVERSE);
    const auto log = commitsLog.getCommitsLogHashesOnly();

    ASSERT_EQ(log.size(), 5);
    EXPECT_EQ(log[0], commitsHashes[0]);
    EXPECT_EQ(log[1], commitsHashes[1]);
    EXPECT_EQ(log[2], commitsHashes[2]);
    EXPECT_EQ(log[3], commitsHashes[3]);
    EXPECT_EQ(log[4], commitsHashes[4]);
}

TEST_F(CommitsLogTests, getCommitsLogHashesOnly_DateOrder)
{
    const auto commitsLog = repository->CommitsLog().setOrder(CppGit::CommitsLog::Order::DATE);
    const auto log = commitsLog.getCommitsLogHashesOnly();

    ASSERT_EQ(log.size(), 5);
    EXPECT_EQ(log[0], commitsHashes[4]);
    EXPECT_EQ(log[1], commitsHashes[3]);
    EXPECT_EQ(log[2], commitsHashes[2]);
    EXPECT_EQ(log[3], commitsHashes[1]);
    EXPECT_EQ(log[4], commitsHashes[0]);
}

TEST_F(CommitsLogTests, getCommitsLogHashesOnly_messagePattern)
{
    const auto commitsLog = repository->CommitsLog().setMessagePattern("Commit4");
    const auto log = commitsLog.getCommitsLogHashesOnly();

    ASSERT_EQ(log.size(), 2);
    EXPECT_EQ(log[0], commitsHashes[4]);
    EXPECT_EQ(log[1], commitsHashes[3]);
}

TEST_F(CommitsLogTests, getlog_NoFilters)
{
    const auto commitsLog = repository->CommitsLog();
    const auto log = commitsLog.getCommitsLogDetailed();

    ASSERT_EQ(log.size(), 5);

    EXPECT_EQ(log[0].getHash(), commitsHashes[4]);
    EXPECT_EQ(log[0].getMessage(), "Commit42");
    EXPECT_EQ(log[0].getDescription(), "Description42");
    EXPECT_EQ(log[0].getParents(), std::vector<std::string>{ commitsHashes[3] });

    EXPECT_EQ(log[1].getHash(), commitsHashes[3]);
    EXPECT_EQ(log[1].getMessage(), "Commit41");
    EXPECT_EQ(log[1].getDescription(), "Description41");
    EXPECT_EQ(log[1].getParents(), std::vector<std::string>{ commitsHashes[2] });

    EXPECT_EQ(log[2].getHash(), commitsHashes[2]);
    EXPECT_EQ(log[2].getMessage(), "Commit3");
    EXPECT_EQ(log[2].getDescription(), "");
    EXPECT_EQ(log[2].getParents(), std::vector<std::string>{ commitsHashes[1] });

    EXPECT_EQ(log[3].getHash(), commitsHashes[1]);
    EXPECT_EQ(log[3].getMessage(), "Commit2");
    EXPECT_EQ(log[3].getDescription(), "");
    EXPECT_EQ(log[3].getParents(), std::vector<std::string>{ commitsHashes[0] });

    EXPECT_EQ(log[4].getHash(), commitsHashes[0]);
    EXPECT_EQ(log[4].getMessage(), "Commit1");
    EXPECT_EQ(log[4].getDescription(), "");
    EXPECT_EQ(log[4].getParents(), std::vector<std::string>{});
}

TEST_F(CommitsLogTests, getlog_ToRef)
{
    const auto commitsLog = repository->CommitsLog();
    const auto log = commitsLog.getCommitsLogDetailed(commitsHashes[2]);

    ASSERT_EQ(log.size(), 3);

    EXPECT_EQ(log[0].getHash(), commitsHashes[2]);
    EXPECT_EQ(log[0].getMessage(), "Commit3");
    EXPECT_EQ(log[0].getDescription(), "");
    EXPECT_EQ(log[0].getParents(), std::vector<std::string>{ commitsHashes[1] });

    EXPECT_EQ(log[1].getHash(), commitsHashes[1]);
    EXPECT_EQ(log[1].getMessage(), "Commit2");
    EXPECT_EQ(log[1].getDescription(), "");
    EXPECT_EQ(log[1].getParents(), std::vector<std::string>{ commitsHashes[0] });

    EXPECT_EQ(log[2].getHash(), commitsHashes[0]);
    EXPECT_EQ(log[2].getMessage(), "Commit1");
    EXPECT_EQ(log[2].getDescription(), "");
    EXPECT_EQ(log[2].getParents(), std::vector<std::string>{});
}

TEST_F(CommitsLogTests, getlog_FromRefToRef)
{
    const auto commitsLog = repository->CommitsLog();
    const auto log = commitsLog.getCommitsLogDetailed(commitsHashes[1], commitsHashes[2]);

    ASSERT_EQ(log.size(), 1);

    EXPECT_EQ(log[0].getHash(), commitsHashes[2]);
    EXPECT_EQ(log[0].getMessage(), "Commit3");
    EXPECT_EQ(log[0].getDescription(), "");
    EXPECT_EQ(log[0].getParents(), std::vector<std::string>{ commitsHashes[1] });
}

TEST_F(CommitsLogTests, getCommitsLogHashesOnly_ToRef)
{
    const auto commitsLog = repository->CommitsLog();
    const auto log = commitsLog.getCommitsLogHashesOnly(commitsHashes[2]);

    ASSERT_EQ(log.size(), 3);
    EXPECT_EQ(log[0], commitsHashes[2]);
    EXPECT_EQ(log[1], commitsHashes[1]);
    EXPECT_EQ(log[2], commitsHashes[0]);
}

TEST_F(CommitsLogTests, getCommitsLogHashesOnly_FromRefToRef)
{
    const auto commitsLog = repository->CommitsLog();
    const auto log = commitsLog.getCommitsLogHashesOnly(commitsHashes[1], commitsHashes[2]);

    ASSERT_EQ(log.size(), 1);
    EXPECT_EQ(log[0], commitsHashes[2]);
}
