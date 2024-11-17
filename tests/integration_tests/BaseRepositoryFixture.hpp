#include "Repository.hpp"

#include <filesystem>
#include <gtest/gtest.h>
#include <memory>

class BaseRepositoryFixture : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    std::filesystem::path repositoryPath = std::filesystem::current_path() / "integration-tests-repo";
    std::unique_ptr<CppGit::Repository> repository;

    static auto getFileContent(const std::filesystem::path& filePath) -> std::string;
};
