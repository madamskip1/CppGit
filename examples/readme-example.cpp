#include <filesystem>

#include <CppGit/BranchesManager.hpp>
#include <CppGit/CommitsManager.hpp>
#include <CppGit/IndexManager.hpp>
#include <CppGit/Merger.hpp>
#include <CppGit/Rebaser.hpp>
#include <CppGit/Repository.hpp>
#include <CppGit/_details/FileUtility.hpp> // Note: _details::FileUtility shouldn't be used by library users, used only to simplify the example

int main()
{
    const auto my_repo_path = std::filesystem::current_path() / "my_repo";

    const auto repository = CppGit::Repository{ my_repo_path };
    repository.initRepository();

    CppGit::_details::FileUtility::createOrOverwriteFile(my_repo_path / "README.md", "My project.");

    const auto indexManager = repository.IndexManager();
    indexManager.add("README.md");
    const auto commitsManager = repository.CommitsManager();
    commitsManager.createCommit("Add README.md");

    const auto branchesManager = repository.BranchesManager();
    branchesManager.createBranch("develop");
    branchesManager.changeBranch("develop");

    CppGit::_details::FileUtility::createOrOverwriteFile(my_repo_path / "file.txt");
    indexManager.add("file.txt");
    commitsManager.createCommit("MVP");

    CppGit::_details::FileUtility::createOrAppendFile(my_repo_path / "file.txt", "Super uber important feature.");
    CppGit::_details::FileUtility::createOrAppendFile(my_repo_path / "README.md", "My super uber important project.");
    indexManager.add("file1.txt");
    indexManager.add("README.md");
    commitsManager.createCommit("Add super uber important feature");

    branchesManager.changeBranch("main");
    CppGit::_details::FileUtility::createOrAppendFile(my_repo_path / "main.txt", "Main branch");
    indexManager.add("main.txt");
    commitsManager.createCommit("Main branch commit");

    branchesManager.changeBranch("develop");
    repository.Rebaser().rebase("main");

    branchesManager.changeBranch("main");
    repository.Merger().mergeNoFastForward("develop", "Merge develop into main");

    branchesManager.deleteBranch("develop");

    return 0;
}
