#include <CppGit/_details/Parser/DiffParser.hpp>
#include <gtest/gtest.h>

TEST(DiffParserTests, fileAdded)
{
    constexpr auto diff = R"(diff --git a/new_file.txt b/new_file.txt
new file mode 100644
index 0000000..180cf8328022becee9aaa2577a8f84ea2b9f3827
--- /dev/null
+++ b/new_file.txt
@@ -0,0 +1,4 @@
+Line 1
+Line 2
+Line 3
+Line 4)";

    auto diffParser = CppGit::DiffParser{};
    const auto diffFiles = diffParser.parse(diff);

    ASSERT_EQ(diffFiles.size(), 1);

    const auto& diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile.isCombined, false);
    ASSERT_EQ(diffFile.indicesBefore.size(), 1);
    EXPECT_EQ(diffFile.indicesBefore[0], "0000000");
    EXPECT_EQ(diffFile.indexAfter, "180cf8328022becee9aaa2577a8f84ea2b9f3827");
    EXPECT_EQ(diffFile.oldMode, 0);
    EXPECT_EQ(diffFile.newMode, 100'644);
    EXPECT_EQ(diffFile.similarityIndex, 0);
    EXPECT_EQ(diffFile.fileA, "/dev/null");
    EXPECT_EQ(diffFile.fileB, "new_file.txt");
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 1);
    EXPECT_EQ(diffFile.hunkRangesBefore[0].first, 0);
    EXPECT_EQ(diffFile.hunkRangesBefore[0].second, 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.first, 1);
    EXPECT_EQ(diffFile.hunkRangeAfter.second, 4);
    ASSERT_EQ(diffFile.hunkContent.size(), 4);
    EXPECT_EQ(diffFile.hunkContent[0], "+Line 1");
    EXPECT_EQ(diffFile.hunkContent[1], "+Line 2");
    EXPECT_EQ(diffFile.hunkContent[2], "+Line 3");
    EXPECT_EQ(diffFile.hunkContent[3], "+Line 4");
}

TEST(DiffParserTests, fileAddedWithoutContext)
{
    constexpr auto diff = R"(diff --git a/test4.txt b/test4.txt
new file mode 100644
index 0000000000000000000000000000000000000000..e69de29bb2d1d6434b8b29ae775ad8c2e48c5391)";

    auto diffParser = CppGit::DiffParser{};
    const auto diffFiles = diffParser.parse(diff);

    ASSERT_EQ(diffFiles.size(), 1);

    const auto& diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile.isCombined, false);
    ASSERT_EQ(diffFile.indicesBefore.size(), 1);
    EXPECT_EQ(diffFile.indicesBefore[0], "0000000000000000000000000000000000000000");
    EXPECT_EQ(diffFile.indexAfter, "e69de29bb2d1d6434b8b29ae775ad8c2e48c5391");
    EXPECT_EQ(diffFile.oldMode, 0);
    EXPECT_EQ(diffFile.newMode, 100'644);
    EXPECT_EQ(diffFile.similarityIndex, 0);
    EXPECT_EQ(diffFile.fileA, "/dev/null");
    EXPECT_EQ(diffFile.fileB, "test4.txt");
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.first, 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.second, 0);
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}


