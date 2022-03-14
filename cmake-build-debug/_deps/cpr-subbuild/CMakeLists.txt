# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.17.5)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(cpr-populate NONE)

include(ExternalProject)
ExternalProject_Add(cpr-populate
                     "UPDATE_DISCONNECTED" "False" "GIT_REPOSITORY" "https://github.com/libcpr/cpr.git" "GIT_TAG" "beb9e98806bb84bcc130a2cebfbcbbc6ce62b335"
                    SOURCE_DIR          "/home/carle/CLionProjects/DuplicateDestroyer/cmake-build-debug/_deps/cpr-src"
                    BINARY_DIR          "/home/carle/CLionProjects/DuplicateDestroyer/cmake-build-debug/_deps/cpr-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
)
