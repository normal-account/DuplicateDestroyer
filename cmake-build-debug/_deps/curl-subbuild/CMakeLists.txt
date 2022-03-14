# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.17.5)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(curl-populate NONE)

include(ExternalProject)
ExternalProject_Add(curl-populate
                     "UPDATE_DISCONNECTED" "False" "URL" "https://github.com/curl/curl/releases/download/curl-7_79_1/curl-7.79.1.tar.xz" "URL_HASH" "SHA256=0606f74b1182ab732a17c11613cbbaf7084f2e6cca432642d0e3ad7c224c3689"
                    SOURCE_DIR          "/home/carle/CLionProjects/DuplicateDestroyer/cmake-build-debug/_deps/curl-src"
                    BINARY_DIR          "/home/carle/CLionProjects/DuplicateDestroyer/cmake-build-debug/_deps/curl-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
)
