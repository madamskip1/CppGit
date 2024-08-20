#include "BaseRepositoryFixture.hpp"
#include "Commit.hpp"
#include "CommitsHistory.hpp"

#include <gtest/gtest.h>

class CommitsHistoryTests : public BaseRepositoryFixture
{
public:
    void SetUp() override
    {
        BaseRepositoryFixture::SetUp();
        const auto& commits = repository->Commits();
        auto commit1Hash = commits.createCommit("Commit1");
        auto commit2Hash = commits.createCommit("Commit2");
        auto commit3Hash = commits.createCommit("Commit3");
        auto commit41Hash = commits.createCommit("Commit41", "Description41");
        auto commit42Hash = commits.createCommit("Commit42", "Description42");

        commitsHashes = { commit1Hash, commit2Hash, commit3Hash, commit41Hash, commit42Hash };
    }

protected:
    std::vector<std::string> commitsHashes;
};

TEST_F(CommitsHistoryTests, getCommitsLogHashesOnly_NoFilters)
{
    const auto& commitsHistory = repository->CommitsHistory();
    const auto& commitsLogHashesOnly = commitsHistory.getCommitsLogHashesOnly();

    ASSERT_EQ(commitsLogHashesOnly.size(), 5);
    EXPECT_EQ(commitsLogHashesOnly[0], commitsHashes[4]);
    EXPECT_EQ(commitsLogHashesOnly[1], commitsHashes[3]);
    EXPECT_EQ(commitsLogHashesOnly[2], commitsHashes[2]);
    EXPECT_EQ(commitsLogHashesOnly[3], commitsHashes[1]);
    EXPECT_EQ(commitsLogHashesOnly[4], commitsHashes[0]);
}

TEST_F(CommitsHistoryTests, getCommitsLogHashesOnly_MaxCount)
{
    const auto& commitsHistory = repository->CommitsHistory().setMaxCount(3);
    const auto& commitsLogHashesOnly = commitsHistory.getCommitsLogHashesOnly();

    ASSERT_EQ(commitsLogHashesOnly.size(), 3);
    EXPECT_EQ(commitsLogHashesOnly[0], commitsHashes[4]);
    EXPECT_EQ(commitsLogHashesOnly[1], commitsHashes[3]);
    EXPECT_EQ(commitsLogHashesOnly[2], commitsHashes[2]);
}

TEST_F(CommitsHistoryTests, getCommitsLogHashesOnly_Skip)
{
    const auto& commitsHistory = repository->CommitsHistory().setSkip(2);
    const auto& commitsLogHashesOnly = commitsHistory.getCommitsLogHashesOnly();

    ASSERT_EQ(commitsLogHashesOnly.size(), 3);
    EXPECT_EQ(commitsLogHashesOnly[0], commitsHashes[2]);
    EXPECT_EQ(commitsLogHashesOnly[1], commitsHashes[1]);
    EXPECT_EQ(commitsLogHashesOnly[2], commitsHashes[0]);
}

TEST_F(CommitsHistoryTests, getCommitsLogHashesOnly_MaxCountSkip)
{
    const auto& commitsHistory = repository->CommitsHistory().setMaxCount(2).setSkip(1);
    const auto& commitsLogHashesOnly = commitsHistory.getCommitsLogHashesOnly();

    ASSERT_EQ(commitsLogHashesOnly.size(), 2);
    EXPECT_EQ(commitsLogHashesOnly[0], commitsHashes[3]);
    EXPECT_EQ(commitsLogHashesOnly[1], commitsHashes[2]);
}

TEST_F(CommitsHistoryTests, getCommitsLogHashesOnly_ReverseOrder)
{
    const auto& commitsHistory = repository->CommitsHistory().setOrder(CppGit::CommitsHistory::Order::REVERSE);
    const auto& commitsLogHashesOnly = commitsHistory.getCommitsLogHashesOnly();

    ASSERT_EQ(commitsLogHashesOnly.size(), 5);
    EXPECT_EQ(commitsLogHashesOnly[0], commitsHashes[0]);
    EXPECT_EQ(commitsLogHashesOnly[1], commitsHashes[1]);
    EXPECT_EQ(commitsLogHashesOnly[2], commitsHashes[2]);
    EXPECT_EQ(commitsLogHashesOnly[3], commitsHashes[3]);
    EXPECT_EQ(commitsLogHashesOnly[4], commitsHashes[4]);
}