TEST(DiffParserTests, fileDeleted)
{
    constexpr auto diff = R"(diff --git a/deleted_file.txt b/deleted_file.txt
deleted file mode 100644
index 180cf8328022becee9aaa2577a8f84ea2b9f3827..0000000
--- a/deleted_file.txt
+++ /dev/null
@@ -1,4 +0,0 @@
-Line 1
-Line 2
-Line 3
-Line 4)";

    auto diffParser = CppGit::DiffParser{};
    const auto diffFiles = diffParser.parse(diff);

    ASSERT_EQ(diffFiles.size(), 1);

    const auto& diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::DELETED);
    EXPECT_EQ(diffFile.isCombined, false);
    ASSERT_EQ(diffFile.indicesBefore.size(), 1);
    EXPECT_EQ(diffFile.indicesBefore[0], "180cf8328022becee9aaa2577a8f84ea2b9f3827");
    EXPECT_EQ(diffFile.indexAfter, "0000000");
    EXPECT_EQ(diffFile.oldMode, 100'644);
    EXPECT_EQ(diffFile.newMode, 0);
    EXPECT_EQ(diffFile.similarityIndex, 0);
    EXPECT_EQ(diffFile.fileA, "deleted_file.txt");
    EXPECT_EQ(diffFile.fileB, "/dev/null");
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 1);
    EXPECT_EQ(diffFile.hunkRangesBefore[0].first, 1);
    EXPECT_EQ(diffFile.hunkRangesBefore[0].second, 4);
    EXPECT_EQ(diffFile.hunkRangeAfter.first, 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.second, 0);
    ASSERT_EQ(diffFile.hunkContent.size(), 4);
    EXPECT_EQ(diffFile.hunkContent[0], "-Line 1");
    EXPECT_EQ(diffFile.hunkContent[1], "-Line 2");
    EXPECT_EQ(diffFile.hunkContent[2], "-Line 3");
    EXPECT_EQ(diffFile.hunkContent[3], "-Line 4");
}

TEST(DiffPraserTests, fileDeletedWithoutContext)
{
    constexpr auto diff = R"(diff --git a/test4.txt b/test4.txt
deleted file mode 100644
index e69de29bb2d1d6434b8b29ae775ad8c2e48c5391..0000000000000000000000000000000000000000)";

    auto diffParser = CppGit::DiffParser{};
    const auto diffFiles = diffParser.parse(diff);

    ASSERT_EQ(diffFiles.size(), 1);

    const auto& diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::DELETED);
    EXPECT_EQ(diffFile.isCombined, false);
    ASSERT_EQ(diffFile.indicesBefore.size(), 1);
    EXPECT_EQ(diffFile.indicesBefore[0], "e69de29bb2d1d6434b8b29ae775ad8c2e48c5391");
    EXPECT_EQ(diffFile.indexAfter, "0000000000000000000000000000000000000000");
    EXPECT_EQ(diffFile.oldMode, 100'644);
    EXPECT_EQ(diffFile.newMode, 0);
    EXPECT_EQ(diffFile.similarityIndex, 0);
    EXPECT_EQ(diffFile.fileA, "test4.txt");
    EXPECT_EQ(diffFile.fileB, "/dev/null");
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.first, 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.second, 0);
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}

TEST(DiffParserTests, fileModified)
{
    constexpr auto diff = R"(diff --git a/modified_file.txt b/modified_file.txt
index 180cf8328022becee9aaa2577a8f84ea2b9f3827..1d89a1850b82787e2766aa3c724048fc74ea4fbc 100644
--- a/modified_file.txt
+++ b/modified_file.txt
@@ -1,4 +1,4 @@
 Line 1
 Line 2
-Line 3
+Modified Line 3
 Line 4)";

    auto diffParser = CppGit::DiffParser{};
    const auto diffFiles = diffParser.parse(diff);

    ASSERT_EQ(diffFiles.size(), 1);

    const auto& diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::MODDIFIED);
    EXPECT_EQ(diffFile.isCombined, false);
    ASSERT_EQ(diffFile.indicesBefore.size(), 1);
    EXPECT_EQ(diffFile.indicesBefore[0], "180cf8328022becee9aaa2577a8f84ea2b9f3827");
    EXPECT_EQ(diffFile.indexAfter, "1d89a1850b82787e2766aa3c724048fc74ea4fbc");
    EXPECT_EQ(diffFile.oldMode, 0);
    EXPECT_EQ(diffFile.newMode, 100'644);
    EXPECT_EQ(diffFile.similarityIndex, 0);
    EXPECT_EQ(diffFile.fileA, "modified_file.txt");
    EXPECT_EQ(diffFile.fileB, "modified_file.txt");
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 1);
    EXPECT_EQ(diffFile.hunkRangesBefore[0].first, 1);
    EXPECT_EQ(diffFile.hunkRangesBefore[0].second, 4);
    EXPECT_EQ(diffFile.hunkRangeAfter.first, 1);
    EXPECT_EQ(diffFile.hunkRangeAfter.second, 4);
    ASSERT_EQ(diffFile.hunkContent.size(), 5);
    EXPECT_EQ(diffFile.hunkContent[0], " Line 1");
    EXPECT_EQ(diffFile.hunkContent[1], " Line 2");
    EXPECT_EQ(diffFile.hunkContent[2], "-Line 3");
    EXPECT_EQ(diffFile.hunkContent[3], "+Modified Line 3");
    EXPECT_EQ(diffFile.hunkContent[4], " Line 4");
}


