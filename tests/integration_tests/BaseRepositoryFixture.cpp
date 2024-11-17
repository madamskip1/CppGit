#include "BaseRepositoryFixture.hpp"

#include <fstream>


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

auto BaseRepositoryFixture::getFileContent(const std::filesystem::path& filePath) -> std::string
{
    auto file = std::ifstream{ filePath };
    std::ostringstream content;
    content << file.rdbuf();

    return content.str();
}
