cmake_minimum_required(VERSION 3.20)

set(CPPGIT_MASTER_PROJECT OFF)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(CPPGIT_MASTER_PROJECT ON)
endif()

if (CPPGIT_MASTER_PROJECT)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

if (CPPGIT_MASTER_PROJECT)
    option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
    option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

    if (ENABLE_ASAN)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
        set(ENV{ASAN_OPTIONS} "detect_leaks=1")
    endif()

    if (ENABLE_UBSAN)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -g")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
    endif()
endif()

set(CppGit_VERSION "0.1.0")

string(REPLACE "." ";" version_splitted ${CppGit_VERSION})
list(GET version_splitted 0 CppGit_SOVERSION)

project(CppGit
    VERSION ${CppGit_VERSION}
    LANGUAGES CXX
)

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        src/Repository.cpp
        src/Commit.cpp
        src/Branch.cpp
        src/BranchesManager.cpp
        src/IndexManager.cpp
        src/CommitsManager.cpp
        src/CommitsLogManager.cpp
        src/DiffGenerator.cpp
        src/Merger.cpp
        src/CherryPicker.cpp
        src/Rebaser.cpp
        src/Resetter.cpp

        # _details (internal)

        src/_details/Parser/CommitParser.cpp
        src/_details/Parser/BranchesParser.cpp
        src/_details/Parser/IndexParser.cpp
        src/_details/Parser/DiffParser.cpp

        src/_details/GitCommandExecutor/GitCommandExecutor.cpp
        src/_details/GitCommandExecutor/GitCommandExecutorUnix.cpp

        src/_details/CommitCreator.cpp
        src/_details/CommitAmender.cpp
        src/_details/ThreeWayMerger.cpp
        src/_details/ReferencesManager.cpp
        src/_details/IndexWorktreeManager.cpp
        src/_details/DiffApplier.cpp
        src/_details/RebaseFilesHelper.cpp
        src/_details/GitFilesHelper.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

set(CppGit_Public_Headers 
    include/CppGit/Repository.hpp
    include/CppGit/Commit.hpp
    include/CppGit/Branch.hpp
    include/CppGit/BranchesManager.hpp
    include/CppGit/IndexManager.hpp
    include/CppGit/CommitsManager.hpp
    include/CppGit/CommitsLogManager.hpp
    include/CppGit/DiffFile.hpp
    include/CppGit/DiffGenerator.hpp
    include/CppGit/Merger.hpp
    include/CppGit/CherryPicker.hpp
    include/CppGit/RebaseTodoCommand.hpp
    include/CppGit/Rebaser.hpp
    include/CppGit/Resetter.hpp
    include/CppGit/Signature.hpp
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        VERSION ${CppGit_VERSION}
        SOVERSION ${CppGit_SOVERSION}
        PUBLIC_HEADER "${CppGit_Public_Headers}"
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

# install _details headers 
install(DIRECTORY include/CppGit/_details/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/CppGit/_details
    FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT ${PROJECT_NAME}-Targets
    FILE ${PROJECT_NAME}Config.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

if (CPPGIT_MASTER_PROJECT)
    target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    enable_testing()
    add_subdirectory(tests)
endif()