TEST(DiffParserTests, fileRenamed)
{
    constexpr auto diff = R"(diff --git a/old_name.txt b/new_name.txt
similarity index 90%
rename from old_name.txt
rename to new_name.txt)";

    auto diffParser = CppGit::DiffParser{};
    const auto diffFiles = diffParser.parse(diff);

    ASSERT_EQ(diffFiles.size(), 1);

    const auto& diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::RENAMED);
    EXPECT_EQ(diffFile.isCombined, false);
    ASSERT_EQ(diffFile.indicesBefore.size(), 0);
    EXPECT_EQ(diffFile.indexAfter, "");
    EXPECT_EQ(diffFile.oldMode, 0);
    EXPECT_EQ(diffFile.newMode, 0);
    EXPECT_EQ(diffFile.similarityIndex, 90);
    EXPECT_EQ(diffFile.fileA, "old_name.txt");
    EXPECT_EQ(diffFile.fileB, "new_name.txt");
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.first, 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.second, 0);
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}

TEST(DiffParserTests, fileRenamedWithContentChanged)
{
    constexpr auto diff = R"(diff --git a/old_name.txt b/new_name.txt
similarity index 90%
rename from old_name.txt
rename to new_name.txt
index 180cf8328022becee9aaa2577a8f84ea2b9f3827..1d89a1850b82787e2766aa3c724048fc74ea4fbc 100644
--- a/old_name.txt
+++ b/new_name.txt
@@ -1,4 +1,4 @@
 Line 1
 Line 2
-Line 3
+Modified Line 3
 Line 4)";

    auto diffParser = CppGit::DiffParser{};
    const auto diffFiles = diffParser.parse(diff);

    ASSERT_EQ(diffFiles.size(), 1);

    const auto& diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::RENAMED_AND_MODIFIED);
    EXPECT_EQ(diffFile.isCombined, false);
    ASSERT_EQ(diffFile.indicesBefore.size(), 1);
    EXPECT_EQ(diffFile.indicesBefore[0], "180cf8328022becee9aaa2577a8f84ea2b9f3827");
    EXPECT_EQ(diffFile.indexAfter, "1d89a1850b82787e2766aa3c724048fc74ea4fbc");
    EXPECT_EQ(diffFile.oldMode, 0);
    EXPECT_EQ(diffFile.newMode, 100'644);
    EXPECT_EQ(diffFile.similarityIndex, 90);
    EXPECT_EQ(diffFile.fileA, "old_name.txt");
    EXPECT_EQ(diffFile.fileB, "new_name.txt");
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 1);
    EXPECT_EQ(diffFile.hunkRangesBefore[0].first, 1);
    EXPECT_EQ(diffFile.hunkRangesBefore[0].second, 4);
    EXPECT_EQ(diffFile.hunkRangeAfter.first, 1);
    EXPECT_EQ(diffFile.hunkRangeAfter.second, 4);
    ASSERT_EQ(diffFile.hunkContent.size(), 5);
    EXPECT_EQ(diffFile.hunkContent[0], " Line 1");
    EXPECT_EQ(diffFile.hunkContent[1], " Line 2");
    EXPECT_EQ(diffFile.hunkContent[2], "-Line 3");
    EXPECT_EQ(diffFile.hunkContent[3], "+Modified Line 3");
    EXPECT_EQ(diffFile.hunkContent[4], " Line 4");
}

