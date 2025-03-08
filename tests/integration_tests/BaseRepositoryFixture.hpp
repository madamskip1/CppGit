#include "CppGit/Commit.hpp"
#include "CppGit/Repository.hpp"

#include <filesystem>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <string_view>

class BaseRepositoryFixture : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    std::filesystem::path repositoryPath = std::filesystem::current_path() / "integration-tests-repo";
    std::unique_ptr<CppGit::Repository> repository;

    static constexpr auto* const AUTHOR_NAME = "TestAuthor";
    static constexpr auto* const AUTHOR_EMAIL = "test@email.com";
    static constexpr auto* const AUTHOR_DATE = "1730738278 +0100";

    auto createCommitWithTestAuthorCommiter(const std::string_view& message, const std::string_view description, std::string parentHash) const -> std::string;
    auto createCommitWithTestAuthorCommiter(const std::string_view& message, std::string parentHash) const -> std::string;
    auto createCommitWithTestAuthorCommiterWithoutParent(const std::string_view& message, const std::string_view description = "") const -> std::string;

    static auto checkCommitAuthorEqualTest(const CppGit::Commit& commit) -> void;
    static auto checkCommitAuthorNotEqualTest(const CppGit::Commit& commit) -> void;
    static auto checkCommitCommiterEqualTest(const CppGit::Commit& commit) -> void;
    static auto checkCommitCommiterNotEqualTest(const CppGit::Commit& commit) -> void;
    static auto checkTestAuthorAndCommiterPreserved(const CppGit::Commit& commit) -> void;
    static auto checkTestAuthorPreservedCommitterModified(const CppGit::Commit& commit) -> void;
    static auto checkTestAuthorAndCommiterModdified(const CppGit::Commit& commit) -> void;

private:
    static auto prepareCommitAuthorCommiterTestEnvp() -> std::vector<std::string>;
};
