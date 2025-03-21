# CppGit

`CppGit` is a library for working with `GIT Repository` inside your `C++ Code`. From adding files to index to performing no-fast-forward merges on your branches. The library uses `plumbing` git methods, making it resistant to porcelain git API changes. 

##### Unix build status: [![Build g++](https://github.com/madamskip1/CppGit/actions/workflows/build_ubuntu_gcc.yaml/badge.svg)](https://github.com/madamskip1/CppGit/actions/workflows/build_ubuntu_gcc.yaml)

```c++
#include <CppGit/Repository.hpp>

int main()
{
	const auto repository = CppGit::Repository{"my_repo"};
	repository.IndexManager().add("file.txt");
	repository.CommitsManager().commit("My Commit");
}
```

## Motivation - Why?

CppGit was personal project created with intention to use it later in GUI app that will help with rebase. It was one reason.
The second one was knowledge. I wanted to make side-project that will improve my C++ skills, but also to learn how Git works under hood (and i did - it gave me a greater understanding of Git and now i sometimes teach my co-workers about certains git operations when they have any problems with git).


## Features

#### Code features

|Feature| Status  |
|--|--|
| Initialize repository | ✔️ |
| Initialize bare repository | ✔️ |
| Add files to index (staging area) | ✔️ |
| Remove files from index | ✔️ |
| List files in index or staging area | ✔️ |
| List tracked and untracked files | ✔️ |
| Create commit | ✔️ |
| Amend commit | ✔️ |
| Get information about given commit | ✔️ |
| Get HEAD commit hash | ✔️ |
| Get current branch | ✔️ |
| List all branches | ✔️ |
| Switch branch | ✔️ |
| Delete branch | ✔️ |
| Detach Head | ✔️ |
| Check if repository is dirty | ✔️ |
| Generate last diff | ✔️ |
| Generate diff between commits | ✔️ |
| Commits log | ✔️ |
| Soft reset | ✔️ |
| Hard reset | ✔️ |
| Merge Fast-Forward | ✔️ |
| Merge No-Fast-Forward | ✔️ |
| Handle merge conflicts | ✔️ |
| Rebase | ✔️ |
| Rebase interactive: Reword | ✔️ |
| Rebase interactive: Edit | ✔️ |
| Rebase interactive: Squash | ✔️ |
| Rebase interactive: Fixup | ✔️ |
| Rebase interactive: Break | ✔️ |
| Rebase interactive: Drop | ✔️ |
| Tags |  |
| Rebase with merge commits |  |
| Rebase interactive: Exec |  |
| Rebase interactive: Label |  |
| Rebase interactive: Reset |  |
| Rebase interactive: Merge |  |
| Rebase interactive: Update ref |  |
| Supports remote repositories |  |


#### System features

| Feature | Status |
|--|--|
| Unix support | ✔️ |
| Windows support |  |
| Thread-safety |  |

# Error-prone

`CppGit` is not "git-error-prone". By this, I mean the library doesn't check correctness of the given input, the state of repository etc. Maintaining proper data and state is left to developers integrating the library into their projects.

#### Why?

I assumed that the library would mainly be used in controlled conditions in the code. Therefore, exceptions and error codes are not needed then and would only slow down the code and increase the binary size. If error checking were required (for example, when taking input from the end user), the programmer using the library should take care of checking the correctness of the data and the state of the repository.

I decided to only leave error codes for cherry-pick, merge and rebase; as  they are inseparable elements of the logic. For example - information that merge conflict occured. 


# Usage/Example

> **_NOTE_**: Full API documentation is available at [CppGit API Documentation](https://madamskip1.github.io/CppGit/).

I will guide through some normal work with git:.
- Create `develop` branch.
- Create commits.
- Rebase onto `main` (assuming work has been done on `main`).
- Merge `develop` branch into `main` with no-fast-forward merge.

Full example code is available at [Example](./examples/readme-example.cpp).

#### Create `CppGit::Repository` instance

`CppGit::Repository` it is the primary class from where it all starts.

```c++
const auto repository = CppGit::Repository{ "~/my_repo" };
```

If there is not repository in path `~/my_repo_` yet, we can intialize git repositoriy with:

```c++
repository.initRepository(); // Will init git repo with `main` as default branch
```

Otherwise we can go ahead.

#### Create initial commit

Git repository is not working properly without any commits. So add `README.md` to staging area and create commit.
(I assume there is `README.md` file already).

```c++
const auto indexManager = repository.IndexManager();
indexManager.add("README.md");
const auto commitsManager = repository.CommitsManager();
commitsManager.createCommit("Add README.md");;
```

#### Create `develop` and do some development

##### Create `develop` branch and checkout onto


```c++
const auto branchesManager = repository.BranchesManager();
// We can use either shorter `develop` or longer branch name `refs/heads/develop`
branchesManager.createBranch("develop");
branchesManager.changeBranch("develop");

branchesManager.getCurrentBranchName(); // Will return `refs/heads/develop`
```

##### Do some works and create commits

_Note_: Now, I assume there are few created files (we focus on working with git, not files).

```c++
indexManager.add("file.txt");
commitsManager.createCommit("MVP");
[...]
indexManager.add("file1.txt");
indexManager.add("README.md");
commitsManager.createCommit("Add super uber important feature");
```

##### Rebase onto main
_Note_: Now, I assume there has been some work done on `main` branch.

```c++
repository.Rebaser().rebase("main");
```

##### Merge No-FF develop into main

```c++
branchesManager.changeBranch("main");
repository.Merger().mergeNoFastForward("develop", "Merge develop into main");
```

##### Delete develop branch

```c++
branchesManager.deleteBranch("develop");
```