TEST(DiffParserTests, fileCopied)
{
    constexpr auto diff = R"(diff --git a/original.txt b/copied_file.txt
similarity index 100%
copy from original.txt
copy to copied_file.txt)";

    auto diffParser = CppGit::DiffParser{};
    const auto diffFiles = diffParser.parse(diff);

    ASSERT_EQ(diffFiles.size(), 1);

    const auto& diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::COPIED);
    EXPECT_EQ(diffFile.isCombined, false);
    ASSERT_EQ(diffFile.indicesBefore.size(), 0);
    EXPECT_EQ(diffFile.indexAfter, "");
    EXPECT_EQ(diffFile.oldMode, 0);
    EXPECT_EQ(diffFile.newMode, 0);
    EXPECT_EQ(diffFile.similarityIndex, 100);
    EXPECT_EQ(diffFile.fileA, "original.txt");
    EXPECT_EQ(diffFile.fileB, "copied_file.txt");
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.first, 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.second, 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.first, 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.second, 0);
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}

TEST(DiffParserTests, fileCopiedWithContentChanged)
{
    constexpr auto diff = R"(diff --git a/original.txt b/copied_file.txt
similarity index 90%
copy from original.txt
copy to copied_file.txt
index 180cf8328022becee9aaa2577a8f84ea2b9f3827..1d89a1850b82787e2766aa3c724048fc74ea4fbc 100644
--- a/original.txt
+++ b/copied_file.txt
@@ -1,4 +1,4 @@
 Line 1
 Line 2
-Line 3
+Modified Line 3
 Line 4)";

    auto diffParser = CppGit::DiffParser{};
    const auto diffFiles = diffParser.parse(diff);

    ASSERT_EQ(diffFiles.size(), 1);

    const auto& diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::COPIED_AND_MODIFIED);
    EXPECT_EQ(diffFile.isCombined, false);
    ASSERT_EQ(diffFile.indicesBefore.size(), 1);
    EXPECT_EQ(diffFile.indicesBefore[0], "180cf8328022becee9aaa2577a8f84ea2b9f3827");
    EXPECT_EQ(diffFile.indexAfter, "1d89a1850b82787e2766aa3c724048fc74ea4fbc");
    EXPECT_EQ(diffFile.oldMode, 0);
    EXPECT_EQ(diffFile.newMode, 100'644);
    EXPECT_EQ(diffFile.similarityIndex, 90);
    EXPECT_EQ(diffFile.fileA, "original.txt");
    EXPECT_EQ(diffFile.fileB, "copied_file.txt");
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 1);
    EXPECT_EQ(diffFile.hunkRangesBefore[0].first, 1);
    EXPECT_EQ(diffFile.hunkRangesBefore[0].second, 4);
    EXPECT_EQ(diffFile.hunkRangeAfter.first, 1);
    EXPECT_EQ(diffFile.hunkRangeAfter.second, 4);
    ASSERT_EQ(diffFile.hunkContent.size(), 5);
    EXPECT_EQ(diffFile.hunkContent[0], " Line 1");
    EXPECT_EQ(diffFile.hunkContent[1], " Line 2");
    EXPECT_EQ(diffFile.hunkContent[2], "-Line 3");
    EXPECT_EQ(diffFile.hunkContent[3], "+Modified Line 3");
    EXPECT_EQ(diffFile.hunkContent[4], " Line 4");
}

TEST(DiffParserTests, fileTypeChanged)
{
    constexpr auto diff = R"(diff --git a/file.txt b/file.txt
old mode 100644
new mode 100755)";

    auto diffParser = CppGit::DiffParser{};
    const auto diffFiles = diffParser.parse(diff);

    ASSERT_EQ(diffFiles.size(), 1);

    const auto& diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::TYPE_CHANGED);
    EXPECT_EQ(diffFile.isCombined, false);
    ASSERT_EQ(diffFile.indicesBefore.size(), 0);
    EXPECT_EQ(diffFile.indexAfter, "");
    EXPECT_EQ(diffFile.oldMode, 100'644);
    EXPECT_EQ(diffFile.newMode, 100'755);
    EXPECT_EQ(diffFile.similarityIndex, 0);
    EXPECT_EQ(diffFile.fileA, "file.txt");
    EXPECT_EQ(diffFile.fileB, "file.txt");
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.first, 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.second, 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.first, 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.second, 0);
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}

