#include "Commit.hpp"
#include "Repository.hpp"

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

    static auto createOrOverwriteFile(const std::filesystem::path& filePath, const std::string_view content) -> void;
    static auto getFileContent(const std::filesystem::path& filePath) -> std::string;


    static constexpr auto* const AUTHOR_NAME = "TestAuthor";
    static constexpr auto* const AUTHOR_EMAIL = "test@email.com";
    static constexpr auto* const AUTHOR_DATE = "1730738278 +0100";

    static auto prepareCommitAuthorCommiterTestEnvp() -> std::vector<std::string>;

    static auto checkCommitAuthorEqualTest(const CppGit::Commit& commit) -> void;
    static auto checkCommitCommiterNotEqualTest(const CppGit::Commit& commit) -> void;
};