TEST_F(CommitsHistoryTests, getCommitsLogHashesOnly_DateOrder)
{
    const auto& commitsHistory = repository->CommitsHistory().setOrder(CppGit::CommitsHistory::Order::DATE);
    const auto& commitsLogHashesOnly = commitsHistory.getCommitsLogHashesOnly();

    ASSERT_EQ(commitsLogHashesOnly.size(), 5);
    EXPECT_EQ(commitsLogHashesOnly[0], commitsHashes[4]);
    EXPECT_EQ(commitsLogHashesOnly[1], commitsHashes[3]);
    EXPECT_EQ(commitsLogHashesOnly[2], commitsHashes[2]);
    EXPECT_EQ(commitsLogHashesOnly[3], commitsHashes[1]);
    EXPECT_EQ(commitsLogHashesOnly[4], commitsHashes[0]);
}

TEST_F(CommitsHistoryTests, getCommitsLogHashesOnly_messagePattern)
{
    const auto& commitsHistory = repository->CommitsHistory().setMessagePattern("Commit4");
    const auto& commitsLogHashesOnly = commitsHistory.getCommitsLogHashesOnly();

    ASSERT_EQ(commitsLogHashesOnly.size(), 2);
    EXPECT_EQ(commitsLogHashesOnly[0], commitsHashes[4]);
    EXPECT_EQ(commitsLogHashesOnly[1], commitsHashes[3]);
}

TEST_F(CommitsHistoryTests, getCommitsDetailed_NoFilters)
{
    const auto& commitsHistory = repository->CommitsHistory();
    const auto& commitsDetailed = commitsHistory.getCommitsLogDetailed();

    ASSERT_EQ(commitsDetailed.size(), 5);

    EXPECT_EQ(commitsDetailed[0].getHash(), commitsHashes[4]);
    EXPECT_EQ(commitsDetailed[0].getMessage(), "Commit42");
    EXPECT_EQ(commitsDetailed[0].getDescription(), "Description42");
    EXPECT_EQ(commitsDetailed[0].getParents(), std::vector<std::string>{ commitsHashes[3] });

    EXPECT_EQ(commitsDetailed[1].getHash(), commitsHashes[3]);
    EXPECT_EQ(commitsDetailed[1].getMessage(), "Commit41");
    EXPECT_EQ(commitsDetailed[1].getDescription(), "Description41");
    EXPECT_EQ(commitsDetailed[1].getParents(), std::vector<std::string>{ commitsHashes[2] });

    EXPECT_EQ(commitsDetailed[2].getHash(), commitsHashes[2]);
    EXPECT_EQ(commitsDetailed[2].getMessage(), "Commit3");
    EXPECT_EQ(commitsDetailed[2].getDescription(), "");
    EXPECT_EQ(commitsDetailed[2].getParents(), std::vector<std::string>{ commitsHashes[1] });

    EXPECT_EQ(commitsDetailed[3].getHash(), commitsHashes[1]);
    EXPECT_EQ(commitsDetailed[3].getMessage(), "Commit2");
    EXPECT_EQ(commitsDetailed[3].getDescription(), "");
    EXPECT_EQ(commitsDetailed[3].getParents(), std::vector<std::string>{ commitsHashes[0] });

    EXPECT_EQ(commitsDetailed[4].getHash(), commitsHashes[0]);
    EXPECT_EQ(commitsDetailed[4].getMessage(), "Commit1");
    EXPECT_EQ(commitsDetailed[4].getDescription(), "");
    EXPECT_EQ(commitsDetailed[4].getParents(), std::vector<std::string>{});
}