TEST(DiffParserTests, fileTypeChangedSymlink)
{
    constexpr auto diff = R"(diff --git a/file.txt b/file.txt
old mode 100644
new mode 120000
index 180cf8328022becee9aaa2577a8f84ea2b9f3827..1d89a1850b82787e2766aa3c724048fc74ea4fbc
--- a/file.txt
+++ b/file.txt
@@ -1 +1 @@
-File content
+/path/to/symlink)";

    auto diffParser = CppGit::DiffParser{};
    const auto diffFiles = diffParser.parse(diff);

    ASSERT_EQ(diffFiles.size(), 1);

    const auto& diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::TYPE_CHANGED_SYMLINK);
    EXPECT_EQ(diffFile.isCombined, false);
    ASSERT_EQ(diffFile.indicesBefore.size(), 1);
    EXPECT_EQ(diffFile.indicesBefore[0], "180cf8328022becee9aaa2577a8f84ea2b9f3827");
    EXPECT_EQ(diffFile.indexAfter, "1d89a1850b82787e2766aa3c724048fc74ea4fbc");
    EXPECT_EQ(diffFile.oldMode, 100'644);
    EXPECT_EQ(diffFile.newMode, 120'000);
    EXPECT_EQ(diffFile.similarityIndex, 0);
    EXPECT_EQ(diffFile.fileA, "file.txt");
    EXPECT_EQ(diffFile.fileB, "file.txt");
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 1);
    EXPECT_EQ(diffFile.hunkRangesBefore[0].first, 1);
    EXPECT_EQ(diffFile.hunkRangesBefore[0].second, -1);
    EXPECT_EQ(diffFile.hunkRangeAfter.first, 1);
    EXPECT_EQ(diffFile.hunkRangeAfter.second, -1);
    ASSERT_EQ(diffFile.hunkContent.size(), 2);
    EXPECT_EQ(diffFile.hunkContent[0], "-File content");
    EXPECT_EQ(diffFile.hunkContent[1], "+/path/to/symlink");
}


TEST(DiffParserTests, moreParentsModified)
{
    constexpr auto diff = R"(diff --cc file1.txt
index 6fefc78134f4d22c90a778f555c4137feded408e,1d89a1850b82787e2766aa3c724048fc74ea4fbc..180cf8328022becee9aaa2577a8f84ea2b9f3827
--- a/file1.txt
+++ b/file1.txt
@@@ -1,4 -1,4 +1,8 @@@
  Line 1
++Added in parent 2
 +Added in both parents
 -Line 2
+Modified in parent 1
  Line 3
  Line 4)";

    auto diffParser = CppGit::DiffParser{};
    const auto diffFiles = diffParser.parse(diff);

    ASSERT_EQ(diffFiles.size(), 1);

    const auto& diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::MODDIFIED);
    EXPECT_EQ(diffFile.isCombined, true);
    ASSERT_EQ(diffFile.indicesBefore.size(), 2);
    EXPECT_EQ(diffFile.indicesBefore[0], "6fefc78134f4d22c90a778f555c4137feded408e");
    EXPECT_EQ(diffFile.indicesBefore[1], "1d89a1850b82787e2766aa3c724048fc74ea4fbc");
    EXPECT_EQ(diffFile.indexAfter, "180cf8328022becee9aaa2577a8f84ea2b9f3827");
    EXPECT_EQ(diffFile.oldMode, 0);
    EXPECT_EQ(diffFile.newMode, 0);
    EXPECT_EQ(diffFile.similarityIndex, 0);
    EXPECT_EQ(diffFile.fileA, "file1.txt");
    EXPECT_EQ(diffFile.fileB, "file1.txt");
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 2);
    EXPECT_EQ(diffFile.hunkRangesBefore[0].first, 1);
    EXPECT_EQ(diffFile.hunkRangesBefore[0].second, 4);
    EXPECT_EQ(diffFile.hunkRangesBefore[1].first, 1);
    EXPECT_EQ(diffFile.hunkRangesBefore[1].second, 4);
    EXPECT_EQ(diffFile.hunkRangeAfter.first, 1);
    EXPECT_EQ(diffFile.hunkRangeAfter.second, 8);
    ASSERT_EQ(diffFile.hunkContent.size(), 7);
    EXPECT_EQ(diffFile.hunkContent[0], "  Line 1");
    EXPECT_EQ(diffFile.hunkContent[1], "++Added in parent 2");
    EXPECT_EQ(diffFile.hunkContent[2], " +Added in both parents");
    EXPECT_EQ(diffFile.hunkContent[3], " -Line 2");
    EXPECT_EQ(diffFile.hunkContent[4], "+Modified in parent 1");
    EXPECT_EQ(diffFile.hunkContent[5], "  Line 3");
    EXPECT_EQ(diffFile.hunkContent[6], "  Line 4");
}

TEST(DiffParserTests, multipleFiles)
{
    constexpr auto diff = R"(diff --git a/test.txt b/test.txt
index 3406f99e1a810bac7e2f890286787350ce2601d3..6fefc78134f4d22c90a778f555c4137feded408e 100644
--- a/test.txt
+++ b/test.txt
@@ -1,3 +1,4 @@
 test
 content
 test
+__added
diff --git a/test2.txt b/test2.txt
new file mode 100644
index 0000000000000000000000000000000000000000..180cf8328022becee9aaa2577a8f84ea2b9f3827
--- /dev/null
+++ b/test2.txt
@@ -0,0 +1 @@
+test2
diff --git a/test3.txt b/test3.txt
new file mode 100644
index 0000000000000000000000000000000000000000..1d89a1850b82787e2766aa3c724048fc74ea4fbc
--- /dev/null
+++ b/test3.txt
@@ -0,0 +1 @@
+test3_content)";

    auto diffParser = CppGit::DiffParser{};
    const auto diffFiles = diffParser.parse(diff);

    ASSERT_EQ(diffFiles.size(), 3);

    const auto& diffFile1 = diffFiles[0];

    EXPECT_EQ(diffFile1.diffStatus, CppGit::DiffStatus::MODDIFIED);
    EXPECT_EQ(diffFile1.isCombined, false);
    ASSERT_EQ(diffFile1.indicesBefore.size(), 1);
    EXPECT_EQ(diffFile1.indicesBefore[0], "3406f99e1a810bac7e2f890286787350ce2601d3");
    EXPECT_EQ(diffFile1.indexAfter, "6fefc78134f4d22c90a778f555c4137feded408e");
    EXPECT_EQ(diffFile1.oldMode, 0);
    EXPECT_EQ(diffFile1.newMode, 100'644);
    EXPECT_EQ(diffFile1.similarityIndex, 0);
    EXPECT_EQ(diffFile1.fileA, "test.txt");
    EXPECT_EQ(diffFile1.fileB, "test.txt");
    ASSERT_EQ(diffFile1.hunkRangesBefore.size(), 1);
    EXPECT_EQ(diffFile1.hunkRangesBefore[0].first, 1);
    EXPECT_EQ(diffFile1.hunkRangesBefore[0].second, 3);
    EXPECT_EQ(diffFile1.hunkRangeAfter.first, 1);
    EXPECT_EQ(diffFile1.hunkRangeAfter.second, 4);
    ASSERT_EQ(diffFile1.hunkContent.size(), 4);
    EXPECT_EQ(diffFile1.hunkContent[0], " test");
    EXPECT_EQ(diffFile1.hunkContent[1], " content");
    EXPECT_EQ(diffFile1.hunkContent[2], " test");
    EXPECT_EQ(diffFile1.hunkContent[3], "+__added");

    const auto& diffFile2 = diffFiles[1];

    EXPECT_EQ(diffFile2.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile2.isCombined, false);
    ASSERT_EQ(diffFile2.indicesBefore.size(), 1);
    EXPECT_EQ(diffFile2.indicesBefore[0], "0000000000000000000000000000000000000000");
    EXPECT_EQ(diffFile2.indexAfter, "180cf8328022becee9aaa2577a8f84ea2b9f3827");
    EXPECT_EQ(diffFile2.oldMode, 0);
    EXPECT_EQ(diffFile2.newMode, 100'644);
    EXPECT_EQ(diffFile2.similarityIndex, 0);
    EXPECT_EQ(diffFile2.fileA, "/dev/null");
    EXPECT_EQ(diffFile2.fileB, "test2.txt");
    ASSERT_EQ(diffFile2.hunkRangesBefore.size(), 1);
    EXPECT_EQ(diffFile2.hunkRangesBefore[0].first, 0);
    EXPECT_EQ(diffFile2.hunkRangesBefore[0].second, 0);
    EXPECT_EQ(diffFile2.hunkRangeAfter.first, 1);
    EXPECT_EQ(diffFile2.hunkRangeAfter.second, -1);
    ASSERT_EQ(diffFile2.hunkContent.size(), 1);
    EXPECT_EQ(diffFile2.hunkContent[0], "+test2");

    const auto& diffFile3 = diffFiles[2];

    EXPECT_EQ(diffFile3.diffStatus, CppGit::DiffStatus::NEW);
    EXPECT_EQ(diffFile3.isCombined, false);
    ASSERT_EQ(diffFile3.indicesBefore.size(), 1);
    EXPECT_EQ(diffFile3.indicesBefore[0], "0000000000000000000000000000000000000000");
    EXPECT_EQ(diffFile3.indexAfter, "1d89a1850b82787e2766aa3c724048fc74ea4fbc");
    EXPECT_EQ(diffFile3.oldMode, 0);
    EXPECT_EQ(diffFile3.newMode, 100'644);
    EXPECT_EQ(diffFile3.similarityIndex, 0);
    EXPECT_EQ(diffFile3.fileA, "/dev/null");
    EXPECT_EQ(diffFile3.fileB, "test3.txt");
    ASSERT_EQ(diffFile3.hunkRangesBefore.size(), 1);
    EXPECT_EQ(diffFile3.hunkRangesBefore[0].first, 0);
    EXPECT_EQ(diffFile3.hunkRangesBefore[0].second, 0);
    EXPECT_EQ(diffFile3.hunkRangeAfter.first, 1);
    EXPECT_EQ(diffFile3.hunkRangeAfter.second, -1);
    ASSERT_EQ(diffFile3.hunkContent.size(), 1);
    EXPECT_EQ(diffFile3.hunkContent[0], "+test3_content");
}

TEST(DiffParserTests, binaryFileChanged)
{
    constexpr auto diff = R"(diff --git a/image.png b/image.png
index 180cf8328022becee9aaa2577a8f84ea2b9f3827..1d89a1850b82787e2766aa3c724048fc74ea4fbc
Binary files differ)";

    auto diffParser = CppGit::DiffParser{};
    const auto diffFiles = diffParser.parse(diff);

    ASSERT_EQ(diffFiles.size(), 1);

    const auto& diffFile = diffFiles[0];

    EXPECT_EQ(diffFile.diffStatus, CppGit::DiffStatus::BINARY_CHANGED);
    EXPECT_EQ(diffFile.isCombined, false);
    ASSERT_EQ(diffFile.indicesBefore.size(), 1);
    EXPECT_EQ(diffFile.indicesBefore[0], "180cf8328022becee9aaa2577a8f84ea2b9f3827");
    EXPECT_EQ(diffFile.indexAfter, "1d89a1850b82787e2766aa3c724048fc74ea4fbc");
    EXPECT_EQ(diffFile.oldMode, 0);
    EXPECT_EQ(diffFile.newMode, 0);
    EXPECT_EQ(diffFile.similarityIndex, 0);
    EXPECT_EQ(diffFile.fileA, "image.png");
    EXPECT_EQ(diffFile.fileB, "image.png");
    ASSERT_EQ(diffFile.hunkRangesBefore.size(), 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.first, 0);
    EXPECT_EQ(diffFile.hunkRangeAfter.second, 0);
    ASSERT_EQ(diffFile.hunkContent.size(), 0);
}

TEST(DiffParserTests, emptyDiff)
{
    constexpr auto diff = "";

    auto diffParser = CppGit::DiffParser{};
    const auto diffFiles = diffParser.parse(diff);

    ASSERT_EQ(diffFiles.size(), 0);
}
